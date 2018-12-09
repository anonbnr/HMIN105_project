#include <stdio.h>
#include <stdlib.h>
#include "utility/stock.h"

int main(int argc, char* argv[]){

  stock st;
  init_empty_stock(&st);
  char *result = stock_toString(&st);
  printf("%s", result);
  init_stock(&st, "Bachar", "Tomatoes", 2, 10);
  result = stock_toString(&st);
  printf("%s", result);
  free(result);
  return EXIT_SUCCESS;
}
