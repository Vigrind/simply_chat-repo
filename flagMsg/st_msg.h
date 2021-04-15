#ifndef ST_MSG_H
#define ST_MSG_H

typedef enum em_type{
    
    SET_NICKNAME,
    LIST_ALL_CLIENT,
    PRIVATE_MESSAGE,
    PUBLIC_MESSAGE,
    CREATE_ROOM,
    JOIN_ROOM

}message_type;


typedef struct message {
    
    message_type type;
    char nickname[25];
    char data[1024];
    char rm_name[25];
    char rm_pswd[25];

}Message;

//function 
int removen(char *msg);
int removen_and_space(char *msg);
#endif