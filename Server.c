
/************************************
 * Cambiare le funzioni ss_size di public_message in void                                 
 * Gestire meglio la deallocazione della stanza
 * Gestire meglio l'invio di messaggi nella stanza
 * Nella funzione associate_c_r , gestisci meglio i return(fai le macro)
 * **********************************
*/

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <sys/stat.h>
#include <sys/socket.h>
#include <pthread.h>
#include <netinet/in.h>
#include <pthread.h>
#include <string.h>
#include "manage_client.h"
#include "./flagmsg/st_msg.h"

#define RED  "\x1B[31m"
#define RESET "\033[0m"

//light
pthread_mutex_t client_list_mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t room_list_mutex = PTHREAD_MUTEX_INITIALIZER;

//global vairable
Client *root;
Client *now;
Room *r_root;
Room *r_now;
int count_client;

//functions
void handler(int sign); 
void initialize_server(struct sockaddr_in *server,int *sd);
void connection(int* sd,struct sockaddr_in *client,socklen_t *client_len);

//thread function
void *manage_thread(void *arg);

//function for manage connection
ssize_t send_private_message(char *chat, char *nickname,char *my_nick);
ssize_t send_all_client_nick(int sd, char *my_nick);
ssize_t send_public_message(char *chat, char *sent_from,char *my_nick);
void send_all_room(char *chat, char *sent_from, char *room_name);

int main()
{
    signal(SIGINT,handler);
    signal(SIGPIPE,SIG_IGN);

    int sd;

    struct sockaddr_in server, client;
    socklen_t client_len;

    initialize_server(&server,&sd);
    
    //initialize linked list2
    root=insert(inet_ntoa(server.sin_addr),sd);
    now=root;
    
    connection(&sd, &client, &client_len);
    
    return 0;
}

void handler(int sign)
{
    if (sign==SIGINT)
    {
        Client *tmp;
        while (root != NULL)
        {
            printf("\nClose sd:%d",root->t_sd);
            close(root->t_sd);
            tmp=root;
            root=root->nextPtr;
            free(tmp);
        }
        printf("\nBye\n");
        exit(EXIT_SUCCESS);
    }
    
}

void initialize_server(struct sockaddr_in *server,int *sd)
{
    (*server).sin_family = AF_INET;
    (*server).sin_port = htons(5400);
    (*server).sin_addr.s_addr = htonl(INADDR_ANY);

    (*sd)=socket(PF_INET,SOCK_STREAM,0);
    if((*sd)==-1) {perror("socket"); exit(EXIT_FAILURE);}
    if(bind((*sd),(struct sockaddr*)server,sizeof((*server)))<0) {perror("bind"); exit(EXIT_FAILURE);}
    if(listen((*sd),5)<0){perror("listen"); exit(EXIT_FAILURE);}
}

void connection(int* sd,struct sockaddr_in* client,socklen_t *client_len)
{
    printf("waiting client.......\n\n");

    int connect;
    
    while (1)
    {
        (*client_len)=sizeof((*client));
        connect=accept((*sd),(struct sockaddr*)client,client_len);
        if(connect==-1) {perror("accept"); exit(EXIT_FAILURE);}
        
        pthread_mutex_lock(&client_list_mutex);
        Client *s_client = insert(inet_ntoa(client->sin_addr),connect);
        s_client->previusPtr = now;
        now->nextPtr = s_client;
        now = s_client;
        pthread_mutex_unlock(&client_list_mutex);

        pthread_t tid;
        int err;
        if((err=pthread_create(&tid,NULL,manage_thread,(void *)s_client))<0)
        {
            printf("error to create thread: %s",strerror(err));
            exit(EXIT_FAILURE);
        }

    }
    
}

