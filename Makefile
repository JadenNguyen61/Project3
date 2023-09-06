CC = gcc
CFLAGS = -Wall -Werror -std=gnu99 -pthread

all: dbclient dbserver

dbclient: dbclient.c 
	$(CC) dbclient.c -o dbclient $(CFLAGS) 

dbserver: dbserver.c msg.h
	$(CC) dbserver.c -o dbserver $(CFLAGS)

clean: 
	rm -f dbclient dbserver
