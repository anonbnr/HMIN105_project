CC=gcc
CFLAGS=-c -g -Wall
THREAD=-lpthread

all: server client

client: client.o client.h socket_wrapper.o utility.h
	@echo creating client executable
	$(CC) -o $@ client.o socket_wrapper.o $(THREAD)
	@echo created executable successfully

server: server.o server.h socket_wrapper.o utility.h
	@echo creating server executable
	$(CC) -o $@ server.o socket_wrapper.o $(THREAD)
	@echo created executable successfully
	@echo

server.o: server.c server.h utility.h socket_wrapper.h
	@echo compiling server program
	$(CC) $(CFLAGS) server.c
	@echo finished compiling
	@echo

client.o: client.c client.h utility.h socket_wrapper.h
	@echo compiling client program
	$(CC) $(CFLAGS) client.c
	@echo finished compiling
	@echo

socket_wrapper.o: socket_wrapper.c socket_wrapper.h
	@echo compiling socket wrapper program
	$(CC) $(CFLAGS) socket_wrapper.c
	@echo finished compiling
	@echo

clean:
	@echo cleaning project
	rm -rf *.o server client
	@echo finished cleaning project
