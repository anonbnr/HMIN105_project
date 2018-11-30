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

whiteboard* init_wb(){
  /*key generation for whiteboard*/
  key_t wb_key = ipc_key_generation("ipc", SHM_WB_CODE, "whiteboard key generation error");

  /*whiteboard creation and internal id reception*/
  int wb_id = shm_id_reception(wb_key, sizeof(whiteboard), IPC_CREAT | 0666, "whiteboard creation error");
  printf("whiteboard id : %d\n", wb_id);

  /*whiteboard memory space attachment to server process*/
  whiteboard *wb = (whiteboard*) shm_attachment(wb_id, NULL, 0666, "whiteboard attachment error");
  printf("whiteboard attached successfully\n");

  /*whiteboard initialization*/
  strcpy(wb->content, ""); //le whiteboard est vide
  printf("whiteboard initialized successfully\n");

  return wb;
}

int* init_shm_clients(){
  /*key generation for shared memory segment (number of connected clients)*/
  key_t shm_clients_key = ipc_key_generation("ipc", SHM_CLIENTS_CODE, "shared segment for number of connected clients key generation error");

  /*shared memory segment (number of connected clients) creation and internal id reception*/
  int shm_clients_id = shm_id_reception(shm_clients_key, sizeof(int), IPC_CREAT | 0666, "shared segment for number of connected clients creation error");
  printf("shared segment for number of clients id : %d\n", shm_clients_id);

  /*shared memory segment (number of connected clients) space attachment to server process*/
  int *shm_clients = (int*) shm_attachment(shm_clients_id, NULL, 0666, "shared segment for number of connected clients attachment error");
  printf("shared segment for number of clients attached successfully\n");

  /*shared memory segment (number of connected clients) initialization*/
  *shm_clients = 0;
  printf("shared segment for number of clients initialized successfully\n");

  return shm_clients;
}

union semun init_sem_union(int sem_id){
  union semun unisem;

  unisem.value = 1;
  semctl(sem_id, 0, SETVAL, unisem); //initialization of whiteboard mutex

  unisem.value = 1;
  semctl(sem_id, 1, SETVAL, unisem); //initialization of concurrent access mutex

  unisem.value = 0;
  semctl(sem_id, 2, SETVAL, unisem); //initialization of server child process communication semaphore
  printf("semaphore array initialized successfully\n");

  return unisem;
}

int init_sem(){
  /*obtention d'une clé pour l'ensemble des sémaphores utilisées*/
  key_t sem_key = ipc_key_generation("ipc", SEM_CODE, "semaphore array key generation error");

  /*création d'un tableau de sémaphores et obtention de son identifiant interne*/
  int sem_id = sem_id_reception(sem_key, 3, IPC_CREAT | 0666, "semaphore array creation error");
  printf("semaphore array id : %d\n", sem_id);

  return sem_id;
}

void init_IPC(whiteboard *wb, int *shm_clients, int *sem_id, union semun *unisem){
  wb = init_wb(); //creation and initialization of whiteboard
  shm_clients = init_shm_clients(); //creation and initialization of shared segment memory for number of connected clients
  *sem_id = init_sem(); //creation of array of semaphores
  *unisem = init_sem_union(*sem_id); //initialization of array of sempahores buffer
}

int create_socket(int domaine, int type, int protocole, const char* error_msg){
  int server_socket_fd = socket(domaine, type, protocole);
  if(server_socket_fd == -1){
    perror(error_msg);
    exit(EXIT_FAILURE);
  }
  printf("server socket created successfully\n");

  return server_socket_fd;
}

struct sockaddr_in init_sockaddr(sa_family_t family, in_port_t port, uint32_t s_addr){
  struct sockaddr_in adr;
  adr.sin_family = family;
  adr.sin_port = htons(port);
  adr.sin_addr.s_addr = s_addr;

  printf("server socket address initialized successfully\n");
  return adr;
}

