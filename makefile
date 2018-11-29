CC=gcc
CFLAGS=-g -Wall

all: server client

client: client.c client.h init_IPC.h server.h
	@echo compiling client
	$(CC) $(CFLAGS) -o $@ client.c
	@echo created client executable

server: server.c server.h init_IPC.h
	@echo compiling server
	$(CC) $(CFLAGS) -o $@ server.c
	@echo created server executable

init: init_IPC.c init_IPC.h
	@echo initialization of IPC objects
	$(CC) $(CFLAGS) -o $@ init_IPC.c
	@echo finished initialization of IPC objects

clean:
	@echo cleaning project
	rm -rf init server client
	@echo finished cleaning project
