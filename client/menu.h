#ifndef MENU_H
#define MENU_H
#include "../flagmsg/st_msg.h"

//color
#define YEL  "\x1B[0;33m"
#define BLU  "\x1B[0;34m"
#define MAG  "\x1B[0;35m"
#define UMAG  "\x1B[4;35m"
#define RED  "\x1B[0;31m"
#define WHT  "\x1B[0;37m"
#define GRN  "\x1B[0;32m"
#define CYN  "\x1B[0;36m"
#define RESET "\033[0m"


//menu function
void print_menu();
void help();

//menu action functions
void send_private_message(char *chooise, int *sd);
void send_public_message(char *chooise,char *nick, int *sd);
void send_list_all(int *sd);
void create_room(char *chooise, char *nick,int *sd, int *flag_room);
void join_room(char *chooise, int *sd, int*flag_room);
void exit_room(int *sd);
void send_room(char *chooise,char *nick, int *sd);
void list_room(int *sd);

#endif