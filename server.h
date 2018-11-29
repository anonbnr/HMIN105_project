#ifndef _SERVER_H
#define _SERVER_H

#define MSG_SIZE 1024
#define PSEUDO_SIZE 100
#define PORT_NUMBER 31000
#define BACKLOG 10

/*structure du message reçu par le client*/
typedef struct message {
  char text[MSG_SIZE];
  char pseudo[PSEUDO_SIZE];
} message;

#endif //_SERVER_H
