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