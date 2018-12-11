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
/*ipc initialization functions*/
whiteboard* init_wb_shm(){
  /*key generation for whiteboard*/
  key_t wb_key = ipc_key_generation("ipc", WB_CODE, "whiteboard key generation error");

  /*whiteboard creation and internal id reception*/
  int wb_id = shm_id_reception(wb_key, sizeof(whiteboard), IPC_CREAT | 0666, "whiteboard creation error");

  /*whiteboard memory space attachment to server process*/
  whiteboard *wb = (whiteboard*) shm_attachment(wb_id, NULL, 0666, "whiteboard attachment error");

  /*whiteboard initialization*/
  init_whiteboard(wb);

  return wb;
}

int* init_shm_clients(){
  /*key generation for connected clients shared segment*/
  key_t shm_clients_key = ipc_key_generation("ipc", SHM_CLIENTS_CODE, "connected clients shared segment key generation error");

  /*connected clients shared segment creation and internal id reception*/
  int shm_clients_id = shm_id_reception(shm_clients_key, sizeof(int), IPC_CREAT | 0666, "connected clients shared segment creation error");

  /*connected clients shared segment attachment to server process*/
  int *connected_clients = (int*) shm_attachment(shm_clients_id, NULL, 0666, "connected clients shared segment attachment error");

  /*connected clients shared segment initialization*/
  *connected_clients = 0;

  return connected_clients;
}

union semun init_sem_union(int sem_id){
  union semun unisem;

  unisem.value = 1;
  semctl(sem_id, 0, SETVAL, unisem); //initialization of whiteboard writing mutex

  unisem.value = 1;
  semctl(sem_id, 1, SETVAL, unisem); //initialization of whiteboard reading mutex

  unisem.value = 0;
  semctl(sem_id, 2, SETVAL, unisem); //initialization of server child process communication semaphore

  unisem.value = 1;
  semctl(sem_id, 3, SETVAL, unisem); //initialization of shared segment for connected clients mutex

  return unisem;
}

int init_sem(){
  /*obtention d'une clé pour l'ensemble des sémaphores utilisées*/
  key_t sem_key = ipc_key_generation("ipc", SEM_CODE, "semaphore array key generation error");

  /*création d'un tableau de sémaphores et obtention de son identifiant interne*/
  int sem_id = sem_id_reception(sem_key, 4, IPC_CREAT | 0666, "semaphore array creation error");
  // printf("semaphore array id : %d\n", sem_id);

  return sem_id;
}

void init_IPC(whiteboard **wb, int **shm_clients, int *sem_id, union semun *unisem){
  *wb = init_wb_shm(); //creation and initialization of whiteboard
  *shm_clients = init_shm_clients(); //creation and initialization of connected clients shared segment memory
  *sem_id = init_sem(); //creation of array of semaphores
  *unisem = init_sem_union(*sem_id); //initialization of array of sempahores buffer
}

/*semaphore and shared segment memory functions*/
void lock_wb_w_mutex(int sem_id, int sem_num){
  struct sembuf op_buf;

  /*initialization of the semaphore operation buffer for the wait semaphore operation*/
  init_sem_op_buf(&op_buf, sem_num, -1, SEM_UNDO);

  /*acquiring of the mutex on the whiteboard*/
  sem_operation(sem_id, &op_buf, 1, "semaphore wait operation error");
}

void unlock_wb_w_mutex(int sem_id, int sem_num){
  struct sembuf op_buf;

  /*initialization of the semaphore operation buffer for the signal semaphore operation*/
  init_sem_op_buf(&op_buf, sem_num, 1, SEM_UNDO);

  /*liberation of the mutex on the whiteboard*/
  sem_operation(sem_id, &op_buf, 1, "semaphore signal operation error");
}

void lock_connected_clients_mutex(int sem_id, int sem_num){
  struct sembuf op_buf;

  /*initialization of the semaphore operation buffer for the wait semaphore operation*/
  init_sem_op_buf(&op_buf, sem_num, -1, SEM_UNDO);

  /*acquiring of the mutex on shared segment memory for connected clients*/
  sem_operation(sem_id, &op_buf, 1, "semaphore wait operation error");
}

