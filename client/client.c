#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/stat.h>
#include <arpa/inet.h>
#include <string.h>
#include <signal.h>
#include <time.h>
#include <pthread.h>   
#include "../flagMsg/st_msg.h"

//color
#define RED  "\x1B[0;31m"
#define GRN  "\x1B[22;32m"
#define YEL  "\x1B[0;33m"
#define BLU  "\x1B[0;34m"
#define MAG  "\x1B[0;35m"
#define UMAG  "\x1B[4;35m"
#define CYN  "\x1B[0;36m"
#define WHT  "\x1B[0;37m"
#define RESET "\033[0m"

//functions
void initialize_connection(struct sockaddr_in*);
void take_input();
void handler(int sig);

//menu functions
void print_menu();
void chose_nickname(char *nick);
void quit();
void help();

//menu action functions
void send_private_message(char *chooise);
void send_public_message(char *chooise, char *nick);
void send_list_all();
void create_room(char *chooise, char *nick);

//thread function
void *t_recive_message(void *arg);

//global variable
int sd;

int main()
{
    //catch signal
    signal(SIGINT,handler);
    signal(SIGQUIT,handler);
    signal(SIGPIPE,handler);
    signal(SIGUSR1,handler);

    srand(time(NULL));

    //variable for server connection
    struct sockaddr_in server;

    initialize_connection(&server);
    take_input();

    close(sd);
    return 0;
}

void initialize_connection(struct sockaddr_in* server)
{
    (*server).sin_family=AF_INET;
    (*server).sin_port=htons(5400);
    inet_aton("0.0.0.0",&server->sin_addr);

    sd=socket(PF_INET,SOCK_STREAM,0);
    connect(sd,(struct sockaddr *)server,sizeof((*server)));
}

void take_input()
{
    char nickname[1024];//temporary solution
    char choise[1024];
    print_menu();
    chose_nickname(nickname);

    //create a thread that wait in backgroud the message from the server
    pthread_t tid;
    pthread_create(&tid,NULL,t_recive_message,NULL);

    while(1)
    {

        fgets(choise,sizeof(choise),stdin);
        removen(choise);

        if(strcmp(choise,"/help")==0)
        {
            help();

        }else if(choise[0]=='@')
        {
            send_private_message(choise);

        }else if(strcmp(choise,"/l")==0)
        {
            send_list_all();
            
        }else if(strncmp(choise,"/p",2)==0)
        {
            send_public_message(choise,nickname);
        
        }else if(strncmp(choise,"/c",2)==0)
        {
            create_room(choise,nickname);
        }

        memset(choise,0,sizeof(choise));
    }
    
}

void handler(int sig)
{
    if(sig==SIGINT)
    {
        close(sd);
        printf("\nThx for using Simply Chat\n");
        exit(EXIT_SUCCESS);
    }

    if(sig==SIGQUIT)
    {
        printf("Bye\n");
        close(sd);
        exit(EXIT_SUCCESS);
    }

    if(sig==SIGPIPE)
    {
        printf("Server close connection\n");
        close(sd);
        exit(EXIT_FAILURE);
    }
    
    if(sig==SIGUSR1)
    {
        printf("\nsorry\n");
        close(sd);
        exit(EXIT_FAILURE);
    }
}

void print_menu()
{
    printf(YEL"**********************\n"RESET
           CYN"Welcome to symply chat\n"RESET
           YEL"**********************\n"RESET
           "Type"BLU" /help "RESET"to see all the option\n"
           "usage"BLU" @<nickname> <message> "RESET"for send private message\n"
           "First choose a nickname: ");
    fflush(stdout);
}

void chose_nickname(char *nick)
{
    Message msg;

    while (1)
    {
        fgets(nick,1024,stdin);//temporary solution
        
        //remove space or '\n' or '\t' or '\r'
        removen_and_space(nick);

        //control the size of the string
        if(strlen(nick)>24 || strlen(nick)<4)
        {
            printf("The lenght must be between 4 and 24\n");

        }else
        {
            //prepare the message
            msg.type=SET_NICKNAME;
            strcpy(msg.nickname,nick);
            
            //send
            send(sd,(void*)&msg,sizeof(msg),0); 
            break;       
        }
    }
    
}

