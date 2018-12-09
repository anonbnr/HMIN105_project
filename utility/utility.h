#ifndef _UTILITY_H
#define _UTILITY_H

#define MSG_SIZE 1024
#define PSEUDO_SIZE 100

/*STRUCTURES*/
//message structure exchanged between client and server
typedef struct message {
  char text[MSG_SIZE];
  char pseudo[PSEUDO_SIZE];
} message;


/*FUNCTIONS*/

/*split function that splits a string into an array based on a delimiter*/
char** split_string(char *str, char *delimiter, size_t* size);
#endif //_UTILITY_H
