#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/sem.h>

#define SHM_CODE 66
#define SHM_SIZE 1000000
#define SEM_ACCESS_CODE 33
#define SEM_COMMUNICATION_CODE 11

int main(int argc, char* argv[]){

  /*obtention d'une clé pour le segment de mémoire partagée*/
  key_t shm_key = ftok("ipc", SHM_CODE);
  if(shm_key == -1){
    perror("shared segment key error");
    exit(EXIT_FAILURE);
  }

  /*création d'un segment de mémoire partagée et obtention de son identifiant interne*/
  int shm_id = shmget(shm_key, SHM_SIZE, IPC_CREAT | 0666);
  if(shm_id == -1){
    perror("shared segment creation error");
    exit(EXIT_FAILURE);
  }
  printf("shared segment id : %d\n", shm_id);

  /*obtention d'une clé pour l'ensemble des sémaphores pour l'accès concurrent*/
  key_t sem_key_access = ftok("ipc", SEM_ACCESS_CODE);
  if(sem_key_access == -1){
    perror("semaphore key (access) error");
    exit(EXIT_FAILURE);
  }

  /*création d'un ensemble de sémaphores pour l'accès concurrent et obtention de son identifiant interne*/
  int sem_id_access = semget(sem_key_access, 2, IPC_CREAT | 0666);
  if(sem_id_access == -1){
    perror("semaphore (access) creation error");
    exit(EXIT_FAILURE);
  }

  printf("semaphore access id : %d\n", sem_id_access);

  /*obtention d'une clé pour l'ensemble des sémaphores pour la communication entre processus fils*/
  key_t sem_key_communication = ftok("ipc", SEM_COMMUNICATION_CODE);
  if(sem_key_communication == -1){
    perror("semaphore key (communication) error");
    exit(EXIT_FAILURE);
  }

  /*création d'un ensemble de sémaphores pour la communication entre processus fils et obtention de son identifiant interne*/
  int sem_id_communication = semget(sem_key_communication, 4, IPC_CREAT | 0666); //le nombre de sémaphores est à vérifier
  if(sem_id_communication == -1){
    perror("semaphore (communication) creation error");
    exit(EXIT_FAILURE);
  }

  printf("semaphore communication id : %d\n", sem_id_communication);

  return EXIT_SUCCESS;
}