void unlock_connected_clients_mutex(int sem_id, int sem_num){
  struct sembuf op_buf;

  /*initialization of the semaphore operation buffer for the signal semaphore operation*/
  init_sem_op_buf(&op_buf, sem_num, 1, SEM_UNDO);

  /*liberation of the mutex on shared segment memory for connected clients*/
  sem_operation(sem_id, &op_buf, 1, "semaphore signal operation error");
}

void increment_connected_clients(int *connected_clients, int sem_id){
  lock_connected_clients_mutex(sem_id, 3);

  /*incrementing the number of clients*/
  *connected_clients += 1;

  unlock_connected_clients_mutex(sem_id, 3);

  printf("Connected clients: %d\n", *connected_clients);
}

void decrement_connected_clients(int *connected_clients, int sem_id){
  lock_connected_clients_mutex(sem_id, 3);

  /*incrementing the number of clients*/
  *connected_clients -= 1;

  unlock_connected_clients_mutex(sem_id, 3);

  printf("Connected clients: %d\n", *connected_clients);
}

//pseudos file functions
// int validate_pseudo(const char* client_pseudo){
//   //lock access to the file with a mutex
//   FILE* file = fopen(PSEUDOS_FILE, "r+");
//   if(file == NULL){
//     perror("clients pseudo file creation error");
//     exit(EXIT_FAILURE);
//   }
//
//   int result = 0;
//   char* pseudo = "";
//   while((fgets(pseudo, PSEUDO_SIZE, file)) != NULL){
//     if(!strcmp(pseudo, client_pseudo)){
//       result = -1;
//       break;
//     }
//     printf("pseudo in validate: %s\n", pseudo);
//   }
//   fclose(file);
//   //unlock access to the file
//   return result;
// }
//
// void add_client(const char* client_pseudo){
//   //lock access to the file with a mutex
//   FILE* file = fopen(PSEUDOS_FILE, "a");
//   if(file == NULL){
//     perror("clients pseudo file creation error");
//     exit(EXIT_FAILURE);
//   }
//
//   fputs(client_pseudo, file);
//   printf("successfully added \"%s\"\n", client_pseudo);
//   fclose(file);
//   //unlock access to the file
// }
//
// void remove_client(const char* client_pseudo){
//   //lock access to the file with a mutex
//   FILE* file = fopen(PSEUDOS_FILE, "r+");
//   if(file == NULL){
//     perror("clients pseudo file creation error");
//     exit(EXIT_FAILURE);
//   }
//
//   rewind(file);
//   char* pseudo = "";
//   char* pseudos = malloc(sizeof(pseudo));
//   while((fgets(pseudo, PSEUDO_SIZE, file)) != NULL)
//     if(strcmp(pseudo, client_pseudo)){
//       strcat(pseudos, pseudo);
//       realloc(pseudos, sizeof(pseudos) + sizeof(pseudo));
//     }
//   rewind(file);
//   fputs(pseudos, file);
//   printf("successfully removed \"%s\"\n", client_pseudo);
//   fclose(file);
//   //unlock access to the file
// }
//
// void display_clients(){
//   //lock access to the file with a mutex
//   printf("entered the display clients function\n");
//   FILE* file = fopen(PSEUDOS_FILE, "r");
//   if(file == NULL){
//     perror("clients pseudo file creation error");
//     exit(EXIT_FAILURE);
//   }
//
//   char* pseudo = "";
//   while((fgets(pseudo, PSEUDO_SIZE, file)) != NULL)
//     printf("%s\n", pseudo);
//
//   fclose(file);
//   //unlock access to the file
// }

/*server functions*/
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

