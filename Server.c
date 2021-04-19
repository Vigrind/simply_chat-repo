/*
 * @Author: Vigrind 
 * @Date: 2021-04-18 17:30:09 
 * @Last Modified by: Vigrind
 * @Last Modified time: 2021-04-18 20:17:25
 */

/************************************
 * Cambiare le funzioni ssize_t di public_message in void                                 
 * Controola i commente dei mutex per i send_all sia client che room
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
#include "client_room.h"
#include "./flagmsg/st_msg.h"

#define RED  "\x1B[31m"
#define RESET "\x1B[0m"

//light
pthread_mutex_t mutex_client_list = PTHREAD_MUTEX_INITIALIZER;  //mutex when work with the client list
pthread_mutex_t mutex_room_list = PTHREAD_MUTEX_INITIALIZER;    //mutex when work with the room list
pthread_mutex_t mutex_send_all = PTHREAD_MUTEX_INITIALIZER;     //mutex when send message to all client     
pthread_mutex_t mutex_room_all = PTHREAD_MUTEX_INITIALIZER;     //mutex when send message into the room

//global vairable
Client *root;
Client *now;
Room *r_root;
Room *r_now;
int count_client = 0;

//functions
void handler(int sign); 
void initialize_server(struct sockaddr_in *server,int *sd);
void connection(int* sd,struct sockaddr_in *client,socklen_t *client_len);

//thread function
void *manage_thread(void *arg);

//function for manage connection
//send a private message to the client
ssize_t send_private_message(char *chat, char *nickname,char *my_nick);

//list all client
void send_all_client_nick(int sd, char *my_nick);

//send public message to all client
ssize_t send_public_message(char *chat, char *sent_from,char *my_nick);

//send message into the room
void send_all_room(char *chat, char *sent_from, char *room_name);

//list all room
void room_list(int sd);

int main()
{
    signal(SIGINT,handler);
    signal(SIGPIPE,SIG_IGN);

    int sd;

    struct sockaddr_in server, client;
    socklen_t client_len;

    initialize_server(&server,&sd);
    
    //initialize Client list
    root=insert(inet_ntoa(server.sin_addr),sd);
    now=root;
    
    connection(&sd, &client, &client_len);
    
    return 0;
}

void handler(int sign)
{
    //delete all lists, client and room
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
        
        Room *r_tmp;
        while (r_root != NULL)
        {
            r_tmp = r_root;
            r_root = r_root->nextPtr;
            free(r_tmp);
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
        
        pthread_mutex_lock(&mutex_client_list);
        Client *s_client = insert(inet_ntoa(client->sin_addr),connect);
        s_client->previusPtr = now;
        now->nextPtr = s_client;
        now = s_client;
        pthread_mutex_unlock(&mutex_client_list);

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
    pthread_mutex_lock(&mutex_client_list);
    count_client++;
    pthread_mutex_unlock(&mutex_client_list);

    //cast arg
    Client *t_client = (Client*)arg;   

    //nuber of byte returned by recv
    ssize_t nread; 

    //message send
    Message msg;

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
        
            //send all client nickname    
            send_all_client_nick(t_client->t_sd,t_client->nickname);
            //send end of list
            msg.type=ENDL_LIST_C;
            if((nread=send(t_client->t_sd,(void*)&msg,sizeof(msg),0))<0){perror(RED"\nCannot send data to client"RESET);}
            break; 
        
        case PUBLIC_MESSAGE:
            
            pthread_mutex_lock(&mutex_send_all);
            if((nread=send_public_message(msg.data,msg.nickname,t_client->nickname))<0){perror(RED"\nCannot send data to client"RESET);}
            pthread_mutex_unlock(&mutex_send_all);

            break;
        
        case CREATE_ROOM:
            
            strcpy(c_room_name,msg.rm_name);
            
            pthread_mutex_lock(&mutex_room_list);
            insert_room(&r_root,&r_now,msg.rm_name,msg.rm_pswd,t_client->nickname);
            t_client->chat_room = r_now;
            r_now->c_list[0] = t_client;
            pthread_mutex_unlock(&mutex_room_list);

            break;
        
        case JOIN_ROOM:
            strcpy(c_room_name,msg.rm_name);
            if ((nread=associate_c_r(r_now,msg.rm_name,msg.rm_pswd,t_client))==N_EXISTS)
            {
                msg.type=ROOM_NOT_EXISTS;
                
                if((nread=send(t_client->t_sd,(void *)&msg,sizeof(msg),0))<0){perror(RED"\nCannot send data to client"RESET);}
            
            }else if (nread == J_ROOM)
            {
                msg.type=JOIN_ROOM;
                if((nread=send(t_client->t_sd,(void *)&msg,sizeof(msg),0))<0){perror(RED"\nCannot send data to client"RESET);}
                
            }else if (nread == F_ROOM)
            {
                msg.type=FULL_ROOM;
                if((nread=send(t_client->t_sd,(void *)&msg,sizeof(msg),0))<0){perror(RED"\nCannot send data to client"RESET);}
            
            }else
            {
                msg.type = WRONG_PWSD;
                if((nread=send(t_client->t_sd,(void *)&msg,sizeof(msg),0))<0){perror(RED"\nCannot send data to client"RESET);}
            }
            break;

        case MSG_ROOM:
            
            pthread_mutex_lock(&mutex_room_all);
            send_all_room(msg.data,msg.nickname,t_client->chat_room->name);
            pthread_mutex_unlock(&mutex_room_all);
            break;

        case EXIT_ROOM:

            pthread_mutex_lock(&mutex_room_list);            
            exit_room(t_client,t_client->chat_room->name,r_now);
            ck_empty_room(&r_now,&r_root,c_room_name);
            pthread_mutex_unlock(&mutex_room_list);
        
            break;
        
        case LIST_ROOM:

            //send all room name
            room_list(t_client->t_sd);
            //send end of list
            msg.type=END_LIST_R;
            if((nread=send(t_client->t_sd,(void*)&msg,sizeof(msg),0)<0)){perror(RED"\nCannot send data to client"RESET);}
            break;
        
        default:
            break;
        }

        memset(msg.nickname,0,strlen(msg.nickname));
        memset(msg.data,0,strlen(msg.data));
        if(nread<0) {break;}
    }

    //if the room is empty, delete the room
    if(c_room_name != NULL && t_client->chat_room != NULL)
    {
        pthread_mutex_lock(&mutex_room_list);            
        exit_room(t_client,c_room_name,r_now);
        ck_empty_room(&r_now,&r_root,c_room_name);
        pthread_mutex_unlock(&mutex_room_list);
    }

    printf(RED"Connection lost from: "RESET"[%s]\n\n"RESET,t_client->nickname);
    fflush(stdout);

    //empty all buffs
    memset(t_client->nickname,0,NICKNAMESIZE);

    //close socket
    //delete the client from the client_list
    close(t_client->t_sd);

    pthread_mutex_lock(&mutex_client_list);
    delete_client(&t_client,&now);
    count_client--;
    pthread_mutex_unlock(&mutex_client_list);
    
    free(t_client);
    return NULL;
}

ssize_t send_private_message(char *chat, char *nickname, char *my_nick)
{
    //set the Message struct 
    Message msg;
    msg.type=PRIVATE_MESSAGE;
    strcpy(msg.nickname,my_nick);
    strcpy(msg.data,chat);

    char reject_ip[INET_ADDRSTRLEN]; // this variable is for exit from while
    char f_nick[NICKNAMESIZE]; // when equal to nickname, send message

    //start from the tail of Client list
    //set pointer fro scroll the list
    Client *find_nick=now;
    strcpy(f_nick,find_nick->nickname);
    strcpy(reject_ip,now->ip);

    ssize_t byte_send; //return the byte send

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

void send_all_client_nick(int sd,char *my_nick)
{
    //set the Message struct 
    Message msg;
    ssize_t bytes_retured;
    msg.type=LIST_ALL_CLIENT;

    //start from the tail of Client list
    //set pointer fro scroll the list
    Client *current = now;
    char reject_ip[INET_ADDRSTRLEN]; // this variable is for exit from while
    strcpy(reject_ip,current->ip);

    while (strcmp(reject_ip,root->ip)!=0)
    {  
        strcpy(msg.nickname,current->nickname);//take the current nickname of the list

        if(strcmp(current->nickname,my_nick)!=0)
        {
            bytes_retured=send(sd,(void*)&msg,sizeof(msg),0);
            if(bytes_retured<=0) {printf(RED"Error to send message to client\n"RESET);}
        }
       
        current=current->previusPtr;
        strcpy(reject_ip,current->ip);
    }
    
}

ssize_t send_public_message(char *chat, char *sent_from, char *my_nick)
{
    Message msg;
    ssize_t bytes_retured;
    char reject_ip[INET_ADDRSTRLEN]; // this variable is for exit from while

    //set Message struct
    msg.type=PUBLIC_MESSAGE;
    strcpy(msg.data,chat);
    strcpy(msg.nickname,sent_from);

    //start from the tail of Client list
    //set pointer fro scroll the list
    Client *currentPtr=now;
    strcpy(reject_ip,currentPtr->ip);

    //scroll the list
    //send the message if the name of the client not equal to the name of the sender
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
    //set Message struct
    Message msg;
    msg.type=MSG_ROOM;
    strcpy(msg.data,chat);
    strcpy(msg.nickname,sent_from);

    //byte send
    ssize_t byte_send;
    
    //start from the tail of the Room list
    //set pointer for scroll the list
    Room *find = r_now;

    //search the name of the room
    while (find != NULL)
    {
        if (strcmp(find->name,room_name)==0)
        {
            //iterate over each element of array c_list
            //send message if the element is not null
            for (size_t i = 0; i < 100; i++)
            {  
                if (find->c_list[i] != NULL)
                {
                    if((byte_send=send(find->c_list[i]->t_sd,(void*)&msg,sizeof(msg),0))<0){printf(RED"ERROR: Inpossibile send data to client\n"RESET);}
                }
            }            
        }
        find=find->previusPtr;
    }
    
}

void room_list(int sd)
{
    //set Message struct
    Message msg;
    msg.type=LIST_ROOM;
   
    //byte send
    ssize_t byte_send;
    
    //start from the tail of the Room list
    //set pointer for scroll the list
    Room *list = r_now;

    while (list != NULL)
    {
        strcpy(msg.rm_name,list->name); //copy the name in msg and send
        if((byte_send=send(sd,(void *)&msg,sizeof(msg),0))<0) {printf(RED"ERROR: Inpossibile send data to client\n"RESET);}
        list = list->previusPtr;
    }
}