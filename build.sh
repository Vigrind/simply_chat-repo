#!/usr/bin/env bash

gcc -Wall -pthread -o server \
  flagMsg/st_msg.h \
  Server.c manage_client.c
