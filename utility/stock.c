#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "stock.h"

/*FUNCTIONS*/
void init_empty_stock(stock *st){
  memset(st, 0, sizeof(stock));
}

void init_stock(stock *st, const char* name, const char* producer, double price, int quantity){
  strcpy(st->producer, producer);
  strcpy(st->name, name);
  st->price = price;
  st->quantity = quantity;
}

void empty_stock(stock *st){
  init_empty_stock(st);
}

char* stock_toString(stock *st){
  int price_length = snprintf(NULL, 0, "%f", st->price);
  int quantity_length = snprintf(NULL, 0, "%d", st->quantity);
  size_t size = strlen(st->producer) + strlen(st->name) + price_length + quantity_length + 13;
  char *result = malloc(size);
  sprintf(result, "[%s] %s %d %f euros.\n", st->producer, st->name, st->quantity, st->price);
  return result;
}
