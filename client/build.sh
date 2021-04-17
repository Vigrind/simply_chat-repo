#!/bin/bash

gcc -Wall -pthread -o client \
  ../flagmsg/st_msg.c \
  client.c
