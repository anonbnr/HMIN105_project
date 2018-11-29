#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <pthread.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/sem.h>
#include "init_IPC.h"
#include "server.h"

int main(int argc, char* argv[]){

  pid_t ppid = getpid();

  /*key for whiteboard*/
  key_t wb_key = ftok("ipc", SHM_WB_CODE);
  if(wb_key == -1){
    perror("whiteboard key error");
    exit(EXIT_FAILURE);
  }
  printf("whiteboard key obtained successfully\n");

  /*internal id of whiteboard*/
  int wb_id = shmget(wb_key, WHITEBOARD_SIZE, 0666);
  if(wb_id == -1){
    perror("whiteboard id error");
    exit(EXIT_FAILURE);
  }
  printf("whiteboard id : %d\n", wb_id);

  /*whiteboard memory space attachment to server process*/
  whiteboard* wb = (whiteboard*) shmat(wb_id, NULL, 0666);
  if (wb == (whiteboard*) -1){
    perror("whiteboard attachment error");
    exit(EXIT_FAILURE);
  }

  printf("whiteboard attached successfully\n");

  /*obtention d'une clé pour le segment de mémoire partagée pour le nombre des clients*/
  // key_t shm_clients_key = ftok("ipc", SHM_CLIENTS_CODE);
  // if(shm_clients_key == -1){
  //   perror("shared segment for clients key error");
  //   exit(EXIT_FAILURE);
  // }

  /*création d'un segment de mémoire partagée et obtention de son identifiant interne*/
  // int shm_clients_id = shmget(shm_clients_key, sizeof(int), 0666);
  // if(shm_clients_id == -1){
  //   perror("shared segment creation for clients error");
  //   exit(EXIT_FAILURE);
  // }

  /*initializer le nombre de clients connectés au serveur*/
  // int *nb_clients_connectes = shmat(shm_clients_id, NULL, 0666);
  // if (nb_clients_connectes == (int*) -1){
  //   perror("shmat clients error");
  //   exit(EXIT_FAILURE);
  // }

  // *nb_clients_connectes = 0;
  // printf("number of clients initialized successfully\n");

  /*whiteboard initialization*/
  strcpy(wb->content, ""); //le whiteboard est vide
  wb->readcount = 0; //il n'y a aucune lecture au début
  printf("whiteboard initialized successfully\n");

  printf("Server Parent Process : %d\n", ppid);
  pid_t pid;

  /*server socket creation*/
  int server_socket_fd = socket(AF_INET, SOCK_STREAM, 0);
  if(server_socket_fd == -1){
    perror("socket creation error");
    exit(EXIT_FAILURE);
  }

  printf("server socket created successfully\n");

  /*server socket binding*/
  struct sockaddr_in server_sockaddr;
  socklen_t server_sockaddr_size = sizeof(struct sockaddr_in);
  server_sockaddr.sin_family = AF_INET;
  server_sockaddr.sin_port = htonl(PORT_NUMBER);
  server_sockaddr.sin_addr.s_addr = INADDR_ANY;

  printf("server socket address initialized successfully\n");

  int bind_res = bind(server_socket_fd, (struct sockaddr*)&server_sockaddr, server_sockaddr_size);
  if(bind_res == -1){
    perror("server socket bind error");
    exit(EXIT_FAILURE);
  }

  printf("server socket address bound successfully\n");

  /*server socket listening*/
  int listen_res = listen(server_socket_fd, BACKLOG);
  if(listen_res == -1){
    perror("listen error");
    exit(EXIT_FAILURE);
  }

  printf("server listening and awaiting connections...\n");

  // /*accepting connections*/
  // struct sockaddr_in client_sockaddr;
  // socklen_t client_sockaddr_size = sizeof(struct sockaddr_in);
  // int client_socket_fd = accept(server_socket_fd, (struct sockaddr*)&client_socket_addr, &client_addr_size);
  // if(client_socket_fd <= 0){
  //   perror("accept error");
  //   exit(EXIT_FAILURE);
  // }

  return EXIT_SUCCESS;
}
