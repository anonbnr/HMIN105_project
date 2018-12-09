#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "utility.h"

char** split_string(char *str, char *delimiter, size_t* size){
	*size = 0;
  	int i = 0;
  	while(str[i] != '\0')
    	if(str[i++] == *delimiter)
      		*size = *size +1;
  	*size = *size +1;
  	char **array = malloc(*size * sizeof(char*));
  	char *rest = NULL;
  	char *token = NULL;
  	int counter = 0;

  	for (token = strtok_r(str, delimiter, &rest); token != NULL; token = strtok_r(NULL, delimiter, &rest)){
	    size_t token_size = strlen(token);
	    array[counter] = malloc((token_size+1) * sizeof(char));
	    strcpy(array[counter], token);
	    counter++;
  	}
  	return array;
}

void substring(const char *str, char *sub, int p, int l){
  size_t length = strlen(str);
  if (p >= 0 && p < length - 1 && l >= 1 && length >= p + l){
    int c = 0;

    while (c < l) {
       sub[c] = str[p+c];
       c++;
    }
    sub[c] = '\0';
  }
}
