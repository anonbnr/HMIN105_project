#ifndef _IPC_WRAPPER_H
#define _IPC_WRAPPER_H
#include <sys/types.h>
#include <sys/sem.h>

/*FUNCTIONS*/
key_t ipc_key_generation(const char* file, int code, const char* error_msg); //a wrapper of ftok() with an error message in case of failure
int shm_id_reception(key_t shm_key, size_t size, int permissions, const char* error_msg); //a wrapper of shmget() with an error message in case of failure
void *shm_attachment(int shm_id, const void *shmaddr, int permissions, const char* error_msg); //a wrapper of shmat() with an error message in case of failure
int sem_id_reception(key_t sem_key, size_t sems, int permissions, const char* error_msg); //a wrapper of semget() with an error message in case of failure
void init_sem_op_buf(struct sembuf* op, unsigned short sem_num, short sem_op, short sem_flg); //initilization of a semaphore operation buffer
void sem_operation(int sem_id, struct sembuf *sops, size_t nops, const char* error_msg); //a wrapper of semop() with an error message in case of failure
void delete_shm(int shm_id, const char* error_msg);
void delete_semtable(int idSem, const char* error_msg);
#endif //_IPC_WRAPPER_H
