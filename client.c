#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <pthread.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/sem.h>
#include "client.h"
#include "socket_wrapper.h"
#include "utility.h"

/*
****************************
CURRENT PLAN FOR THE CLIENT
****************************

Main thread AND Emission thread
===========

1) ???create local WB???
4) Create socket address
5) Connect to server
2) Start Reception Thread
6) Ask the name of the client and save it in a variable
7) Send name to server

--
I started the threads at the beginning so that,
as soon as the client connects to the server, he can
receive the Welcome message.
--
--
I asked the client for his name in the main thread.
This is to append every message with the client's name
before it's sent to the server
--
This thread will be in an infinite while loop

.Wait for user input
.Create the message structure
.??Add it to the WB??
.Send it to the server
.Display it on the client's screen if it's sent

Reception Thread
===============
This thread will be in an infinite while loop

.Receive messages from server
.??Add it to the WB??
.display it
*/


//thread de reception

void *receptionThread(void *par){


	/*
		char *pos;
		if( (pos = strchr(pseudo, '\n')) != NULL)
			*pos = '\0';
	*/
	//get thread parameters
	thread_params *params = (thread_params*) par;
	int sockfd = params->sockfd;
	message msg;

	while(1){
		/*printf("Reception thread.\n");
		sleep(2);*/

		//receive message
		recv(sockfd, &msg, sizeof(msg), 0);

		/*
		//cut the last character from the pseudo and the message, which is a \n
		int sizepseudo = strlen(msg.pseudo);
		int sizetext = strlen(msg.text);

		printf("pseudo before: %s\n", msg.pseudo);
		printf("text before: %s\n", msg.text);
		char *pos;
		if( (pos = strchr(msg.pseudo, '\n')) != NULL)
			*pos = '\0';

		if( (pos = strchr(msg.text, '\n')) != NULL)
			*pos = '\0';
		*/
		//print message on screen
		printf("%s: %s\n", msg.pseudo, msg.text);
		//empty message structure

	}
}

int main(int argc, char* argv[]){
	pthread_t idTh;
	int sock;
	struct sockaddr_in server_address;
	char pseudo[PSEUDO_SIZE];


	//argument check

	//EXPAND THIS FUNCTION TO TEST THE TYPE OF EACH ARGUMENT
	if (argc != 3) {
	  perror("Invalid number of arguments. Exiting...");
	  exit(1);
	 }

	//Create client TCP socket
	sock = create_socket(AF_INET, SOCK_STREAM, 0, "client socket creation error");

	//Create address structure of the server based on parameters

	/*server_address.sin_family = AF_INET;
	server_address.sin_addr.s_addr= inet_addr(argv[1]);
	server_address.sin_port =  htons(argv[2]);
	*/

	server_address.sin_family = AF_INET;

	int s = inet_pton(AF_INET, argv[1], &server_address.sin_addr.s_addr);
	if(s <= 0){
       	if (s == 0){
            perror("Not a valid network address. Exiting...");
            exit(3);
       	}
        else{
            perror("Not a valid address family. Exiting...");
            exit(4);
        }
    }

    server_address.sin_port =  htons(atoi(argv[2]));
		printf("server socket address initialized successfully\n");

	//Connect to server
	connect_socket(sock, (struct sockaddr*) &server_address, sizeof(server_address), "Error connecting to server. Exiting...");

 	//initialize parameter to be passed to the thread
 	thread_params *params = malloc(sizeof(thread_params));
 	params->sockfd = sock;
 	//Start reception thread
 	if (pthread_create(&idTh, NULL, receptionThread, params) != 0){
 		perror("Error in creating thread. Exiting...");
 		exit(6);
 	}

 	//Get client's pseudo after the server asks for it
 	fgets(pseudo, PSEUDO_SIZE, stdin);
 	printf("pseudo before: %s", pseudo);
 	char *pos;
		if( (pos = strchr(pseudo, '\n')) != NULL)
			*pos = '\0';
	printf("pseudo after: %s\n", pseudo);
 	//test if it was read right
 	//printf("Hello %s, enjoy chatting and remember to remaind civil.\n", pseudo);

 	message msgpseudo;
 	strcpy(msgpseudo.pseudo, pseudo);
 	send(sock, &msgpseudo, sizeof(msgpseudo), 0);

 	while(1){

 	}

}
