#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>
#include <pthread.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/sem.h>

#define SHM_WB_CODE 66
#define SHM_CLIENTS_CODE 99
#define WHITEBOARD_SIZE 1000000
#define SEM_ACCESS_CODE 33
#define MSG_SIZE 1024
#define PSEUDO_SIZE 100
#define BUFFER_SIZE 10000

typedef struct message{
  char text[MSG_SIZE];
  char pseudo[PSEUDO_SIZE];
} message;

typedef struct w_buffer{
  char content[BUFFER_SIZE];
} w_buffer;

typedef struct whiteboard {
  char content[WHITEBOARD_SIZE];
  size_t readcount;
}

int main(int argc, char* argv[]){

  /*vérification du nombre de clients*/
  if(argc != 2){
    fprintf(stderr, "Erreur : syntaxe d'utilisation %s clients\n", argv[0]);
    exit(EXIT_FAILURE);
  }

  int nb_clients_max = atoi(argv[1]);
  pid_t ppid = getpid();

  /*obtention d'une clé pour le segment de mémoire partagée*/
  key_t shm_key = ftok("ipc", SHM_WB_CODE);
  if(shm_key == -1){
    perror("shared segment key error");
    exit(EXIT_FAILURE);
  }
  printf("shared segment key obtained successfully\n");

  /*obtention de la clé interne du segment de mémoire partagée désignant le whiteboard*/
  int shm_id = shmget(shm_key, WHITEBOARD_SIZE, 0666);
  if(shm_id == -1){
    perror("shared segment creation error");
    exit(EXIT_FAILURE);
  }
  printf("shared segment id : %d\n", shm_id);

  /*attacher le segment de mémoire partagée au processus du serveur*/
  char* wb = (char*) shmat(shm_id, NULL, 0666);
  if (wb == (char*) -1){
    perror("shmat error");
    exit(EXIT_FAILURE);
  }

  printf("shared segment attached successfully\n");

  /*obtention d'une clé pour le segment de mémoire partagée pour le nombre des clients*/
  key_t shm_clients_key = ftok("ipc", SHM_CLIENTS_CODE);
  if(shm_clients_key == -1){
    perror("shared segment for clients key error");
    exit(EXIT_FAILURE);
  }

  /*création d'un segment de mémoire partagée et obtention de son identifiant interne*/
  int shm_clients_id = shmget(shm_clients_key, sizeof(int), 0666);
  if(shm_clients_id == -1){
    perror("shared segment creation for clients error");
    exit(EXIT_FAILURE);
  }

  /*initializer le nombre de clients connectés au serveur*/
  int *nb_clients_connectes = shmat(shm_clients_id, NULL, 0666);
  if (nb_clients_connectes == (int*) -1){
    perror("shmat clients error");
    exit(EXIT_FAILURE);
  }

  *nb_clients_connectes = 0;
  printf("number of clients initialized successfully\n");
  printf("Server Parent Process : %d\n", ppid);

  /*initialization du whiteboard*/
  strcpy(wb, ""); //le whiteboard est vide
  printf("shared segment initialized successfully\n");
  pid_t pid;
  int fork_result = 0;
  message client_msg;
  char* msg = NULL;

  while(*nb_clients_connectes < nb_clients_max){
    if((fork_result = fork()) == 0){ //processus fils
      pid = getpid();
      printf("Processus fils : %d\n", pid);

      // printf("Veuillez insérer votre pseudo : ");
      // scanf("%s", client_msg.pseudo);
      // printf("Salut %s\n", client_msg.pseudo);
      printf("contenu du whiteboard avant : \n%s\n", wb);
      //
      // printf("Veuillez saisir un message : ");
      // scanf("%s", client_msg.text);
      //
      // msg = malloc(sizeof(client_msg.text) + sizeof(client_msg.pseudo) + 7*sizeof(ssize_t));
      // sprintf(msg, "%s [%ld] : %s\n", client_msg.pseudo, time(0), client_msg.text);
      //
      // strcat(wb, client_msg.text);
      // free(msg);
      char msg[MSG_SIZE];
      sprintf(msg, "%d\n", pid);
      strcat(wb, msg);
      printf("contenu du whiteboard après : \n%s\n", wb);
    }
    else
      *nb_clients_connectes = *nb_clients_connectes + 1;
  }
  return EXIT_SUCCESS;
}
