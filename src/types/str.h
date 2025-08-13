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

str* str_initl(const char*, size_t len);
//str_initfl has the 'correct' behaviour where it forces the len and doesnt read extra bytes
//plan to switch everything to str_initfl, when everything will work with it
str* str_initfl(const char*, size_t len);
str* str_init(const char*);
void str_free(str*);
void str_push(str*, const char*);
void str_pushl(str*, const char*, size_t);
void str_clear(str*);
void str_popf(str*, int);
void str_popb(str*, int);
#endif //__STR_H
