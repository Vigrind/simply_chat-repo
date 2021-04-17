#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "manage_client.h"

//client functions
Client* insert(char *ip,int sd)
{
    Client *nowptr=(Client *)malloc(sizeof(Client));
    if (nowptr == NULL)
    {
        printf("No memory aviable\n");
        exit(EXIT_FAILURE);
    }
    nowptr->t_sd=sd;
    nowptr->nextPtr=NULL;
    nowptr->previusPtr=NULL;
    strncpy(nowptr->ip,ip,INET_ADDRSTRLEN);
    return nowptr;
    
}

void delete_client(Client **delete, Client **now)
{
    if((*delete)==(*now))
    {
        (*now) = (*delete)->previusPtr;
        (*now)->nextPtr = NULL;
        
    }else{
        (*delete)->previusPtr->nextPtr = (*delete)->nextPtr;
        (*delete)->nextPtr->previusPtr = (*delete)->previusPtr;
    }
}

//Room functions
void insert_room(Room **root,Room **now, char *name,char *pssw,char *own)
{
    Room *newPtr=(Room *)malloc(sizeof(Room));
    
    if(newPtr == NULL)
    {
        printf("No memory aviable\n");
        exit(EXIT_FAILURE);
    }
    
    strncpy(newPtr->name,name,24);
    strncpy(newPtr->psw,pssw,24);
    strncpy(newPtr->owner,own,24);
    newPtr->nextPtr=NULL;
    newPtr->previusPtr=NULL;

    if ((*root)==NULL)
    {
        (*root)=newPtr;
        (*now)=(*root);
    
    }else{
        newPtr->previusPtr=(*now);
        (*now)->nextPtr=newPtr;
        (*now) = newPtr;
    }
    
}

int delete_room(Room **now,Room **root,char *name)
{   
    Room *delete = (*now);

    printf("delete name:%s\n",delete->name);//DEBUG

    if (delete == NULL)
    {
        printf("NULL\n");//DEBUG
        return 1;
    }
     
    
    while (strcmp(delete->name,name)!=0 && delete != NULL)
    {
        printf("while per cercare il nome\n");//DEBUG
        printf("name:%s, delete:%s\n",name,delete->name);
        
        if(delete->previusPtr == NULL) break;
        
        delete = delete->previusPtr;
    }

    if(delete==(*now) && strcmp(delete->name,name)==0 && delete==(*root))
    {
        printf("primo free\n");//DEBUG
        free(delete);
        (*now)=NULL;
        (*root)=NULL;
    
    }else if(delete==(*now) && strcmp(delete->name,name)==0)
    {   
        printf("secondo free\n");//DEBUG
        (*now) = delete->previusPtr;
        (*now)->nextPtr = NULL;
        free(delete);
        
    }else if(delete != (*now) && strcmp(delete->name,name)==0){
      printf("terzo free\n");//DEBUG
        if(delete == (*root))
        {
       
            (*root) = delete->nextPtr;
            (*root)->previusPtr = NULL;
            free(delete); 

        }else{
           
            delete->previusPtr->nextPtr = delete->nextPtr;
            delete->nextPtr->previusPtr = delete->previusPtr;
            free(delete);
        }
    }

    return 0;
}

int associate_c_r(Room *nowptr, char *r_name, char *passw, Client *client)
{
    int i = 0;

    while(nowptr != NULL)
    {
        if (strcmp(nowptr->name,r_name)==0 )
        {   
            if (strcmp(nowptr->psw,passw)==0)
            {
                while (i<100)
                {
                    if (nowptr->c_list[i] != NULL)
                    {
                        printf("CLIENT_LSIT_NAME:%s, AT POSITION:[%d]\n",nowptr->c_list[i]->nickname,i);//DEBUG
                    }

                    if (nowptr->c_list[i] == NULL)
                    {
                        printf("client name:%s\n",client->nickname);//DEBUG
                        printf("Client_list_positiom:%d\n",i);//DEBUG
                        client->chat_room = nowptr;
                        nowptr->c_list[i] = client;
                        printf("Room_client_list_name:%s\n",nowptr->c_list[i]->nickname);//DEBUG
                        return 1;
                    }
                    i++;
                }     
                
            }else
            {
                return 3;
            }
            
            return 2;
        }
        
        nowptr=nowptr->previusPtr;
    }

    return 0;
}

void exit_room(Client *current, char *r_name, Room *nowPtr)
{
    while (nowPtr != NULL)
    {
        if (strcmp(nowPtr->name,r_name)==0)
        {
            for (size_t i = 0; i < 100; i++)
            {
                if (strcmp(current->nickname,nowPtr->c_list[i]->nickname)==0)
                {
                    current->chat_room = NULL;
                    nowPtr->c_list[i] = NULL;
                    return;
                }
                
            }
            
        }
        
        nowPtr=nowPtr->previusPtr;
    }
    return;
}

void ck_empty_room(Room **now, Room **root, char *r_name)
{
    Room *search=(*now);

    while (search != NULL)
    {
        if (strcmp(search->name,r_name)==0)
        {
            for (size_t i = 0; i < 100; i++)
            {
                if (search->c_list[i]!=NULL)
                {
                    printf("esco dal check\n");//DEBUG
                    return;
                }
                
            }

            //if the c_list is empty, exit from the room
            printf("Entro nel delete\n");//DEBUG
            delete_room(now,root,r_name);
            printf("esco del check_dopo il delete\n");//DEBUG
            return;
                 
        }
        
    }
    
}