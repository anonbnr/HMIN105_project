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
  int value; /*value when cmd=SETVAL*/
  struct semid_ds *buf; /*buffer when cmd=IPC_STAT or cmd=IPC_SET*/
  unsigned short *array; /*array when cmd=GETALL or cmd=SETALL*/
  struct seminfo *__buf; /*buffer when cmd=IPC_INFO (Linux)*/
};

/*FUNCTIONS*/
//ipc initialization functions
key_t ipc_key_generation(const char* file, int code, const char* error_msg);
int shm_id_reception(key_t shm_key, size_t size, int permissions, const char* error_msg);
void *shm_attachment(int shm_id, const void *shmaddr, int permissions, const char* error_msg);
int sem_id_reception(key_t sem_key, size_t sems, int permissions, const char* error_msg);
void init_sem_op_buf(struct sembuf* op, unsigned short sem_num, short sem_op, short sem_flg);
void sem_operation(int sem_id, struct sembuf *sops, size_t nops, const char* error_msg);
whiteboard* init_wb();
int* init_shm_clients();
int init_sem();
union semun init_sem_union(int sem_id);
void init_IPC(whiteboard *wb, int *shm_clients, int *sem_id, union semun *unisem);
void increment_connected_clients(int *shm_clients, int sem_id);
void decrement_connected_clients(int *shm_clients, int sem_id);


#endif //_SERVER_H
