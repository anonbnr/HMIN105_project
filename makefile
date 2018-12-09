CC=gcc
CFLAGS=-c -g -Wall
THREAD=-lpthread
WRAPPER=wrapper/
UTILITY=utility/

all: server client

client: client.o client.h $(WRAPPER)socket_wrapper.o $(UTILITY)utility.o
	@echo creating client executable
	$(CC) -o $@ client.o $(WRAPPER)socket_wrapper.o $(UTILITY)utility.o $(THREAD)
	@echo created executable successfully

client.o: client.c client.h $(UTILITY)utility.h $(WRAPPER)socket_wrapper.h
	@echo compiling client program
	$(CC) $(CFLAGS) client.c
	@echo finished compiling
	@echo

server: server.o server.h $(WRAPPER)ipc_wrapper.o $(WRAPPER)socket_wrapper.o $(UTILITY)stock.o $(UTILITY)utility.o
	@echo creating server executable
	$(CC) -o $@ server.o $(WRAPPER)ipc_wrapper.o $(WRAPPER)socket_wrapper.o $(UTILITY)stock.o $(UTILITY)utility.o $(THREAD)
	@echo created executable successfully
	@echo

server.o: server.c server.h $(WRAPPER)socket_wrapper.h $(WRAPPER)ipc_wrapper.h $(UTILITY)utility.h $(UTILITY)stock.h
	@echo compiling server program
	$(CC) $(CFLAGS) server.c
	@echo finished compiling
	@echo

utility.o: $(UTILITY)utility.c $(UTILITY)utility.h
	@echo compiling utility program
	$(CC) $(CFLAGS) $(UTILITY)utility.c
	@mv utility.o $(UTILITY)
	@echo finished compiling
	@echo

stock.o: $(UTILITY)stock.c $(UTILITY)stock.h
	@echo compiling stock program
	$(CC) $(CFLAGS) $(UTILITY)stock.c
	@mv stock.o $(UTILITY)
	@echo finished compiling
	@echo

socket_wrapper.o: $(WRAPPER)socket_wrapper.c $(WRAPPER)socket_wrapper.h
	@echo compiling socket wrapper program
	$(CC) $(CFLAGS) $(WRAPPER)socket_wrapper.c
	@mv socket_wrapper.o $(WRAPPER)
	@echo finished compiling
	@echo

ipc_wrapper.o: $(WRAPPER)ipc_wrapper.c $(WRAPPER)ipc_wrapper.h
	@echo compiling ipc wrapper program
	$(CC) $(CFLAGS) $(WRAPPER)ipc_wrapper.c
	@mv ipc_wrapper.o $(WRAPPER)
	@echo finished compiling
	@echo

clean:
	@echo cleaning project
	rm -rf *.o server client
	@echo finished cleaning project
