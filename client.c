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
#include <ctype.h>
#include "client.h"
#include "wrapper/socket_wrapper.h"
#include "utility/utility.h"

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


/*GLOBAL VARIABLE*/
char **action_list;

/*
FUNCTIONS
============
*/
void init_action(){
	action_list = malloc(ACTIONS_NUMBER*sizeof(char*));
	action_list[0] = "add";
	action_list[1] = "addTo";
	action_list[2] = "modifyPrice";
	action_list[3] = "removeFrom";
	action_list[4] = "removeStock";
	action_list[5] = "buy";
	action_list[6] = "quit";
	action_list[7] = "display";
	action_list[8] = "help";
}
/*
Function to validate the pseudo
only alphanumerical string allowed
No spaces
*/

int validate_pseudo(char* pseudo) {
    int pseudo_length;

    char* pseudo_no_n = removeTrailingSlashN(pseudo);

    pseudo_length=strlen(pseudo_no_n);
    if(pseudo_length > PSEUDO_SIZE){
    	printf("Your pseudo must be less than %d characters\n", PSEUDO_SIZE);
    		return -1;
    	}

    for (int i = 0; i < pseudo_length ; i++)
        if (!isalnum(pseudo_no_n[i])){
        	printf("'%c' is not an alphanumeric character.\n", pseudo_no_n[i]);
            return -1;
        }
    return 1;
}

/*
Function to validate the action that is
input by the user
*/
int validate_action(char *action){
  	char* delimiter = " ";
	size_t size;

	char** fields = split_string(action, delimiter, &size);
	int action_exists = 0;

	for(int i = 0; i < size; i++)
	//check if the action (first element of the array) is valid
	for(int i = 0; i < ACTIONS_NUMBER; i++){
		if(!strcmp(action_list[i], action)){
			//we found a valid action
			action_exists = 1;
			break;
		}
	}
	if(action_exists == 0){
		printf("This action is not valid. Check the \"help\" command\n");
		return -1;

	}

	//check number of arguments
	/*
	add: 3
	addTo, modifyPrice, removeFrom: 2
	removeStock: 1
	buy: 4
	quit, display, help: 0
	*/	
	int number_args;
	//get the number of arguments needed for the current action
	if(!strcmp(action, "quit")||!strcmp(action, "display")||!strcmp(action, "help"))
		number_args = 0;
	else
		if(!strcmp(action, "removeStock"))
			number_args = 1;
		else
			if (!strcmp(action, "addTo") || !strcmp(action, "modifyPrice") || !strcmp(action, "removeFrom"))
				number_args = 2;
			else
				if(!strcmp(action, "add"))
					number_args = 3;
					else
					if(!strcmp(action, "buy"))
					number_args = 4;

	//incorrect number of arguments
	if(size != number_args+1){
		printf("Wrong argument number. Use \"help\" for reference\n");
		return -2;
	}


	//verify arguments type

	//add to, removeFrom, modifyPrice have 2 args. arg1 is a string, arg2 is a number (int for the first 2, float for the second)
	if(!strcmp(action, "addTo") || !strcmp(action, "removeFrom")){
		//arg 2 is an int
		int arg2len = strlen(fields[2]);
		if(isStringAnInt(fields[2], arg2len)==-3){
			printf("Your second argument must be an integer\n");
			return -3;
		}

	}
	else
		if(!strcmp(action, "modifyPrice")){
			int arg2len = strlen(fields[2]);
			if(isStringADecimal(fields[2], arg2len)==-3){
				return -3;
			}
		}
		else
			if(!strcmp(action, "add")){
				if(isStringAnInt(fields[1], strlen(fields[1]))==-3){
					return -3;
				}
				if(isStringADecimal(fields[3], strlen(fields[3])) == -3){
					return -3;
				}
			}
			else
				if(!strcmp(action, "buy")){
					if(isStringAnInt(fields[1], strlen(fields[1]))==-3){
					return -3;
					}

					if(strcmp(fields[3], "from") != 0)
						return -3;
				}
	return 1;
}

/*
input function
*/

/*
Typically, the safest and easiest way to use fgets is to allocate a single, large-enough line buffer. Use that to read the line, then copy it into correctly sized buffers.

https://stackoverflow.com/questions/43813594/getting-input-with-fgets-in-a-loop
*/
void input_action(){
	char *action = malloc(MSG_SIZE * sizeof(char));
	//printf("inside input action\n");
  	//do{
    	printf("What do you want to do ?\n");
    	if(fgets(action, MSG_SIZE+1, stdin) != NULL){
    	} //fgets adds a /n at the end so i have to adjust the size for it
  	//}while(validate_action(action) != 1);

    validate_action(action);
}

/*
THREAD
=======
*/
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

		//printf("while loop\n");
		//receive message
		recv_message(sockfd, &msg, sizeof(msg), 0, "Message reception error");

		//printf("message received\n");
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

/*
MAIN
====
*/
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

	message server_msg;
 	recv_message(sock, &server_msg, sizeof(server_msg), 0, "Message reception error");


 	//ADD A SECOND DO WHILE THAT SURROUNDS THIS ONE
 	//AS LONG AS THE MESSAGE RECEIVED BY THE SERVER IS -1, KEEP REPEATING THIS SAME PROCESS
 	do{
 		printf("Please enter your pseudo: \n");
 		//Get client's pseudo after the server asks for it
 		fgets(pseudo, PSEUDO_SIZE, stdin);
 		//printf("pseudo before: %s", pseudo);
 	}while(validate_pseudo(pseudo) == -1);
 	
 	message msgpseudo;
 	strcpy(msgpseudo.pseudo, pseudo);
 	send_message(sock, &msgpseudo, sizeof(msgpseudo), 0, "Message sending error");
 	
 	recv_message(sock, &server_msg, sizeof(server_msg), 0, "Message reception error");

 	//initialize parameter to be passed to the thread
 	thread_params *params = malloc(sizeof(thread_params));
 	params->sockfd = sock;
 	//Start reception thread
 	if (pthread_create(&idTh, NULL, receptionThread, params) != 0){
 		perror("Error in creating thread. Exiting...");
 		exit(6);
 	}

 	
 	init_action();
 	//receive messages
 	while(1){
 		input_action();
 	}
}
