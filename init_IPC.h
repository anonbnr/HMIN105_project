#ifndef _INIT_IPC_H
#define _INIT_IPC_H

#define WHITEBOARD_SIZE 1000000
#define MSG_SIZE 1024
#define PSEUDO_SIZE 100

/*STRUCTURES*/
//message structure exchanged between client and server
typedef struct message {
  char text[MSG_SIZE];
  char pseudo[PSEUDO_SIZE];
} message;

#endif //_INIT_IPC_H
