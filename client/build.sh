#!/bin/bash

gcc -Wall -pthread -o client \
  ../flagMsg/st_msg.c \
  client.c
