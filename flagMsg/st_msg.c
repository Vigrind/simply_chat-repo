#include "st_msg.h"

int removen(char *msg)
{
    while (*msg != '\0')
    {
        if (*msg=='\n' || *msg=='\r')
        {
            *msg='\0';
        }
        msg++;
    }
    
    return 0;
}

int removen_and_space(char *msg)
{
    while (*msg != '\0')
    {
        if (*msg=='\n' || *msg=='\r' || *msg==' ' || *msg=='\t')
        {
            *msg='\0';
        }
        msg++;
    }
    
    return 0;
}