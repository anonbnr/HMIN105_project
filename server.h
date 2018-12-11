#ifndef _SERVER_H
#define _SERVER_H
#include "utility/stock.h"
#include "utility/utility.h"

/*CONSTANTS*/
#define WB_CODE 66
#define SHM_CLIENTS_CODE 99
#define SEM_CODE 33
#define PORT_NUMBER 31000
#define BACKLOG 10
#define MAX_STOCK 10000
#define MAX_CLIENT 200
#define PSEUDOS_FILE "pseudos.txt"

/*STRUCTURES*/
//whiteboard structure
/**
  a whiteboard is an array of MAX_STOCK stocks
  everytime an array is added/removed, nb_stocks is incremented/decremented
  before adding a stock, nb_stocks is always checked to see if it's <= MAX_STOCK
*/
typedef struct whiteboard {
  stock content[MAX_STOCK];
  int nb_stocks;
} whiteboard;

//union structure for semaphores
union semun {
  int value; /*value when cmd=SETVAL*/
  struct semid_ds *buf; /*buffer when cmd=IPC_STAT or cmd=IPC_SET*/
  unsigned short *array; /*array when cmd=GETALL or cmd=SETALL*/
  struct seminfo *__buf; /*buffer when cmd=IPC_INFO (Linux)*/
};

/*FUNCTIONS*/
/*ipc initialization functions*/
whiteboard* init_wb_shm(); //initialization of the whiteboard shared segment that returns a pointer to the whiteboard afterwards
int* init_shm_clients(); //initialization of the connected clients shared segment that returns a pointer to the segment afterwards
int init_sem(); //creation of a SV semaphore array that returns the semaphore array ID afterwards
union semun init_sem_union(int sem_id); //initialization of a SV semaphore array union buffer that returns the initialized buffer afterwards
void init_IPC(whiteboard **wb, int **shm_clients, int *sem_id, union semun *unisem); //initialization of all IPC objects required for the application

/*semaphore and shared segment memory functions*/
void lock_wb_w_mutex(int sem_id, int sem_num);
void unlock_wb_w_mutex(int sem_id, int sem_num);
void lock_connected_clients_mutex(int sem_id, int sem_num);
void unlock_connected_clients_mutex(int sem_id, int sem_num);
void increment_connected_clients(int *connected_clients, int sem_id);
void decrement_connected_clients(int *connected_clients, int sem_id);

// //pseudos file functions
// int validate_pseudo(const char* client_pseudo);
// void add_client(const char* client_pseudo);
// void remove_client(const char* client_pseudo);
// void display_clients();

/*server functions*/
void send_controlled_content(int client_socket_fd, const char* output, message *msg); //sends the contents of the whiteboard to the client
void send_greeting_message(int client_socket_fd, whiteboard *wb, int sem_id, const char* server_pseudo, const char* client_pseudo); //sends the greeting message to the client upon after receiving his pseudo

/*whiteboard functions*/
void init_whiteboard(whiteboard *wb); //initializes a whiteboard structure
char* get_whiteboard_content(whiteboard *wb); //gets the contents of the whiteboard as a string

//actions on the whiteboard
char* add(whiteboard *wb, const char* client_pseudo, char** args, int index); //add product_name qty price
char* addTo(whiteboard *wb, const char* client_pseudo, char** args, int index); //addTo product_name qty
char* removeFrom(whiteboard *wb, const char* client_pseudo, char** args, int index); //removeFrom product_name qty
char* modifyPrice(whiteboard *wb, const char* client_pseudo, char** args, int index); //modifyPrice product_name new_price
char* removeStock(whiteboard *wb, const char* client_pseudo, char** args, int index); //removeStock product_name
char* buy(whiteboard *wb, const char* client_pseudo, char** args, int index); //buy qty product_name from producer_pseudo
char* quit(whiteboard *wb, const char* client_pseudo);
char* execute_action(whiteboard *wb, int sem_id, char* action, char* client_pseudo);

//validation of the action functions
int validate_add(whiteboard *wb, const char* client_pseudo, char** args, char** return_msg);
int validate_addTo(whiteboard *wb, const char* client_pseudo, char** args, char** return_msg);
int validate_removeFrom(whiteboard *wb, const char* client_pseudo, char** args, char** return_msg);
int validate_modifyPrice(whiteboard *wb, const char* client_pseudo, char** args, char** return_msg);
int validate_removeStock(whiteboard *wb, const char* client_pseudo, char** args, char** return_msg);
int validate_buy(whiteboard *wb, const char* client_pseudo, char** args, char** return_msg);


#endif //_SERVER_H
