#ifndef _CLIENT_H
#define _CLIENT_H



/*Global data for client*/
#define ACTIONS_NUMBER 9

typedef struct thread_params {
	int sockfd;
} thread_params;



/*FUNCTIONS*/
int validate_pseudo(char* pseudo);
void init_action();
int validate_action(char *action);
int input_action();
void helptext();
#endif //_CLIENT_H
