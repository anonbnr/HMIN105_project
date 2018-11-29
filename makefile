CC=gcc
CFLAGS=-c -g -Wall

all: server client

client: client.o server.o init_IPC.h client.h
	@echo creating client executable
	$(CC) -o $@ $?
	@echo created executable successfully

server: server.o init_IPC.h server.h
	@echo creating server executable
	$(CC) -o $@ $?
	@echo created executable successfully
	@echo

server.o: server.c server.h init_IPC.h
	@echo compiling server program
	$(CC) $(CFLAGS) server.c
	@echo finished compiling
	@echo

client.o: client.c client.h init_IPC.h server.o server.h
	@echo compiling client program
	$(CC) $(CFLAGS) client.c
	@echo finished compiling
	@echo

clean:
	@echo cleaning project
	rm -rf *.o server client
	@echo finished cleaning project
