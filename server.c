#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>
#include <pthread.h>
#include "server.h"
#include "wrapper/ipc_wrapper.h"
#include "wrapper/socket_wrapper.h"

/*FUNCTIONS*/
//ipc initialization functions
whiteboard* init_wb_shm(){
  /*key generation for whiteboard*/
  key_t wb_key = ipc_key_generation("ipc", WB_CODE, "whiteboard key generation error");

  /*whiteboard creation and internal id reception*/
  int wb_id = shm_id_reception(wb_key, sizeof(whiteboard), IPC_CREAT | 0666, "whiteboard creation error");
  printf("whiteboard id : %d\n", wb_id);

  /*whiteboard memory space attachment to server process*/
  whiteboard *wb = (whiteboard*) shm_attachment(wb_id, NULL, 0666, "whiteboard attachment error");
  printf("whiteboard attached successfully\n");

  /*whiteboard initialization*/
  init_whiteboard(wb);
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

  unisem.value = 1;
  semctl(sem_id, 3, SETVAL, unisem); //initialization of shared segment for connected clients mutex
  printf("semaphore array initialized successfully\n");

  return unisem;
}

int init_sem(){
  /*obtention d'une clé pour l'ensemble des sémaphores utilisées*/
  key_t sem_key = ipc_key_generation("ipc", SEM_CODE, "semaphore array key generation error");

  /*création d'un tableau de sémaphores et obtention de son identifiant interne*/
  int sem_id = sem_id_reception(sem_key, 4, IPC_CREAT | 0666, "semaphore array creation error");
  printf("semaphore array id : %d\n", sem_id);

  return sem_id;
}

void init_IPC(whiteboard **wb, int **shm_clients, int *sem_id, union semun *unisem){
  *wb = init_wb_shm(); //creation and initialization of whiteboard
  *shm_clients = init_shm_clients(); //creation and initialization of shared segment memory for number of connected clients
  *sem_id = init_sem(); //creation of array of semaphores
  *unisem = init_sem_union(*sem_id); //initialization of array of sempahores buffer
}

// void increment_connected_clients(int *shm_clients, int sem_id){
//   printf("Connected clients (before) : %d\n", *shm_clients);
//   struct sembuf op_buf;
//
//   /*initialization of the semaphore operation buffer for the wait semaphore operation*/
//   init_sem_op_buf(&op_buf, 3, -1, SEM_UNDO);
//
//   /*acquiring of the mutex on shared segment memory for connected clients*/
//   sem_operation(sem_id, &op_buf, 1, "semaphore wait operation error");
//
//   /*incrementing the number of clients*/
//   *shm_clients = *shm_clients + 1;
//
//   /*initialization of the semaphore operation buffer for the signal semaphore operation*/
//   init_sem_op_buf(&op_buf, 3, 1, SEM_UNDO);
//
//   /*liberation of the mutex on shared segment memory for connected clients*/
//   sem_operation(sem_id, &op_buf, 1, "semaphore signal operation error");
//
//   printf("Connected clients (after) : %d\n", *shm_clients);
// }
//
// void decrement_connected_clients(int *shm_clients, int sem_id){
//   printf("Connected clients (before) : %d\n", *shm_clients);
//   struct sembuf op_buf;
//
//   /*initialization of the semaphore operation buffer for the wait semaphore operation*/
//   init_sem_op_buf(&op_buf, 3, -1, SEM_UNDO);
//
//   /*acquiring of the mutex on shared segment memory for connected clients*/
//   sem_operation(sem_id, &op_buf, 1, "semaphore wait operation error");
//
//   /*decrementing the number of clients*/
//   *shm_clients = *shm_clients - 1;
//
//   /*initialization of the semaphore operation buffer for the signal semaphore operation*/
//   init_sem_op_buf(&op_buf, 3, 1, SEM_UNDO);
//
//   /*liberation of the mutex on shared segment memory for connected clients*/
//   sem_operation(sem_id, &op_buf, 1, "semaphore signal operation error");
//
//   printf("Connected clients (after) : %d\n", *shm_clients);
// }

//whiteboard functions
void init_whiteboard(whiteboard *wb){
  for (int i=0; i<MAX_STOCK; i++)
    init_empty_stock(&(wb->content[i]));
  wb->nb_stocks = 0;
}

char* get_whiteboard_content(whiteboard *wb){
  size_t size = 0;
  char* stock_output = NULL;
  for (int i=0; i<MAX_STOCK; i++){
    if (!is_null(&(wb->content[i]))){
      stock_output = stock_toString(&(wb->content[i]));
      size += strlen(stock_output);
      free(stock_output);
    }
  }

  if(size == 0)
    return "No products are available at the moment";

  char *result = malloc(size * sizeof(char));
  for (int i=0; i<MAX_STOCK; i++)
    if (!is_null(&(wb->content[i]))){
      stock_output = stock_toString(&(wb->content[i]));
      strcat(result, stock_output);
      free(stock_output);
    }

  return result;
}

