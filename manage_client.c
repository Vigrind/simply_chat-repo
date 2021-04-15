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

void delete_room(Room **now,Room **root,char *name)
{   
    Room *delete = (*now);
    
    while (strcmp(delete->name,name) && delete != NULL)
    {
        
        if(delete->previusPtr == NULL) break;
        
        delete = delete->previusPtr;
    }

    if(delete==(*now) && delete->name == name && delete==(*root))
    {
        free(delete); 
    
    }else if(delete==(*now) && delete->name == name)
    {   
       
        (*now) = delete->previusPtr;
        (*now)->nextPtr = NULL;
        free(delete);
        
    }else if(delete != (*now) && delete->name == name){
      
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
}