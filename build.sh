#!/usr/bin/env bash

gcc -Wall -pthread -o server \
  flagmsg/st_msg.c \
  Server.c client_room.c
