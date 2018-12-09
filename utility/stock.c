#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "stock.h"

/*FUNCTIONS*/
void init_empty_stock(stock *st){
  strcpy(st->producer, "");
  strcpy(st->name, "");
  st->price = INIT_NULL_VALUE;
  st->quantity = INIT_NULL_VALUE;
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

int is_null(stock *st){
  printf("st->price = %lf\n", st->price);
  printf("st->quantity = %d\n", st->quantity);
  // printf("strlen(st->producer) == 0 ? %d\n", strlen(st->producer) == 0);
  // printf("strlen(st->name) == 0 ? %d\n", strlen(st->name) == 0);
  // printf("st->price == %d ? %d\n", INIT_NULL_VALUE, st->price == INIT_NULL_VALUE);
  // printf("st->quantity == %d ? %d\n", INIT_NULL_VALUE, st->quantity == INIT_NULL_VALUE);
  return (strlen(st->producer) == 0) && (strlen(st->name) == 0) && (st->price == INIT_NULL_VALUE) && (st->quantity == INIT_NULL_VALUE);
}

char* stock_toString(stock *st){
  int price_length = snprintf(NULL, 0, "%f", st->price);
  int quantity_length = snprintf(NULL, 0, "%d", st->quantity);
  size_t size = strlen(st->producer) + strlen(st->name) + price_length + quantity_length + 13;
  char *result = malloc(size);
  sprintf(result, "[%s] %s %d %f euros.\n", st->producer, st->name, st->quantity, st->price);
  return result;
}
