#ifndef _STOCK_H
#define _STOCK_H

/*CONSTANTS*/
#define MAX_PRODUCT_NAME 1024
#define INIT_NULL_VALUE -1

#include "utility.h"

/*STOCK STRUCTURE*/
/**
  a stock is identified by the product's name that it will stock, the producer and owner of the stock,
  the quantity of the product to stock, and the price per piece of the product
*/
typedef struct stock {
  char name[MAX_PRODUCT_NAME];
  char producer[PSEUDO_SIZE];
  double price;
  int quantity;
} stock;

/*FUNCTIONS*/
void init_empty_stock(stock *st); //initializes a stock as an empty stock
void init_stock(stock *st, const char* name, const char* producer, double price, int quantity); //initializes a stock with the provided fields' values
void empty_stock(stock *st); //empties the contents of a stock (equivalent to initializing a stock as an empty stock)
int is_null(stock *st); //checks if the stock structure is null
char* stock_toString(stock *st); //toString to display the contents of a stock

#endif //_STOCK_H
