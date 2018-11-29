#ifndef _INIT_IPC_H
#define _INIT_IPC_H

#define SHM_WB_CODE 66
#define SHM_CLIENTS_CODE 99
#define WHITEBOARD_SIZE 1000000
#define SEM_ACCESS_CODE 33

//whiteboard structure
typedef struct whiteboard {
  char content[WHITEBOARD_SIZE];
  size_t readcount;
} whiteboard;

//union structure for semaphores
union semun {
  int value; /*valeur quand cmd=SETVAL*/
  struct semid_ds *buf; /*tampon quand cmd=IPC_STAT ou cmd=IPC_SET*/
  unsigned short *array; /*tableau quand cmd=GETALL ou cmd=SETALL*/
  struct seminfo *__buf; /*tampon quand cmd=IPC_INFO (sous Linux)*/
};

#endif //_INIT_IPC_H