void *manage_thread(void *arg)
{
    //Indicate that the thread is never to be joined with pthread_join.
    pthread_detach(pthread_self());

    //lock mutex for increment global variable
    pthread_mutex_lock(&client_list_mutex);
    count_client++;
    pthread_mutex_unlock(&client_list_mutex);

    //cast arg
    Client *t_client = (Client*)arg;   

    //nuber of byte returned by recv
    ssize_t nread; 
    
    //message send
    Message msg;
    
    //In case mgs.type == LIST_ALL_CLIENT
    //Send all client number - 1, not_me decreases its value
    int not_me;

    //for check function in case:EXIT_ROOM
    char c_room_name[25];

    while (recv(t_client->t_sd,NULL,1,MSG_PEEK | MSG_DONTWAIT)!=0)
    {

        if((nread=recv(t_client->t_sd,(void*)&msg,sizeof(msg),0))<=0) {perror(RED"\nERROR:recive client data"RESET); break;}

        switch (msg.type)
        {
        case SET_NICKNAME:
            
            //save nickname
            strcpy(t_client->nickname,msg.nickname);
            printf("New connection from: [%s]\n",t_client->nickname);
            break;
        
        case PRIVATE_MESSAGE:
            
            //send private message
            send_private_message(msg.data,msg.nickname,t_client->nickname);
            break;

        case LIST_ALL_CLIENT:
            
            //set char *msg.data to value of not_me - 1
            not_me=count_client;
            --not_me;
            sprintf(msg.data,"%d",not_me);
             
            //send client number
            if((nread=send(t_client->t_sd,(void*)&msg,sizeof(msg),0))<0){perror(RED"\nCannot send data to client"); break;}   
           
            //if number of client is gt 0 , send client's nickname
            if(not_me > 0)
            {
                //send all client nickname    
                if((nread=send_all_client_nick(t_client->t_sd,t_client->nickname))<0){perror(RED"\nCannot send data to client"); break;}
            }
            
            break; 
        
        case PUBLIC_MESSAGE:

            if((nread=send_public_message(msg.data,msg.nickname,t_client->nickname))<0){perror(RED"\nCannot send data to client"); break;}
            break;
        
        case CREATE_ROOM:
            
            pthread_mutex_lock(&room_list_mutex);
            insert_room(&r_root,&r_now,msg.rm_name,msg.rm_pswd,t_client->nickname);
            t_client->chat_room = r_now;
            r_now->c_list[0] = t_client;
            pthread_mutex_unlock(&room_list_mutex);

            printf("Room name:%s, Room_pass:%s, Room_own:%s, Firs_client_name:%s\n",r_now->name,r_now->psw,r_now->owner,r_now->c_list[0]->nickname);//DEBUG
            printf("Client is in room:%s\n",t_client->chat_room->name);//DEBUG

            break;
        
        case JOIN_ROOM:
            strcpy(c_room_name,msg.rm_name);
            if ((nread=associate_c_r(r_now,msg.rm_name,msg.rm_pswd,t_client))==0)
            {
                msg.type=ROOM_NOT_EXISTS;
                
                if((nread=send(t_client->t_sd,(void *)&msg,sizeof(msg),0))<0){perror(RED"\nCannot send data to client"); break;}
            
            }else if (nread == 1)
            {
                msg.type=JOIN_ROOM;
                if((nread=send(t_client->t_sd,(void *)&msg,sizeof(msg),0))<0){perror(RED"\nCannot send data to client"); break;}
                
            }else if (nread == 2)
            {
                msg.type=FULL_ROOM;
                if((nread=send(t_client->t_sd,(void *)&msg,sizeof(msg),0))<0){perror(RED"\nCannot send data to client"); break;}
            
            }else
            {
                msg.type = WRONG_PWSD;
                if((nread=send(t_client->t_sd,(void *)&msg,sizeof(msg),0))<0){perror(RED"\nCannot send data to client"); break;}
            }
            break;

        case MSG_ROOM:
            
            send_all_room(msg.data,msg.nickname,t_client->chat_room->name);
            break;
        
        case EXIT_ROOM:

            printf("Name of the room:%s\n",t_client->chat_room->name);
            exit_room(t_client,t_client->chat_room->name,r_now);
            
            //if the room is empty, delete the room
            pthread_mutex_lock(&room_list_mutex);            
            ck_empty_room(&r_now,&r_root,c_room_name);
            pthread_mutex_unlock(&room_list_mutex);
            
            printf("ESCO\n");//DEBUG
            break;

        default:
            break;
        }

        memset(msg.nickname,0,strlen(msg.nickname));
        memset(msg.data,0,strlen(msg.data));
        if(nread<=0) {break;}
    }

    //if the room is empty, delete the room
    if(c_room_name != NULL)
    {
        pthread_mutex_lock(&room_list_mutex);            
        ck_empty_room(&r_now,&r_root,c_room_name);
        pthread_mutex_unlock(&room_list_mutex);
    }

    printf(RED"Connection lost from: "RESET"[%s]\n\n"RESET,t_client->nickname);
    fflush(stdout);

    //empty all buffs
    memset(t_client->nickname,0,NICKNAMESIZE);

    //close socket
    //delete the client from the client_list
    close(t_client->t_sd);

    pthread_mutex_lock(&client_list_mutex);
    delete_client(&t_client,&now);
    count_client--;
    pthread_mutex_unlock(&client_list_mutex);
    
    free(t_client);
    return NULL;
}

