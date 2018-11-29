#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/sem.h>
#include "init_IPC.h"

int main(int argc, char* argv[]){

  /*obtention d'une clé pour whiteboard*/
  key_t wb_key = ftok("ipc", SHM_WB_CODE);
  if(wb_key == -1){
    perror("whiteboard key error");
    exit(EXIT_FAILURE);
  }

  /*création d'un whiteboard et obtention de son identifiant interne*/
  int wb_id = shmget(wb_key, sizeof(whiteboard), IPC_CREAT | 0666);
  if(wb_id == -1){
    perror("whiteboard creation error");
    exit(EXIT_FAILURE);
  }
  printf("whiteboard id : %d\n", wb_id);

  /*obtention d'une clé pour le segment de mémoire partagée pour le nombre de clients*/
  key_t shm_clients_key = ftok("ipc", SHM_CLIENTS_CODE);
  if(shm_clients_key == -1){
    perror("shared segment for clients key error");
    exit(EXIT_FAILURE);
  }

  /*création d'un segment de mémoire partagée pour le nombre de clients et obtention de son identifiant interne*/
  int shm_clients_id = shmget(shm_clients_key, sizeof(int), IPC_CREAT | 0666);
  if(shm_clients_id == -1){
    perror("shared segment creation for clients error");
    exit(EXIT_FAILURE);
  }
  printf("shared segment for number of clients id : %d\n", shm_clients_id);

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

  /*initialization du tableau des sémaphores pour l'accès concurrent au whiteboard*/
  //initialization pour la sémaphore (mutex) utilisée en lecture des variables d'états*/
  union semun unisem;
  unisem.value = 1;

  semctl(sem_id_access, 0, SETVAL, unisem);

  unisem.value = 0;
  semctl(sem_id_access, 1, SETVAL, unisem);

  printf("semaphore array initialized successfully\n");

  return EXIT_SUCCESS;
}
