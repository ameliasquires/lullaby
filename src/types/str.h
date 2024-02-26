#ifndef __STR_H
#define __STR_H

#include <string.h> 
#include <stdio.h>
#include <stdlib.h>

typedef struct {
  size_t len;
  size_t _bytes; //may be used in the future
  char* c; 
} str;

str* str_init(char*);
void str_free(str*);
void str_push(str*, char*);
void str_pushl(str*, char*, size_t);
void str_clear(str*);

#endif //__STR_H