void send_greeting_message(int client_socket_fd, whiteboard *wb, int sem_id, const char* server_pseudo, const char* client_pseudo){
  message msg;
  strcpy(msg.pseudo, server_pseudo);

  size_t size = snprintf(NULL, 0, "Hello \"%s\" and welcome to the open-market\n", client_pseudo);
  lock_wb_w_mutex(sem_id, 0);
  char *wb_content = get_whiteboard_content(wb);
  unlock_wb_w_mutex(sem_id, 0);
  size += snprintf(NULL, 0, "Market Status:\n=============\n%s\n", wb_content);
  size += strlen("\n\nuse the \"help\" command for a list of possible actions\n\n");
  size++; //adding space for '\0'

  char *wb_content_output = malloc(size * sizeof(char));
  sprintf(wb_content_output, "Hello \"%s\" and welcome to the open-market\nMarket Status:\n=============\n%s\n\n\nuse the \"help\" command for a list of possible actions\n\n", client_pseudo, wb_content);

  send_controlled_content(client_socket_fd, wb_content_output, &msg);
  free(wb_content_output);
  wb_content_output = NULL;
}

/*whiteboard functions*/
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
      stock_output = NULL;
    }
  }

  if(size == 0)
    return "No products are available at the moment";
  else
    size += snprintf(NULL, 0, "total: %d stocks\n", wb->nb_stocks);

  char *result = malloc(size * sizeof(char));
  for (int i=0; i<MAX_STOCK; i++)
    if (!is_null(&(wb->content[i]))){
      stock_output = stock_toString(&(wb->content[i]));
      strcat(result, stock_output);
      free(stock_output);
      stock_output = NULL;
    }

  size_t total_string_size = snprintf(NULL, 0, "total: %d stocks\n", wb->nb_stocks);
  char *total_string = malloc(total_string_size * sizeof(char));
  sprintf(total_string, "total: %d stocks\n", wb->nb_stocks);
  strcat(result, total_string);
  free(total_string);

  return result;
}

//actions on whiteboard
//add qty product_name price
char* add(whiteboard *wb, const char* client_pseudo, char** args, int index){
  int quantity = (int) strtol(args[0], NULL, 10);
  double price = (double) strtod(args[2], NULL);

  init_stock(&(wb->content[index]), args[1], client_pseudo, price, quantity);
  wb->nb_stocks += 1;

  size_t size = snprintf(NULL, 0, "\"%s\" added a stock of %s \"%s\" for %.2f euros/piece.", client_pseudo, args[0], args[1], price);
  char *return_msg = malloc(size * sizeof(char));
  sprintf(return_msg, "\"%s\" added a stock of %s \"%s\" for %.2f euros/piece.", client_pseudo, args[0], args[1], price);
  return return_msg;
}

//addTo product_name qty
char* addTo(whiteboard *wb, const char* client_pseudo, char** args, int index){
  int quantity = (int) strtol(args[1], NULL, 10);

  (wb->content[index]).quantity += quantity;

  size_t size = snprintf(NULL, 0, "\"%s\" added %s to the \"%s\" stock.", client_pseudo, args[1], args[0]);
  char *return_msg = malloc(size * sizeof(char));
  sprintf(return_msg, "\"%s\" added %s to the \"%s\" stock.", client_pseudo, args[1], args[0]);
  return return_msg;
}

//removeFrom product_name qty
char* removeFrom(whiteboard *wb, const char* client_pseudo, char** args, int index){
  int quantity = (int) strtol(args[1], NULL, 10);

  (wb->content[index]).quantity -= quantity;

  size_t size = snprintf(NULL, 0, "\"%s\" removed %s from the \"%s\" stock.", client_pseudo, args[1], args[0]);
  char *return_msg = malloc(size * sizeof(char));
  sprintf(return_msg, "\"%s\" removed %s from the \"%s\" stock.", client_pseudo, args[1], args[0]);
  return return_msg;
}

//modifyPrice product_name new_price
char* modifyPrice(whiteboard *wb, const char* client_pseudo, char** args, int index){
  double new_price = strtod(args[1], NULL);
  double old_price = (wb->content[index]).price;

  (wb->content[index]).price = new_price;

  size_t size = snprintf(NULL, 0, "\"%s\" changed the price of \"%s\" from %.2f/piece to %.2f/piece.", client_pseudo, args[0], old_price, new_price);
  char *return_msg = malloc(size * sizeof(char));
  sprintf(return_msg, "\"%s\" changed the price of \"%s\" from %.2f/piece to %.2f/piece.", client_pseudo, args[0], old_price, new_price);
  return return_msg;
}

