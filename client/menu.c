#include "menu.h"
#include <stdio.h>
#include <sys/socket.h>
#include <string.h>

void print_menu()
{
    printf(YEL"**********************\n"RESET
           CYN"Welcome to symply chat\n"RESET
           YEL"**********************\n"RESET
           "Type"BLU" /help "RESET"to see all the option\n"
           "First choose a nickname: ");
    fflush(stdout);
    
}

void help()
{
    printf(BLU"\n/l"RESET":list all client\n"
           BLU"[CTRL+C] or [CTRL+/]"RESET":exit\n"
           BLU"/p"RESET":send public message:"BLU" /p <message>\n"RESET
           BLU"/c"RESET":create a private room:"BLU"/c <name> <password>\n"RESET
           BLU"/j"RESET":join a room:"BLU" /j <room_name> <room_passwd>\n"RESET
           BLU"-"RESET":send private message:"BLU" -<nickname> <message>\n"RESET
           BLU"/rl"RESET":list all room\n");
}

void send_private_message(char *chooise, int *sd)
{
    Message msg;
    msg.type=PRIVATE_MESSAGE;

    char *s_username;
    char *s_msg;
            
    //s_username takes the username(stops to first space), choise + 1 because '@' not needet
    //s_msg takes the rest of the string 
    s_username=strtok(chooise+1," ");
    s_msg=strtok(NULL,"");

    //check error on username
    if(s_username==NULL) 
    {
        printf(RED"ERROR: Format is @<username><message>"RESET);
    
    }else if(strlen(s_username)>24) 
    {
        printf(RED"ERROR: Username must have max 24 characters"RESET);
    
    }else if (s_msg == NULL)
    {
        printf(RED"ERROR: You must enter message"RESET);
    
    }else
    {
        //prepare the srtucture to send 
        strcpy(msg.nickname,s_username);
        strcpy(msg.data,s_msg);

        //send the structure
        send((*sd),(void*)&msg,sizeof(msg),0);
    }

}

void send_list_all(int *sd)
{
    Message msg;
    msg.type=LIST_ALL_CLIENT;

    //prepare the srtucture to send 
    send((*sd),(void*)&msg,sizeof(msg),0);
    printf(CYN"List all client\n"RESET);
}

void send_public_message(char *chooise,char *nick, int *sd)
{
    Message msg;
    msg.type=PUBLIC_MESSAGE;

    char *s_msg;
    char *not_need;

    //start of the string, until the first space not neddet
    not_need=strtok(chooise," ");
    //s_msg takes the rest of the string
    s_msg=strtok(NULL,"");
    //check error on message
    if(s_msg == NULL)
    {
        printf(RED"ERROR: You must enter message"RESET);
    }else
    {
        //set message
        strcpy(msg.data,s_msg);
        strncpy(msg.nickname,nick,24);
        //send message
        send(*sd,(void*)&msg,sizeof(msg),0);
    }       
}

void create_room(char *chooise, char *nick,int *sd, int *flag_room)
{   
    Message msg;
    msg.type = CREATE_ROOM;

    char *not_need;
    char *s_name;
    char *s_pass;

    //start of the string, until the first space not neddet
    not_need=strtok(chooise," ");
    s_name=strtok(NULL," ");
    s_pass=strtok(NULL,"");
    
    if (s_name == NULL)
    {
        *flag_room = 0;
        printf(RED"ERROR: Format is /c <name> <pass>\n"RESET);     
    
    }else if(strlen(s_name)>24)
    {
        *flag_room = 0;
        printf(RED"ERROR: room_name must have max 24 characters\n"RESET);
    
    }else if(s_pass == NULL)
    {
        *flag_room = 0;
        printf(RED"ERROR: Format is /c <name> <password>\n"RESET);
    
    }else if(strlen(s_pass)>24)
    {
        *flag_room = 0;
        printf(RED"ERROR: Password must have max 24 characters\n"RESET);
    }
    else
    {
        strncpy(msg.nickname,nick,24);
        strncpy(msg.rm_name,s_name,24);
        strncpy(msg.rm_pswd,s_pass,24);

        send(*sd,(void *)&msg,sizeof(msg),0);
        printf(CYN"Successful room join\n"RESET);
    }
    
}

void join_room(char *chooise, int *sd, int*flag_room)
{
    Message msg;
    msg.type=JOIN_ROOM;

    char *not_need;
    char *s_name;
    char *s_pass;

    //start of the string, until the first space not neddet
    not_need=strtok(chooise," ");
    s_name=strtok(NULL," ");
    s_pass=strtok(NULL,"");
    
    if (s_name == NULL)
    {
        printf(RED"ERROR: Format is /j <name> <pass>\n"RESET);
        *flag_room = 0;
    
    }else if(strlen(s_name)>24)
    {
        printf(RED"ERROR: room_name must have max 24 characters\n"RESET);
        *flag_room = 0;
    
    }else if(s_pass == NULL)
    {
        printf(RED"ERROR: Format is /j <name> <password>\n"RESET);
        *flag_room = 0;

    }else if(strlen(s_pass)>24)
    {
        printf(RED"ERROR: Password must have max 24 characters\n"RESET);
        *flag_room = 0;
    }
    else
    {
        strncpy(msg.rm_name,s_name,24);
        strncpy(msg.rm_pswd,s_pass,24);

        send(*sd,(void *)&msg,sizeof(msg),0);
    }
}

void exit_room(int *sd)
{
    Message msg;
    msg.type = EXIT_ROOM;

    send(*sd,(void *)&msg,sizeof(msg),0);
}

void send_room(char *chooise,char *nick, int *sd)
{
    Message msg;
    msg.type=MSG_ROOM;
  
    //set message
    strcpy(msg.data,chooise);
    strncpy(msg.nickname,nick,24);
    //send message
    send(*sd,(void*)&msg,sizeof(msg),0);       
}

void list_room(int *sd)
{
    Message msg;
    msg.type= LIST_ROOM;

    printf(CYN"List all room\n"RESET);
    send(*sd,(void*)&msg,sizeof(msg),0);
}