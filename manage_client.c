#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "manage_client.h"


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

int substring(char *msg,char *first_number, char *second_number)
{
    int j = 0;

    while (*msg != '\0')
    {
        if (*msg=='-')
        {
            ++msg;
            while (*msg != '\0')
            {
                second_number[j]=*msg;
                j++;
                msg++;
            }
            return 0;

        }else
        {
            *first_number=*msg;
            first_number++;
        }
        msg++;
    }

    return 0;
}