//removeStock product_name
char* removeStock(whiteboard *wb, const char* client_pseudo, char** args, int index){
  empty_stock(&(wb->content[index]));

  wb->nb_stocks -= 1;

  size_t size = snprintf(NULL, 0, "\"%s\" removed their \"%s\" stock.", client_pseudo, args[0]);
  char *return_msg = malloc(size * sizeof(char));
  sprintf(return_msg, "\"%s\" removed their \"%s\" stock.", client_pseudo, args[0]);
  return return_msg;
}

//buy qty product_name from producer_pseudo
char *buy(whiteboard *wb, const char* client_pseudo, char** args, int index){
  int current_quantity = (wb->content[index]).quantity;
  int quantity = (int) strtol(args[0], NULL, 10);
  double price = (wb->content[index]).price;

  if(quantity <= current_quantity)
    (wb->content[index]).quantity -= quantity;
  else{
    size_t size = snprintf(NULL, 0, "Error: cannot purchase %d items from the \"%s\" stock of \"%s\" (current quantity: %d < requested quantity: %d)\n", quantity, args[1], args[2], current_quantity, quantity);
    char *return_msg = malloc(size * sizeof(char));
    sprintf(return_msg, "Error: cannot purchase %d items from the \"%s\" stock of \"%s\" (current quantity: %d < requested quantity: %d)\n", quantity, args[1], args[2], current_quantity, quantity);
    return return_msg;
  }

  size_t size = snprintf(NULL, 0, "\"%s\" bought %d pieces from the \"%s\" stock of \"%s\" at %.2f euros/piece.", client_pseudo, quantity, args[1], args[2], price);
  char *return_msg = malloc(size * sizeof(char));
  sprintf(return_msg, "\"%s\" bought %d pieces from the \"%s\" stock of \"%s\" at %.2f euros/piece.", client_pseudo, quantity, args[1], args[2], price);
  return return_msg;
}

char* quit(whiteboard *wb, const char* client_pseudo) {
  for (int i=0; i<MAX_STOCK; i++)
    if(!is_null(&(wb->content[i])))
      if(!strcmp((wb->content[i]).producer, client_pseudo)){
        empty_stock(&(wb->content[i]));
        wb->nb_stocks -= 1;
      }

  size_t size = snprintf(NULL, 0, "\"%s\" just quit the market and all of his stocks were removed", client_pseudo);
  char *return_msg = malloc(size * sizeof(char));
  sprintf(return_msg, "\"%s\" just quit the market and all of his stocks were removed", client_pseudo);
  return return_msg;
}

