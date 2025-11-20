#include "str.h"
#include "../lua.h"
#include "../util.h"

#define alloc_buffer 64

str* str_initl(const char* init, size_t len){

  str* s = malloc(sizeof * s);
  s->_bytes = len + 1 + alloc_buffer;
  s->c = malloc(s->_bytes);
  if(s->c == NULL) p_fatal("failed to allocate string\n");
  s->len = len ;
  
  memcpy(s->c, init, (len + 1) * sizeof * init);
  return s;
}

str* str_initfl(const char* init, size_t len){

  str* s = malloc(sizeof * s);
  s->_bytes = len + 1 + alloc_buffer;
  s->c = malloc(s->_bytes);
  if(s->c == NULL) p_fatal("failed to allocate string\n");
  s->len = len ;
  
  memcpy(s->c, init, (len) * sizeof * init);
  s->c[len] = '\0';
  return s;
}

str* str_init(const char* init){
  return str_initl(init, strlen(init));
}

void str_free(str* s){
  free(s->c);
  free(s);
}

void str_push(str* s, const char* insert){
  s->len += strlen(insert);
  if(s->len + 1 >= s->_bytes)
    s->c = realloc(s->c, s->_bytes = s->len + 1 + alloc_buffer);
  strcat(s->c, insert);
}

void str_pushl(str* s, const char* insert, size_t l){
  if(s->len + l >= s->_bytes)
    s->c = realloc(s->c, s->_bytes = s->len + l + alloc_buffer);
  //strcat(s->c, insert);
  for(int i = 0; i != l; i++){
    s->c[i + s->len] = insert[i];
  }
  s->len += l;
  s->c[s->len] = '\0';
}

void str_clear(str* s){
  memset(s->c, 0, s->len);

  s->len = 0;
}

void str_popf(str* s, int len){
  memmove(s->c, s->c + len, s->len -= len);
  s->c[s->len] = 0;
}

void str_popb(str* s, int len){
  s->len -= len;
  s->len = s->len > 0 ? s->len : 0;
  s->c[s->len] = 0;
}