void send_controlled_content(int client_socket_fd, const char* output, message *msg){
  size_t output_length = strlen(output);
  if(output_length <= MSG_SIZE){
    strcpy(msg->text, output);
    send_message(client_socket_fd, msg, sizeof(*msg), 0, "Message sending error");
  }
  else{
    size_t remaining_chars = strlen(output);
    int start = 0;
    while(remaining_chars > MSG_SIZE){
      substring(output, msg->text, start, MSG_SIZE);
      send_message(client_socket_fd, msg, sizeof(*msg), 0, "Message sending error");
      remaining_chars -= MSG_SIZE;
      start += MSG_SIZE;
    }

    if(remaining_chars != 0){
      substring(output, msg->text, start, remaining_chars);
      send_message(client_socket_fd, msg, sizeof(*msg), 0, "Message sending error");
    }
  }
}

void send_greeting_message(int client_socket_fd, whiteboard *wb, const char* server_pseudo, const char* client_pseudo){
  message msg;
  strcpy(msg.pseudo, server_pseudo);

  size_t size = snprintf(NULL, 0, "Hello \"%s\" and welcome to the open-market\n", client_pseudo);
  char *wb_content = get_whiteboard_content(wb);
  size += snprintf(NULL, 0, "Market Status:\n=============\n%s\n", wb_content);
  size += snprintf(NULL, 0, "\nuse the \"help\" command for a list of possible actions\n\n%s ", ACTION_INPUT_SYMBOL);
  size++; //adding space for '\0'

  char *wb_content_output = malloc(size * sizeof(char));
  sprintf(wb_content_output, "Hello \"%s\" and welcome to the open-market\nMarket Status:\n=============\n%s\nuse the \"help\" command for a list of possible actions\n\n%s ", client_pseudo, wb_content, ACTION_INPUT_SYMBOL);

  send_controlled_content(client_socket_fd, wb_content_output, &msg);
  free(wb_content);
  free(wb_content_output);
}

//add qty product_name price
char* add(whiteboard *wb, const char* client_pseudo, const char** args, int index){
  int quantity = (int) strtol(args[0], NULL, 10);
  double price = (double) strtod(args[2], NULL);

  init_stock(&(wb->content[index]), args[1], client_pseudo, price, quantity);

  size_t size = strlen(client_pseudo) + strlen(args[0]) + strlen(args[1]) + strlen(args[2]) + 37;
  char *return_msg = malloc(size * sizeof(char));
  sprintf(return_msg, "\"%s\" added a stock of %s \"%s\" for %s/piece.", client_pseudo, args[0], args[1], args[2]);
  return return_msg;
}

//addTo product_name qty
char* addTo(whiteboard *wb, const char* client_pseudo, const char** args, int index){
  int quantity = (int) strtol(args[1], NULL, 10);

  (wb->content[index]).quantity += quantity;

  size_t size = strlen(client_pseudo) + strlen(args[0]) + strlen(args[1]) + 25;
  char *return_msg = malloc(size * sizeof(char));
  sprintf(return_msg, "\"%s\" added %s to the \"%s\" stock.", client_pseudo, args[1], args[0]);
  return return_msg;
}

//removeFrom product_name qty
char* removeFrom(whiteboard *wb, const char* client_pseudo, const char** args, int index){
  int quantity = (int) strtol(args[1], NULL, 10);

  (wb->content[index]).quantity -= quantity;

  size_t size = strlen(client_pseudo) + strlen(args[0]) + strlen(args[1]) + 30;
  char *return_msg = malloc(size * sizeof(char));
  sprintf(return_msg, "\"%s\" removed %s from the \"%s\" stock.", client_pseudo, args[1], args[0]);
  return return_msg;
}

//modifyPrice product_name new_price
char* modifyPrice(whiteboard *wb, const char* client_pseudo, const char** args, int index){
  double new_price = strtod(args[1], NULL);
  double old_price = (wb->content[index]).price;
  int old_price_size = snprintf(NULL, 0, "%lf", old_price);

  (wb->content[index]).price = new_price;

  size_t size = strlen(client_pseudo) + strlen(args[0]) + strlen(args[1]) + old_price_size + 49;
  char *return_msg = malloc(size * sizeof(char));
  sprintf(return_msg, "\"%s\" changed the price of \"%s\" from %lf/piece to %lf/piece.", client_pseudo, args[0], old_price, new_price);
  return return_msg;
}

//removeStock product_name
char* removeStock(whiteboard *wb, const char* client_pseudo, const char** args, int index){
  empty_stock(&(wb->content[index]));

  size_t size = strlen(client_pseudo) + strlen(args[0]) + 26;
  char *return_msg = malloc(size * sizeof(char));
  sprintf(return_msg, "\"%s\" removed their \"%s\" stock.", client_pseudo, args[0]);
  return return_msg;
}

//buy qty product_name from producer_pseudo
char *buy(whiteboard *wb, const char* client_pseudo, const char** args, int index){
  int quantity = (int) strtol(args[0], NULL, 10);
  double price = (wb->content[index]).price;

  (wb->content[index]).quantity -= quantity;

  size_t size = strlen(client_pseudo) + strlen(args[0]) + strlen(args[1]) + strlen(args[3]) + 52;
  char *return_msg = malloc(size * sizeof(char));
  sprintf(return_msg, "\"%s\" bought %d pieces from the \"%s\" stock of \"%s\" at %lf/piece.", client_pseudo, quantity, args[1], args[3], price);
  return return_msg;
}