char* execute_action(whiteboard *wb, int sem_id, char* action, char* client_pseudo){
  char* notification_update = "";
  size_t action_size;
  char** action_array = split_string(action, " ", &action_size);
  char** args = NULL;
  if(action_size > 1)
    args = malloc((action_size-1) * sizeof(char*));

  lock_wb_w_mutex(sem_id, 0); //locking the whiteboard for writing
  // if(!strcmp(client_pseudo, "Joseph")){
  //   printf("sleeping for 10 seconds...\n");
  //   sleep(10);
  // }

  //start of critical section
  if(!strcmp(action_array[0], "add")){
    //arguments initialization
    args[0] = malloc(sizeof(action_array[1])+1);
    args[1] = malloc(sizeof(action_array[2])+1);
    args[2] = malloc(sizeof(action_array[3])+1);
    strcpy(args[0], action_array[1]); //adding quantity argument
    strcpy(args[1], action_array[2]); //adding product_name argument
    strcpy(args[2], action_array[3]); //adding price argument

    int validation_result = validate_add(wb, client_pseudo, args, &notification_update);

    if(validation_result >= 0){
      char* return_msg = add(wb, client_pseudo, args, validation_result);
      notification_update = malloc(strlen(return_msg) * sizeof(char));
      strcpy(notification_update, return_msg);
      free(return_msg);
      return_msg = NULL;
    }
  }

  else if(!strcmp(action_array[0], "addTo")){
    //arguments initialization
    args[0] = malloc(sizeof(action_array[1])+1);
    args[1] = malloc(sizeof(action_array[2])+1);
    strcpy(args[0], action_array[1]); //adding product_name argument
    strcpy(args[1], action_array[2]); //adding quantity argument

    int validation_result = validate_addTo(wb, client_pseudo, args, &notification_update);

    if(validation_result >= 0){
      char* return_msg = addTo(wb, client_pseudo, args, validation_result);
      notification_update = malloc(strlen(return_msg) * sizeof(char));
      strcpy(notification_update, return_msg);
      free(return_msg);
      return_msg = NULL;
    }
  }

  else if(!strcmp(action_array[0], "removeFrom")){
    //arguments initialization
    args[0] = malloc(sizeof(action_array[1])+1);
    args[1] = malloc(sizeof(action_array[2])+1);
    strcpy(args[0], action_array[1]); //adding product_name argument
    strcpy(args[1], action_array[2]); //adding quantity argument

    int validation_result = validate_removeFrom(wb, client_pseudo, args, &notification_update);

    if(validation_result >= 0){
      char* return_msg = removeFrom(wb, client_pseudo, args, validation_result);
      notification_update = malloc(strlen(return_msg) * sizeof(char));
      strcpy(notification_update, return_msg);
      free(return_msg);
      return_msg = NULL;
    }
  }

  else if(!strcmp(action_array[0], "modifyPrice")){
    //arguments initialization
    args[0] = malloc(sizeof(action_array[1])+1);
    args[1] = malloc(sizeof(action_array[2])+1);
    strcpy(args[0], action_array[1]); //adding product_name argument
    strcpy(args[1], action_array[2]); //adding new_price argument

    int validation_result = validate_modifyPrice(wb, client_pseudo, args, &notification_update);

    if(validation_result >= 0){
      char* return_msg = modifyPrice(wb, client_pseudo, args, validation_result);
      notification_update = malloc(strlen(return_msg) * sizeof(char));
      strcpy(notification_update, return_msg);
      free(return_msg);
      return_msg = NULL;
    }
  }

  else if(!strcmp(action_array[0], "removeStock")){
    //arguments initialization
    args[0] = malloc(sizeof(action_array[1])+1);
    strcpy(args[0], action_array[1]); //adding product_name argument

    int validation_result = validate_removeStock(wb, client_pseudo, args, &notification_update);

    if(validation_result >= 0){
      char* return_msg = removeStock(wb, client_pseudo, args, validation_result);
      notification_update = malloc(strlen(return_msg) * sizeof(char));
      strcpy(notification_update, return_msg);
      free(return_msg);
      return_msg = NULL;
    }
  }

  else if(!strcmp(action_array[0], "buy")){ //buy qty product_name from producer_pseudo
    //arguments initialization
    args[0] = malloc(sizeof(action_array[1])+1);
    args[1] = malloc(sizeof(action_array[2])+1);
    args[2] = malloc(sizeof(action_array[4])+1);
    strcpy(args[0], action_array[1]); //adding quantity argument
    strcpy(args[1], action_array[2]); //adding product_name argument
    strcpy(args[2], action_array[4]); //adding producer_pseudo argument

    int validation_result = validate_buy(wb, client_pseudo, args, &notification_update);

    if(validation_result >= 0){
      char* return_msg = buy(wb, client_pseudo, args, validation_result);
      notification_update = malloc(strlen(return_msg) * sizeof(char));
      strcpy(notification_update, return_msg);
      free(return_msg);
      return_msg = NULL;
    }
  }

  else if(!strcmp(action_array[0], "display")){
    char* return_msg = get_whiteboard_content(wb);
    notification_update = malloc(strlen(return_msg) * sizeof(char));
    strcpy(notification_update, return_msg);
  }

  if(args != NULL){
    free(args);
    args = NULL;
  }
  unlock_wb_w_mutex(sem_id, 0); //unlocking the whiteboard after writing
  return notification_update;
}