void *t_recive_message(void *arg)
{
    pthread_detach(pthread_self());
    
    while(1)
    {
        Message msg;
        ssize_t bytes_returned;
        int number_client;
        
        //recive data from the server
        bytes_returned = recv(sd,(void*)&msg,sizeof(msg),0);
        if(bytes_returned<=0) {printf(RED"Cannot recive data from server\n"RESET); quit();}

        switch (msg.type)
        {
        case LIST_ALL_CLIENT:
        
            //convert msg.data that contains number of client
            number_client=atoi(msg.data);
            
            if(number_client > 0)
            {
                printf(CYN"List of all client connected\n"RESET);
                while(number_client > 0)
                {
                    bytes_returned = recv(sd,(void*)&msg,sizeof(msg),0);
                    if(bytes_returned<=0) {printf(RED"Cannot recive data from server\n"RESET); quit();}
                    
                    printf(MAG"%s\n"RESET,msg.nickname);
                    fflush(stdout);
                    number_client--;
                }
                printf(CYN"End of list\n"RESET);
                fflush(stdout);
            }else
            {
                printf("No one is connected\n");
            }
            
            break;
        
        case PRIVATE_MESSAGE:
            
            printf(YEL"%s:"RESET BLU"%s\n"RESET,msg.nickname,msg.data);
            fflush(stdout);
            break;
        
        case PUBLIC_MESSAGE:
            
            printf(MAG"Public mex from"YEL"[%s]:"RED"%s\n"RESET,msg.nickname,msg.data);
            fflush(stdout);
            break;
        
        default:
            break;
        }
    }

    return NULL;
}

void quit()
{
    close(sd);
    exit(EXIT_FAILURE);
}

void help()
{
    printf(BLU"\n/l"RESET":list all client\n"
                   BLU"[CTRL+C] or [CTRL+/]"RESET":exit\n"
                   BLU"/p"RESET":send public message: usage"BLU" /p <message>\n"RESET
                   BLU"/c"RESET":create a private room: usage"BLU"/c <name> <password>\n"RESET);
} 

void send_private_message(char *chooise)
{
    Message msg;
    msg.type=PRIVATE_MESSAGE;

    char *s_username;
    char *s_msg;
            
    //s_username takes the username(stops to first space), choise + 1 because '@' not needet
    //s_msg takes the rest of the string 
    s_username=strtok(chooise+1," ");
    s_msg=strtok(NULL,"");

    //check error on username
    if(strlen(s_username)>24) {printf(RED"ERROR: Username must have max 24 characters"RESET); kill(getpid(),SIGUSR1);}
    if(s_username==NULL) {printf(RED"ERROR: Format is @<username><message>"RESET); kill(getpid(),SIGUSR1);}

    //check error on message
    if(s_msg == NULL){printf(RED"ERROR: You must enter message"RESET);kill(getpid(),SIGUSR1);}

    //prepare the srtucture to send 
    strcpy(msg.nickname,s_username);
    strcpy(msg.data,s_msg);

    //send the structure
    send(sd,(void*)&msg,sizeof(msg),0);
}

void send_list_all()
{
    Message msg;
    msg.type=LIST_ALL_CLIENT;

    //prepare the srtucture to send 
    send(sd,(void*)&msg,sizeof(msg),0);
}

void send_public_message(char *chooise,char *nick)
{
    Message msg;
            msg.type=PUBLIC_MESSAGE;

            char *s_msg;
            char *not_need;

            //start of the string, until the first space not neddet
            not_need=strtok(chooise," ");
            //s_msg takes the rest of the string
            s_msg=strtok(NULL,"");

            //check error on message
            if(s_msg == NULL){printf(RED"ERROR: You must enter message"RESET);kill(getpid(),SIGUSR1);}
            
            //set message
            strcpy(msg.data,s_msg);
            strncpy(msg.nickname,nick,24);
            //send message
            send(sd,(void*)&msg,sizeof(msg),0);
}

void create_room(char *chooise, char *nick)
{   
    Message msg;
    msg.type = CREATE_ROOM;

    char *not_need;
    char *s_name;
    char *s_pass;

    //start of the string, until the first space not neddet
    not_need=strtok(chooise," ");
    s_name=strtok(NULL," ");
    s_pass=strtok(NULL,"");
    
    if (s_name == NULL)
    {
        printf(RED"ERROR: Format is /c <name> <pass>\n"RESET);     
    
    }else if(strlen(s_name)>24)
    {
        printf(RED"ERROR: Username must have max 24 characters\n"RESET);
    
    }else if(s_pass == NULL)
    {
        printf(RED"ERROR: Format is /c <name> <password>\n"RESET);
    
    }else if(strlen(s_pass)>24)
    {
        printf(RED"ERROR: Passwprd must have max 24 characters\n"RESET);
    }
    else
    {
        strncpy(msg.nickname,nick,24);
        strncpy(msg.rm_name,s_name,24);
        strncpy(msg.rm_pswd,s_pass,24);

        send(sd,(void *)&msg,sizeof(msg),0);
    }
    

}