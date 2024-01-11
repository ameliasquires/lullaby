#include "i_str.h"

str* str_init(char* init){
  if(init == NULL){
    char cc = '\0';
    init = &cc;
  }

  size_t len = strlen(init);
  str* s = malloc(sizeof * s);
  s->c = malloc(len + 1);
  s->len = len ;
  
  memcpy(s->c, init, len + 1);
  return s;
}

void str_free(str* s){
  free(s->c);
  return free(s);
}

void str_push(str* s, char* insert){
  s->len += strlen(insert);
  s->c = realloc(s->c, s->len + 5);
  strcat(s->c, insert);
}

void str_clear(str* s){
  memset(s->c, 0, s->len);

  s->len = 0;
}

