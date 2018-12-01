#ifndef _SERVER_H
#define _SERVER_H
#include <sys/types.h>
#include <netdb.h>
#include "utility.h"

#define SHM_WB_CODE 66
#define SHM_CLIENTS_CODE 99
#define SEM_CODE 33
#define PORT_NUMBER 31000
#define BACKLOG 10

/*STRUCTURE*/
//whiteboard structure
typedef struct whiteboard {
  char content[WHITEBOARD_SIZE];
} whiteboard;

//union structure for semaphores
union semun {
  int value; /*valeur quand cmd=SETVAL*/
  struct semid_ds *buf; /*tampon quand cmd=IPC_STAT ou cmd=IPC_SET*/
  unsigned short *array; /*tableau quand cmd=GETALL ou cmd=SETALL*/
  struct seminfo *__buf; /*tampon quand cmd=IPC_INFO (sous Linux)*/
};

/*FUNCTIONS*/
key_t ipc_key_generation(const char* file, int code, const char* error_msg);
int shm_id_reception(key_t shm_key, size_t size, int permissions, const char* error_msg);
void *shm_attachment(int shm_id, const void *shmaddr, int permissions, const char* error_msg);
int sem_id_reception(key_t sem_key, size_t sems, int permissions, const char* error_msg);
whiteboard* init_wb();
int* init_shm_clients();
int init_sem();
union semun init_sem_union(int sem_id);
void init_IPC(whiteboard *wb, int *shm_clients, int *sem_id, union semun *unisem);

#endif //_SERVER_H
