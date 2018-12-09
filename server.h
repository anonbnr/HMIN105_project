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
//ipc initialization functions
whiteboard* init_wb_shm(); //initialization of the whiteboard shared segment that returns a pointer to the whiteboard afterwards
int* init_shm_clients(); //initialization of the number of clients shared segment that returns a pointer to the shared segment afterwards
int init_sem(); //creation of a SV semaphore array that returns the semaphore array ID afterwards
union semun init_sem_union(int sem_id); //initialization of a SV semaphore array union buffer that returns the initialized buffer afterwards
void init_IPC(whiteboard *wb, int *shm_clients, int *sem_id, union semun *unisem); //initialization of all IPC objects required for the application
// void increment_connected_clients(int *shm_clients, int sem_id);
// void decrement_connected_clients(int *shm_clients, int sem_id);

//whiteboard functions
void init_whiteboard(whiteboard *wb); //initializes a whiteboard structure
char* display(whiteboard *wb);

#endif //_SERVER_H
