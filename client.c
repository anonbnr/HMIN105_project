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
void helptext(){
	printf("add: used to add a new stock\n     syntax:'add [number] [product] [price]'\n\n");
	printf("addTo: used to add a number of items to an existing stock\n     syntax: addTo [product] [number]\n\n");
	printf("modifyPrice: used to change the price of an existing stock\n     syntax: modifyPrice [product] [new price]\n\n");
	printf("removeFrom: used to remove a number of items from an existing stock\n     syntax: removeFrom [product] [quantity]\n\n");
	printf("removeStock: used to completely remove an existing stock\n syntax:     removeStock [product]\n\n");
	printf("buy: buys an number elements from the specified stock\n syntax:     buy [qty] [product] from [producer]\n\n");
	printf("quit: removes your stocks and closes the app\n     syntax: quit\n\n");
	printf("display:  displays all the market\n    syntax: display\n\n");
	printf("help: shows a help message\n     syntax: help\n\n\n");

}
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

	//	for(int i = 0; i < size; i++)
	//check if the action (first element of the array) is valid
	for(int i = 0; i < ACTIONS_NUMBER; i++){
		if(!strcmp(action_list[i], fields[0])){
			//we found a valid action
			action_exists = 1;
			break;
		}
	}
	if(action_exists == 0){
		printf("Error: This action is not valid. Check the \"help\" command\n\n");
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
		printf("Error: Wrong argument number. Use \"help\" for reference\n\n");
		return -2;
	}


	//verify arguments type

	//add to, removeFrom, modifyPrice have 2 args. arg1 is a string, arg2 is a number (int for the first 2, float for the second)
	if(!strcmp(action, "addTo") || !strcmp(action, "removeFrom")){
		//arg 2 is an int
		int arg2len = strlen(fields[2]);
		if(isStringAnInt(fields[2], arg2len)==-3){
			printf("Error: Your second argument must be a positive integer\n\n");
			return -3;
		}

	}
	else
		if(!strcmp(action, "modifyPrice")){
			int arg2len = strlen(fields[2]);
			if(isStringADecimal(fields[2], arg2len)==-3){
				printf("Error: Your second argument must be a positive number\n\n");
				return -3;
			}
		}
		else
			if(!strcmp(action, "add")){
				if(isStringAnInt(fields[1], strlen(fields[1]))==-3){
					printf("Error: Your first argument must be  a positive integer\n\n");
					return -3;
				}
				if(isStringADecimal(fields[3], strlen(fields[3])) == -3){
					printf("Error: Your third argument must be  a positive integer\n\n");
					return -3;
				}
			}
			else
				if(!strcmp(action, "buy")){
					if(isStringAnInt(fields[1], strlen(fields[1]))==-3){
					printf("Error: Your first argument must be  a positive integer\n\n");
					return -3;
					}

					if(strcmp(fields[3], "from") != 0){
						printf("Error: Your third argument must be  a positive integer\n\n");
						return -3;
					}

				}
	if(!strcmp(action, "help")){
			helptext();
	}
	return 1;
}

/*
input function
*/

/*
UNUSED
*/
int input_action(char *action, char *action_saved){
	//char *action
	//printf("inside input action\n");
    printf("What do you want to do ?\n\n");
    if(fgets(action, MSG_SIZE+1, stdin) != NULL){
    	//no error from reading
    	//printf("Action string from begining of input_action: %s\n", action);

    	action = removeTrailingSlashN(action);
    	if(validate_action(action) == 1){
    		//action is valid, send it to the server
    		strcpy(action_saved, action);
			return 1;
	    	}
    }//fgets adds a /n at the end so i have to adjust the size for it
    else{
    	//error reading client text
    	printf("Internal error [fgets]. Application will now close...\n Sorry for the inconvenience\n\n");
    	exit(1);
    }
    return -1;
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

	while(strcmp(msg.text, "Bye")){
		recv_message(sockfd, &msg, sizeof(msg), 0, "Message reception error");
		printf("%s\n", msg.text);
		//empty message structure

	}
	printf("The program will now close...\n");
	exit(1);
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
		// printf("server socket address initialized successfully\n");

	//Connect to server
	connect_socket(sock, (struct sockaddr*) &server_address, sizeof(server_address), "Error connecting to server. Exiting...");

	message server_msg;
 	recv_message(sock, &server_msg, sizeof(server_msg), 0, "Message reception error");

 	//print message on screen
	printf("%s\n", server_msg.text);

 	do{
 	do{
 		printf("Please enter your pseudo: \n");
 		//Get client's pseudo after the server asks for it
 		fgets(pseudo, PSEUDO_SIZE, stdin);
 		//printf("pseudo before: %s", pseudo);
 	}while(validate_pseudo(pseudo) == -1);

 	//send pseudo to server
 	message msgpseudo;
 	strcpy(msgpseudo.pseudo, pseudo);
 	send_message(sock, &msgpseudo, sizeof(msgpseudo), 0, "Message sending error");

 	//receive validation message from server
 	recv_message(sock, &server_msg, sizeof(server_msg), 0, "Message reception error");

 	printf("%s\n", server_msg.text);
 	//while it's a negative response,ask the user for another input
 	}while(!strcmp(server_msg.text, "-1"));
 	//print message on screen

	//printf("%s: %s\n", server_msg.pseudo, server_msg.text);

 	//initialize parameter to be passed to the thread
 	thread_params *params = malloc(sizeof(thread_params));
 	params->sockfd = sock;
 	//Start reception thread
 	if (pthread_create(&idTh, NULL, receptionThread, params) != 0){
 		perror("Error in creating thread. Exiting...");
 		exit(6);
 	}

 	//initilize the action definition array
 	init_action();


 	//send messages
 	while(1){
 		char* action = malloc(MSG_SIZE * sizeof(char));
 		char *action_saved = malloc(MSG_SIZE * sizeof(char));
 		printf("What do you want to do ?\n");
    	if(fgets(action, MSG_SIZE+1, stdin) != NULL){
	    	//no error from reading
	    	action = removeTrailingSlashN(action);

	    	//continue if action is empty instead of crashing
	    	if(!strcmp(action,"")){
	    		continue;
	    	}

	    	strcpy(action_saved, action);

	    	if(validate_action(action) == 1){
	    		//action is valid, send it to the server
	    		//printf("Action saved from inside the if in main %s\n", action_saved);

	    		if(!strcmp(action, "help")){
	    			continue;
	    			printf("Help function\n");
	    		}
	    		message action_msg;
	    		strcpy(action_msg.pseudo, pseudo);
	    		strcpy(action_msg.text, action_saved);
					//printf("action message being sent\n");

	    		send_message(sock, &action_msg, sizeof(action_msg), 0, "Message sending error");


		    	}
    	}//fgets adds a /n at the end so i have to adjust the size for it

	    else{
	    	//error reading client text
	    	printf("Internal error [fgets]. Application will now close...\n Sorry for the inconvenience\n");
	    	exit(1);
	    }

	    //printf("Action saved at the end of thw while loop: %s\n", action_saved);
	 	}
}
