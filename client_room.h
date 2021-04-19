#ifndef MANAGE_CLIENT_H
#define MANAGE_CLIENT_H
#include <arpa/inet.h>

#define NICKNAMESIZE 25
#define ROOM_NAME 25
#define PASW_SIZE 25

//this enumeration is for associate_c_r
typedef enum ex_rm {
    N_EXISTS,
    J_ROOM,
    F_ROOM,
    W_PSWD
}room_option;

typedef struct client{
    struct client *nextPtr;
    struct client *previusPtr;
    struct room *chat_room;
    char nickname[NICKNAMESIZE];
    char ip[INET_ADDRSTRLEN];
    int t_sd; //socket descriptor     
}Client;

typedef struct room
{
    char name[ROOM_NAME];
    char psw[PASW_SIZE];
    char owner[NICKNAMESIZE];
    Client *c_list[100];
    struct room *nextPtr;
    struct room *previusPtr;

}Room;

//client functions
Client* insert(char *ip,int sd);
void delete_client(Client **current, Client **now);

/*room functions*/

//create Room node 
void insert_room(Room **root,Room **now, char *name,char *pssw,char *own);
//separate the client from the room
void exit_room(Client *current, char *r_name, Room *nowPtr);
//check if the room is empty, in positive case, delete the room
void ck_empty_room(Room **now, Room **root, char *r_name);
//delete the room from the Room list
int delete_room(Room **now,Room **root,char *name);
//join the client into the room
int associate_c_r(Room *nowptr, char *r_name, char *passw, Client *client);
#endif