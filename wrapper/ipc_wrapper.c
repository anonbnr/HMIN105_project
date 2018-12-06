#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include "ipc_wrapper.h"

/*FUNCTIONS*/
key_t ipc_key_generation(const char* file, int code, const char* error_msg){
  key_t ipc_key = ftok(file, code);
  if (ipc_key == -1){
    perror(error_msg);
    exit(EXIT_FAILURE);
  }

  return ipc_key;
}

int shm_id_reception(key_t shm_key, size_t size, int permissions, const char* error_msg){
  int shm_id = shmget(shm_key, size, permissions);
  if(shm_id == -1){
    perror(error_msg);
    exit(EXIT_FAILURE);
  }

  return shm_id;
}

void *shm_attachment(int shm_id, const void *shmaddr, int permissions, const char* error_msg){
  void *shm = shmat(shm_id, shmaddr, permissions);
  if(shm == (void*) -1){
    perror(error_msg);
    exit(EXIT_FAILURE);
  }

  return shm;
}

int sem_id_reception(key_t sem_key, size_t sems, int permissions, const char* error_msg){
  int sem_id = semget(sem_key, sems, permissions);
  if(sem_id == -1){
    perror(error_msg);
    exit(EXIT_FAILURE);
  }

  return sem_id;
}

void init_sem_op_buf(struct sembuf* op, unsigned short sem_num, short sem_op, short sem_flg){
  op->sem_num = sem_num;
  op->sem_op = sem_op;
  op->sem_flg = sem_flg;
}

void sem_operation(int sem_id, struct sembuf *sops, size_t nops, const char* error_msg){
  int semop_res = semop(sem_id, sops, nops);
  if (semop_res == -1){
    perror(error_msg);
    exit(EXIT_FAILURE);
  }
}
