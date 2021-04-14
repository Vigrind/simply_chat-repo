#!/usr/bin/env bash

gcc -Wall -pthread -o server \
  flagMsg/st_msg.c \
  Server.c manage_client.c