//validation of the action functions
//add qty product_name price
int validate_add(whiteboard *wb, const char* client_pseudo, char** args, char** return_msg){
  int free_index = 0;
  for (int i=0; i<MAX_STOCK; i++){
    if(!is_null(&(wb->content[i]))){
      if(!strcmp((wb->content[i]).producer, client_pseudo) && !strcmp((wb->content[i]).name, args[1])){
        size_t size = snprintf(NULL, 0, "Error: a \"%s\" stock already exists for \"%s\"\n", args[1], client_pseudo);
        *return_msg = malloc(size * sizeof(char));
        sprintf(*return_msg, "Error: a \"%s\" stock already exists for \"%s\"\n", args[1], client_pseudo);
        return -1;
      }
    }
    else
      free_index = i;
  }
  return free_index;
}

int validate_addTo(whiteboard *wb, const char* client_pseudo, char** args, char** return_msg){
  for (int i=0; i<MAX_STOCK; i++){
    if(!is_null(&(wb->content[i]))){
      if(!strcmp((wb->content[i]).producer, client_pseudo) && !strcmp((wb->content[i]).name, args[0]))
        return i;
    }
  }

  size_t size = snprintf(NULL, 0, "Error: \"%s\" stock does not exist for \"%s\"\n", args[0], client_pseudo);
  *return_msg = malloc(size * sizeof(char));
  sprintf(*return_msg, "Error: \"%s\" stock does not exist for \"%s\"\n", args[0], client_pseudo);
  return -1;
}

int validate_removeFrom(whiteboard *wb, const char* client_pseudo, char** args, char** return_msg){
  for (int i=0; i<MAX_STOCK; i++){
    if(!is_null(&(wb->content[i]))){
      if(!strcmp((wb->content[i]).producer, client_pseudo) && !strcmp((wb->content[i]).name, args[0])){
        int current_quantity = (wb->content[i]).quantity;
        int quantity = (int) strtol(args[1], NULL, 10);
        if(quantity <= current_quantity)
          return i;
        else{
          size_t size = snprintf(NULL, 0, "Error: cannot remove %d from the \"%s\" stock (current quantity: %d < requested quantity: %d)\n", quantity, args[0], current_quantity, quantity);
          *return_msg = malloc(size * sizeof(char));
          sprintf(*return_msg, "Error: cannot remove %d from the \"%s\" stock (current quantity: %d < requested quantity: %d)\n", quantity, args[0], current_quantity, quantity);
          return -1;
        }
      }
    }
  }

  size_t size = snprintf(NULL, 0, "Error: \"%s\" stock does not exist for \"%s\"\n", args[0], client_pseudo);
  *return_msg = malloc(size * sizeof(char));
  sprintf(*return_msg, "Error: \"%s\" stock does not exist for \"%s\"\n", args[0], client_pseudo);
  return -1;
}

int validate_modifyPrice(whiteboard *wb, const char* client_pseudo, char** args, char** return_msg){
  for (int i=0; i<MAX_STOCK; i++)
    if(!is_null(&(wb->content[i])))
      if(!strcmp((wb->content[i]).producer, client_pseudo) && !strcmp((wb->content[i]).name, args[0]))
        return i;

  size_t size = snprintf(NULL, 0, "Error: \"%s\" stock does not exist for \"%s\"\n", args[0], client_pseudo);
  *return_msg = malloc(size * sizeof(char));
  sprintf(*return_msg, "Error: \"%s\" stock does not exist for \"%s\"\n", args[0], client_pseudo);
  return -1;
}

int validate_removeStock(whiteboard *wb, const char* client_pseudo, char** args, char** return_msg){
  for (int i=0; i<MAX_STOCK; i++){
    if(!is_null(&(wb->content[i]))){
      if(!strcmp((wb->content[i]).producer, client_pseudo) && !strcmp((wb->content[i]).name, args[0]))
        return i;
    }
  }

  size_t size = snprintf(NULL, 0, "Error: \"%s\" stock does not exist for \"%s\"\n", args[0], client_pseudo);
  *return_msg = malloc(size * sizeof(char));
  sprintf(*return_msg, "Error: \"%s\" stock does not exist for \"%s\"\n", args[0], client_pseudo);
  return -1;
}

