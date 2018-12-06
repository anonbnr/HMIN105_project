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

#endif //_UTILITY_H
