#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "client_room.h"

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

    if (delete == NULL)
    {
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
                    if (nowptr->c_list[i] == NULL)
                    {
                        client->chat_room = nowptr;
                        nowptr->c_list[i] = client;
                        return J_ROOM;
                    }
                    i++;
                }     
                
            }else
            {
                return W_PSWD;
            }
            
            return F_ROOM;
        }
        
        nowptr=nowptr->previusPtr;
    }

    return N_EXISTS;
}

void exit_room(Client *current, Room *my_room)
{
    //scroll array c_list
    //if the element != NULL control is nickname
    //if the nickname is == to nickname of the client, separate the client from the list
    for (size_t i = 0; i < 100; i++)
    {
        if (my_room->c_list[i]!= NULL)
        {
            if (strcmp(current->nickname,my_room->c_list[i]->nickname)==0)
            {
                current->chat_room = NULL; //set pointer to NULL, the client is no longer associated
                my_room->c_list[i] = NULL; //the room is no longer associated to the client
                return;
            }
        }    
                
    }
}

void ck_empty_room(Room **now, Room **root, char *r_name)
{
    //start at the tail of the Room list
    Room *search=(*now);

    while (search != NULL)
    {
        if (strcmp(search->name,r_name)==0)
        {
            //if there is 1 element != NULL in the c_list return
            //if all element is == to NULL, exit from the for and delete the room
            for (size_t i = 0; i < 100; i++)
            {
                if (search->c_list[i]!=NULL)
                {
                    return;
                }
                
            }

            delete_room(now,root,r_name);           
            return;        
        }
        
        search=search->previusPtr;
    }
    
}