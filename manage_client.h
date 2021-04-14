#ifndef MANAGE_CLIENT_H
#define MANAGE_CLIENT_H
#include <arpa/inet.h>

#define NICKNAMESIZE 25

typedef struct client{
    struct client *nextPtr;
    struct client *previusPtr;
    char nickname[NICKNAMESIZE];
    char ip[INET_ADDRSTRLEN];
    int t_sd;
}Client;

Client* insert(char *ip,int sd);
void delete_client(Client **current, Client **now);
int substring(char *msg, char *first_number, char *second_number);
#endif