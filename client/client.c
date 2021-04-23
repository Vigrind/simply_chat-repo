/*
 * @Author: Vigrind 
 * @Date: 2021-04-18 17:30:32 
 * @Last Modified by: Vigrind
 * @Last Modified time: 2021-04-18 23:24:43
 */
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/stat.h>
#include <arpa/inet.h>
#include <string.h>
#include <signal.h>
#include <time.h>
#include <pthread.h>
#include "menu.h"   
#include "../flagmsg/st_msg.h"

pthread_mutex_t mutex_nick = PTHREAD_MUTEX_INITIALIZER; //mutex for choose_nickname(main thread) and t_recive_message synchronization
pthread_cond_t cond_nick = PTHREAD_COND_INITIALIZER;    //condition variable for choose_nickname(main thread) and t_recive_message synchronization

//functions
void initialize_connection(struct sockaddr_in *server);
void handler(int sig);

//client start
void take_input();

//choose a nickname
void chose_nickname(char *nick);

//exit from the application
void quit();

//thread function
void *t_recive_message(void *arg);

//global variable
int sd;
int user_ok=0;
int flag_room = 0;

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
    
    //create a thread that wait in backgroud the message from the server
    pthread_t tid;
    pthread_create(&tid,NULL,t_recive_message,NULL);

    chose_nickname(nickname);

    while(1)
    {

        fgets(choise,sizeof(choise),stdin);
        removen(choise);

        if(strcmp(choise,"/help")==0 && flag_room == 0)
        {
            help();

        }else if(choise[0]=='-')
        {
            send_private_message(choise,&sd);

        }else if(strcmp(choise,"/l")==0 && flag_room == 0)
        {
            send_list_all(&sd);
            
        }else if(strcmp(choise,"/rl")==0 && flag_room == 0)
        {
            list_room(&sd);

        }else if(strncmp(choise,"/p",2)==0 && flag_room == 0)
        {
            send_public_message(choise,nickname,&sd);
        
        }else if(strncmp(choise,"/c",2)==0 && flag_room == 0)
        {
            flag_room = 1;
            create_room(choise,nickname,&sd,&flag_room);

        }else if (strncmp(choise,"/j",2)==0 && flag_room == 0)
        {
            flag_room = 1;
            join_room(choise,&sd,&flag_room);

        }else if (strcmp(choise,"/exit")==0 && flag_room == 1)
        {
            flag_room = 0;
            exit_room(&sd);
            printf(CYN"Exit from the room\n"RESET);
        
        }else if (choise[0]!='\0' && flag_room == 1)
        {
            send_room(choise,nickname,&sd);
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
}

void chose_nickname(char *nick)
{
    Message msg;
    int count = 0;

    while (1)
    {   
        //first input don't need mutex
        if (count == 0)
        {
            fgets(nick,1024,stdin);//temporary solution
        }else
        {
            //wait for the result of t_recive_message thread, that set user_ok
            //Use condition variable for passive wait, and best thread synchronization
            pthread_mutex_lock(&mutex_nick);
               
                pthread_cond_wait(&cond_nick,&mutex_nick);        
               
                if (user_ok == 1)
                {
                    break;
                }else
                {
                    fgets(nick,1024,stdin);//temporary solution
                }
        
            pthread_mutex_unlock(&mutex_nick);
        }
        
        //remove space or '\n' or '\t' or '\r'
        removen_and_space(nick);

        //control the size of the string
        if(strlen(nick)>24 || strlen(nick)<4)
        {
            printf("The lenght must be between 4 and 24\n");
            count = 0; // if the user choose wrong format for nickname, reset count
        }else
        {
            count++; 
            //prepare the message
            msg.type=SET_NICKNAME;
            strcpy(msg.nickname,nick);
            
            //send
            send(sd,(void*)&msg,sizeof(msg),0);    
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
        
        //recive data from the server
        bytes_returned = recv(sd,(void*)&msg,sizeof(msg),0);
        if(bytes_returned<=0) {printf(RED"Cannot recive data from server\n"RESET); quit();}

        switch (msg.type)
        {
        case SET_NICKNAME:
            
            pthread_mutex_lock(&mutex_nick);
                
                //if the server send that the nickname is available
                //change user_ok from 0 to 1
                if (strcmp(msg.data,"user_ok")==0)
                {
                    user_ok = 1;
                    pthread_cond_signal(&cond_nick); //send signal to cond variable in function choose_nickname
                }else
                {
                    pthread_cond_signal(&cond_nick);
                    printf(RED"L'username è gia esistente\n"RESET); //send signal to cond variable in function choose_nickname
                    fflush(stdout);
                }
            
            pthread_mutex_unlock(&mutex_nick);
            
            break;
            
        case LIST_ALL_CLIENT:
           
            printf(MAG"%s\n"RESET,msg.nickname);
            fflush(stdout);
            break;
        
        case ENDL_LIST_C:

            printf(CYN"End of list\n"RESET);
            fflush(stdout);
            break;
        
        case PRIVATE_MESSAGE:
            
            printf(YEL"%s:" BLU"%s\n"RESET,msg.nickname,msg.data);
            fflush(stdout);
            break;
        
        case PUBLIC_MESSAGE:
            
            printf(MAG"Public mex from"YEL"[%s]:"RED"%s\n"RESET,msg.nickname,msg.data);
            fflush(stdout);
            break;
        
        case JOIN_ROOM:
            
            printf(CYN"Successful room[%s] join\n"RESET,msg.rm_name);
            fflush(stdout);
            break;
        
        case FULL_ROOM:

            flag_room = 0;
            printf(CYN"cannot join, the room is full\n"RESET);
            fflush(stdout);
            break;
        
        case ROOM_NOT_EXISTS:
            
            flag_room = 0;
            printf(CYN"The room doesn't exists\n"RESET);
            fflush(stdout);
            break;
        
        case WRONG_PWSD:
            
            flag_room = 0;
            printf(CYN"Wrong Password\n"RESET);
            fflush(stdout);
            break;
        
        case MSG_ROOM:
            
            printf(MAG"Room:"YEL"[%s]:"RED"%s\n"RESET,msg.nickname,msg.data);
            fflush(stdout);
            break;

        case LIST_ROOM:
            printf(YEL"%s\n"RESET,msg.rm_name);
            fflush(stdout);
            break;
        
        case END_LIST_R:
            printf(CYN"End of list\n"RESET);
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