char* quit(whiteboard *wb, const char* client_pseudo) {
  for (int i=0; i<MAX_STOCK; i++)
    if(!is_null(&(wb->content[i])))
      if(!strcmp((wb->content[i]).producer, client_pseudo))
        empty_stock(&(wb->content[i]));
  size_t size = strlen(client_pseudo) + 59;
  char *return_msg = malloc(size * sizeof(char));
  sprintf(return_msg, "\"%s\" just quit the market and all of his stocks were removed", client_pseudo);
  return return_msg;
}

//add qty product_name price
int validate_add(whiteboard *wb, const char* client_pseudo, const char** args){

  return -1; //error
}

int validate_action(whiteboard *wb, char* action, const char* client_pseudo, const char** args, char* return_msg){
  //validate the values by accessing the whiteboard and verifying them accordingly
  if(!strcmp(action, "add")){
    //semaphore array needs to be handled here
    //call validate_add
  }

  else if(!strcmp(action, "addTo")){
    //semaphore array needs to be handled here
    //call validate_addTo
  }

  else if(!strcmp(action, "removeFrom")){
    //semaphore array needs to be handled here
    //call validate_removeFrom
  }

  else if(!strcmp(action, "modifyPrice")){
    //semaphore array needs to be handled here
    //call validate_modifyPrice
  }

  else if(!strcmp(action, "removeStock")){
    //semaphore array needs to be handled here
    //call validate_removeStock
  }

  else if(!strcmp(action, "buy")){
    //semaphore array needs to be handled here
    //call validate_buy
  }

  else if(!strcmp(action, "quit")){
    //semaphore array needs to be handled here
    strcpy(return_msg, quit(wb, client_pseudo));
    return 0; //success code
  }

  else if(!strcmp(action, "display")){
    //semaphore array needs to be handled here
    sprintf(return_msg, "%d stocks:\n", wb->nb_stocks);
    return 0; //success code
  }

  return -1; //error
}

int main(int argc, char* argv[]){

  whiteboard *wb = malloc(sizeof(whiteboard));
  int *shm_clients = malloc(sizeof(int));
  int sem_id;
  union semun unisem;

  /*creation and initialization of IPC objects used in the application*/
  init_IPC(&wb, &shm_clients, &sem_id, &unisem);
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

  while(1){
    int client_socket_fd = accept_socket(server_socket_fd, (struct sockaddr*)&client_sockaddr, &client_sockaddr_size, "server accepting connections error");
    if ((pid = fork()) == 0){ //server child process for client handling
      close_socket(server_socket_fd, "server socket closing error");
      // increment_connected_clients(shm_clients, sem_id);
      char quit_msg[2] = ""; //":q"

      /*initializing server message and pseudo*/
      message server_msg;
      strcpy(server_msg.pseudo, "Server");
      strcpy(server_msg.text, "Greetings! Please enter your pseudo (maximum of 100 characters, no spaces) : ");

      /*requesting pseudo from client*/
      send_message(client_socket_fd, &server_msg, sizeof(server_msg), 0, "Message sending error");

      /*receiving pseudo message from client*/
      message client_msg;
      recv_message(client_socket_fd, &client_msg, sizeof(client_msg), 0, "Message reception error");

      /*sending greeting message*/
      send_greeting_message(client_socket_fd, wb, server_msg.pseudo, client_msg.pseudo);

      do {
        //wait for actions sent by client
        recv_message(client_socket_fd, &client_msg, sizeof(client_msg), 0, "Message reception error");

        /**TODO
         * actions must be validated here (values only)
         * if an action is validated we execute it
         * else we send an error message to the client
        */


        //if client wants to quit the chatroom
        if(strcmp(quit_msg, client_msg.text) == 0){
          strcpy(server_msg.text, "Bye Bye...");
          send_message(client_socket_fd, &server_msg, sizeof(server_msg), 0, "Message sending error");
          //TODO remove all stocks related to client in the whiteboard
          close_socket(client_socket_fd, "client socket closing error");
          exit(EXIT_SUCCESS);
        }

        else{
          /*
          lock write mutex
            write the message on the whiteboard
            printf("CHATROOM\n========\n%s\n", *wb);
            lock semaphore of connected clients
              get number of connected clients
              initialize semaphore of communicating child processes to number of clients * 2
            unlock semaphore of connected clients
            lock semaphore of communicating child processes
            send whiteboard content
            wait for zero for semaphore communicating child processes
          */
        }

      } while(strcmp(quit_msg, client_msg.text));

    }
    else{
      // int client_fd_close_res = close(client_socket_fd);
      // if(client_fd_close_res == -1){
      //   perror("client socket closing error");
      //   exit(EXIT_FAILURE);
      // }
    }
  }

  free(wb);
  free(shm_clients);

  return EXIT_SUCCESS;
}
