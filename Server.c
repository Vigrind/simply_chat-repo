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
#include "./flagMsg/st_msg.h"

#define RED  "\x1B[31m"
#define RESET "\033[0m"

//light
pthread_mutex_t client_list_mutex = PTHREAD_MUTEX_INITIALIZER;

//global vairable
Client *root;
Client *now;
int count_client;

//function
void handler(int sign); 
void initialize_server(struct sockaddr_in *server,int *sd);
void connection(int* sd,struct sockaddr_in* client,socklen_t *client_len);
void *manage_thread(void *arg);
ssize_t send_private_message(char *chat, char *nickname,char *my_nick);
ssize_t send_all_client_nick(int sd);

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
            printf("Close sd:%d\n",root->t_sd);
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
    printf("waiting client.......\n");

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

    while (recv(t_client->t_sd,NULL,1,MSG_PEEK | MSG_DONTWAIT)!=0)
    {
        if((nread=recv(t_client->t_sd,(void*)&msg,sizeof(msg),0))<=0) {perror(RED"ERROR:recive client data"RESET); break;}

        switch (msg.type)
        {
        case SET_NICKNAME:
            
            //save nickname
            strcpy(t_client->nickname,msg.nickname);
            break;
        
        case PRIVATE_MESSAGE:
            send_private_message(msg.data,msg.nickname,t_client->nickname);
            break;

        case LIST_ALL_CLIENT:
            
            //set char *msg.data to value of count_client
            sprintf(msg.data,"%d",count_client);
            
            //send client number
            if((nread=send(t_client->t_sd,(void*)&msg,sizeof(msg),0))<0){perror(RED"Cannot send data to client"); break;}

            //send all client nickname    
            if((nread=send_all_client_nick(t_client->t_sd))<0){perror(RED"Cannot send data to client"); break;};
            break; 
        
        default:
            break;
        }

        memset(msg.nickname,0,strlen(msg.nickname));
        memset(msg.data,0,strlen(msg.data));
        if(nread<=0) {break;}
    }

    //empty all buffs
    memset(t_client->first_number,0,strlen(t_client->first_number));
    memset(t_client->second_number,0,strlen(t_client->second_number));
    memset(t_client->nickname,0,NICKNAMESIZE);

    //close connection
    //delete the client from the client_list
    printf("Connection lost\n");
    fflush(stdout);
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
    printf("exit\n");//DEBUG
    return byte_send;
}

ssize_t send_all_client_nick(int sd)
{
    Message msg;
    ssize_t bytes_retured;

    Client *current = now;
    char reject_ip[INET_ADDRSTRLEN];
    strcpy(reject_ip,current->ip);

    while (strcmp(reject_ip,root->ip)!=0)
    {
        strcpy(msg.nickname,current->nickname);
        bytes_retured=send(sd,(void*)&msg,sizeof(msg),0);
        if(bytes_retured<=0) {printf("Error to send message to client\n"); return bytes_retured;}
       
        current=current->previusPtr;
        strcpy(reject_ip,current->ip);
    }
    
    return bytes_retured;
}