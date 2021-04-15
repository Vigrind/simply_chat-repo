#ifndef MANAGE_CLIENT_H
#define MANAGE_CLIENT_H
#include <arpa/inet.h>

#define NICKNAMESIZE 25
#define ROOM_NAME 25
#define PASW_SIZE 25

typedef struct client{
    struct client *nextPtr;
    struct client *previusPtr;
    struct room *chat_room;
    char nickname[NICKNAMESIZE];
    char ip[INET_ADDRSTRLEN];
    int t_sd;
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

//room functions
void insert_room(Room **root,Room **now, char *name,char *pssw,char *own);
void delete_room(Room **now,Room **root,char *name);
#endif