void bind_socket(int socket_fd, struct sockaddr* adr, socklen_t size, const char* error_msg){
  int bind_res = bind(socket_fd, adr, size);

  if(bind_res == -1){
    perror(error_msg);
    exit(EXIT_FAILURE);
  }

  printf("server socket address bound successfully\n");
}
void listen_socket(int socket_fd, int backlog, const char* error_msg){
  int listen_res = listen(socket_fd, backlog);
  if(listen_res == -1){
    perror(error_msg);
    exit(EXIT_FAILURE);
  }

  printf("server listening and awaiting connections...\n");
}

int accept_socket(int socket_fd, struct sockaddr *adr, socklen_t *size, const char* error_msg){
  int client_socket_fd = accept(socket_fd, adr, size);
  if(client_socket_fd <= 0){
    perror(error_msg);
    exit(EXIT_FAILURE);
  }

  return client_socket_fd;
}

void close_socket(int socket_fd, const char* error_msg){
  int close_res = close(socket_fd);
  if(close_res == -1){
    perror(error_msg);
    exit(EXIT_FAILURE);
  }
}

void send_message(int socket_fd, const void *buffer, size_t buffer_size, int permissions, const char* error_msg){
  int send_res = send(socket_fd, buffer, buffer_size, permissions);
  if(send_res == -1){
    perror(error_msg);
    exit(EXIT_FAILURE);
  }
}

void recv_message(int socket_fd, void *buffer, size_t buffer_size, int permissions, const char* error_msg){
  int recv_res = recv(socket_fd, buffer, buffer_size, permissions);
  if(recv_res == -1){
    perror(error_msg);
    exit(EXIT_FAILURE);
  }
}

int main(int argc, char* argv[]){

  whiteboard* wb = NULL;
  int *shm_clients = NULL, sem_id;
  union semun unisem;

  init_IPC(wb, shm_clients, &sem_id, &unisem); //creation and initialization of IPC objects used in the application

  pid_t ppid = getpid(), pid;
  printf("Server Parent Process : %d\n", ppid);

  /*server socket creation*/
  int server_socket_fd = create_socket(AF_INET, SOCK_STREAM, 0, "server socket creation error");

  /*server socket address initialization*/
  struct sockaddr_in server_sockaddr = init_sockaddr(AF_INET, PORT_NUMBER, INADDR_ANY);

  /*server socket binding*/
  bind_socket(server_socket_fd, (struct sockaddr*)&server_sockaddr, sizeof(server_sockaddr), "server socket binding error");

  /*server socket listening*/
  listen_socket(server_socket_fd, BACKLOG, "socket server listen error");

  /*server accepting connections*/
  struct sockaddr_in client_sockaddr;
  socklen_t client_sockaddr_size = sizeof(client_sockaddr);
  int client_socket_fd;

  while(1){
    client_socket_fd = accept_socket(server_socket_fd, (struct sockaddr*)&client_sockaddr, &client_sockaddr_size, "server accepting connections error");

    if ((pid = fork()) == 0){ //server child process for client handling
      close_socket(server_socket_fd, "server socket closing error");

      /*initializing server message and pseudo*/
      message server_msg;
      strcpy(server_msg.pseudo, "Server");
      sprintf(server_msg.text, "%s: Greetings! Please enter your pseudo (maximum of 100 characters) : ", server_msg.pseudo);

      send_message(client_socket_fd, &server_msg, sizeof(server_msg), 0, "Message sending error");

      /*receiving client message containing his pseudo*/
      message client_msg;
      recv_message(client_socket_fd, &client_msg, sizeof(client_msg), 0, "Message reception error");

      printf("client pseudo : %s\n", client_msg.pseudo);
    }
    else{
      // int client_fd_close_res = close(client_socket_fd);
      // if(client_fd_close_res == -1){
      //   perror("client socket closing error");
      //   exit(EXIT_FAILURE);
      // }
    }
  }

  return EXIT_SUCCESS;
}
