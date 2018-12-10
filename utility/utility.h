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
char** split_string(char *str, char *delimiter, size_t* size); //split function that splits a string into an array based on a delimiter*/
void substring(const char *str, char *sub, int p, int l); //returns sub, a substring of s of length l starting at position p
/*split function that splits a string into an array based on a delimiter*/
char** split_string(char *str, char *delimiter, size_t* size);
/*test if a string is an integer*/
int isStringAnInt(char *string, int argsize);
/* test if a string is a decimal number*/
int isStringADecimal(char *string, int argsize);
/*remove the trailing n from a string and replace it with \0 */
char* removeTrailingSlashN(char *string);

#endif //_UTILITY_H