ssize_t send_private_message(char *chat, char *nickname, char *my_nick)
{
    Message msg;
    msg.type=PRIVATE_MESSAGE;
    strcpy(msg.nickname,my_nick);
    strcpy(msg.data,chat);

    char reject_ip[INET_ADDRSTRLEN];
    char f_nick[NICKNAMESIZE];

    Client *find_nick=now;
    strcpy(f_nick,find_nick->nickname);
    strcpy(reject_ip,now->ip);

    ssize_t byte_send;

    while (strcmp(reject_ip,root->ip)!=0)
    {
        if (strcmp(f_nick,nickname)==0)
        {
            byte_send=send(find_nick->t_sd,(void *)&msg,sizeof(msg),0);
            if(byte_send<0) {perror("send_private_message"); return byte_send;}
            break;        
        }

        find_nick=find_nick->previusPtr;
        strcpy(f_nick,find_nick->nickname);        
        strcpy(reject_ip,find_nick->ip);
    }
    return byte_send;
}

ssize_t send_all_client_nick(int sd,char *my_nick)
{
    Message msg;
    ssize_t bytes_retured;

    Client *current = now;
    char reject_ip[INET_ADDRSTRLEN];
    strcpy(reject_ip,current->ip);

    while (strcmp(reject_ip,root->ip)!=0)
    {  
        strcpy(msg.nickname,current->nickname);

        if(strcmp(current->nickname,my_nick)!=0)
        {
            bytes_retured=send(sd,(void*)&msg,sizeof(msg),0);
            if(bytes_retured<=0) {printf("Error to send message to client\n"); return bytes_retured;}
        }
       
        current=current->previusPtr;
        strcpy(reject_ip,current->ip);
    }
    
    return bytes_retured;
}

ssize_t send_public_message(char *chat, char *sent_from, char *my_nick)
{
    Message msg;
    ssize_t bytes_retured;
    char reject_ip[INET_ADDRSTRLEN];

    //set message
    msg.type=PUBLIC_MESSAGE;
    strcpy(msg.data,chat);
    strcpy(msg.nickname,sent_from);

    //set pointer fro scroll the list
    Client *currentPtr=now;
    strcpy(reject_ip,currentPtr->ip);

    //scroll the list
    while (strcmp(reject_ip,root->ip)!=0)
    {
        if (strcmp(currentPtr->nickname,my_nick)!=0)
        {
            if((bytes_retured=send(currentPtr->t_sd,(void*)&msg,sizeof(msg),0))<0){printf("Error to send message\n");return bytes_retured;}
        }
        
        currentPtr=currentPtr->previusPtr;
        strcpy(reject_ip,currentPtr->ip);
    }
    
    return bytes_retured;
}

void send_all_room(char *chat, char *sent_from, char *room_name)
{
    Message msg;
    msg.type=MSG_ROOM;
    strcpy(msg.data,chat);
    strcpy(msg.nickname,sent_from);

    Room *find = r_now;

    //search the name of the room
    while (find != NULL)
    {
        if (strcmp(find->name,room_name)==0)
        {
            //iterate over each element
            //if the element does not exist, jump to the next 
            for (size_t i = 0; i < 100; i++)
            {  
                if (find->c_list[i]==NULL)
                {
                    i++;
                }else
                {
                    send(find->c_list[i]->t_sd,(void*)&msg,sizeof(msg),0);
                }
            }
                    
        }
        find=find->previusPtr;
    }
    
}