int validate_buy(whiteboard *wb, const char* client_pseudo, char** args, char** return_msg){
  for (int i=0; i<MAX_STOCK; i++){
    if(!is_null(&(wb->content[i]))){
      if(!strcmp(client_pseudo, args[2])){
        size_t size = snprintf(NULL, 0, "Error: \"%s\" cannot buy items from their own stocks\n", client_pseudo);
        *return_msg = malloc(size * sizeof(char));
        sprintf(*return_msg, "Error: \"%s\" cannot buy items from their own stocks\n", client_pseudo);
        return -1;
      }
      else if(!strcmp((wb->content[i]).producer, args[2]) && !strcmp((wb->content[i]).name, args[1]))
        return i;
    }
  }

  size_t size = snprintf(NULL, 0, "Error: either \"%s\" or a stock \"%s\" they own do not exist\n", args[2], args[1]);
  *return_msg = malloc(size * sizeof(char));
  sprintf(*return_msg, "Error: either \"%s\" or a stock \"%s\" they own do not exist\n", args[2], args[1]);
  return -1;
}

int main(int argc, char* argv[]){

  whiteboard *wb = malloc(sizeof(whiteboard));
  int *connected_clients = malloc(sizeof(int));
  int sem_id;
  union semun unisem;

  /*creation and initialization of IPC objects used in the application*/
  init_IPC(&wb, &connected_clients, &sem_id, &unisem);

  pid_t ppid = getpid(), pid;
  printf("Server Parent Process: %d\n", ppid);

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
      increment_connected_clients(connected_clients, sem_id);

      /*initializing server message and pseudo*/
      message server_msg;
      strcpy(server_msg.pseudo, "Server");
      strcpy(server_msg.text, "Greetings and Welcome to the open-market platform!");

      /*sending welcoming message*/
      send_message(client_socket_fd, &server_msg, sizeof(server_msg), 0, "Message sending error");

      /*receiving pseudo from client*/
      message client_msg;
      recv_message(client_socket_fd, &client_msg, sizeof(client_msg), 0, "Message reception error");
      // while(validate_pseudo(client_msg.pseudo) == -1){
      //   strcpy(server_msg.text, "-1");
      //   send_message(client_socket_fd, &server_msg, sizeof(server_msg), 0, "Message sending error");
      //   recv_message(client_socket_fd, &client_msg, sizeof(client_msg), 0, "Message reception error");
      // }
      //
      // add_client(client_msg.pseudo);
      // display_clients();
      //
      // while(1);

      /*sending greeting message*/
      send_greeting_message(client_socket_fd, wb, sem_id, server_msg.pseudo, client_msg.pseudo);

      do {
        //wait for actions sent by client
        char* notification_update = "";
        recv_message(client_socket_fd, &client_msg, sizeof(client_msg), 0, "Message reception error");

        //if client wants to quit the chatroom
        if(!strcmp(client_msg.text, "quit")){
          strcpy(server_msg.text, "Bye");
          send_message(client_socket_fd, &server_msg, sizeof(server_msg), 0, "Message sending error");
          close_socket(client_socket_fd, "client socket closing error (child process)");

          lock_wb_w_mutex(sem_id, 0);
          char* quit_return = quit(wb, client_msg.pseudo);
          unlock_wb_w_mutex(sem_id, 0);

          notification_update = malloc(strlen(quit_return) * sizeof(char));
          strcpy(notification_update, quit_return);
          printf("%s\n", notification_update);
          printf("%s\n", get_whiteboard_content(wb));

          decrement_connected_clients(connected_clients, sem_id);
          //broadcast the update through the thread
          exit(EXIT_SUCCESS);
        }

        else{
          notification_update = execute_action(wb, sem_id, client_msg.text, client_msg.pseudo);
          if(strstr(notification_update, "Error") != NULL)
            send_controlled_content(client_socket_fd, notification_update, &client_msg);
          else{
            //broadcast the updates
            printf("%s\n", notification_update);
            printf("%s\n", get_whiteboard_content(wb));
          }
        }

      } while(strcmp(client_msg.text, "quit"));

    }
    else
      close_socket(client_socket_fd, "client socket closing error (parent process)");
  }
  
  free(wb);
  wb = NULL;

  return EXIT_SUCCESS;
}
