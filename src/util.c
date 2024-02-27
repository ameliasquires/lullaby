#include "util.h"
#include <stdio.h>
#include <stdlib.h>
#include "lua.h"

int gen_parse(char* inp, int len, parray_t** _table){
  str* current = str_init(""), *last = NULL;
  int state = 0; 

  parray_t* table = *_table;
  for(int i = 0; i < len; i++){

    if(state != 1 && inp[i] == ';'){
      parray_set(table, last->c, (void*)current);
      str_free(last);
      last = NULL;
      current = str_init("");
      state = 0;
    } else if(state != 1 && inp[i] == '='){
      last = current;
      current = str_init("");
      if(inp[i+1] == '"'){
        state = 1;
        i++;
      }
    } else if(state == 1 && inp[i] == '"'){
      state = 0;
    } else if(current->c[0] != '\0' || inp[i] != ' ') str_pushl(current, inp + i, 1);
  }

  if(last != NULL){
    parray_set(table, last->c, (void*)current);
    str_free(last);
  }
  *_table = table;
  return 1;
}

char* strnstr(const char *s1, const char *s2, size_t n) {
  // simplistic algorithm with O(n2) worst case, stolen from stack overflow
  size_t i, len;
  char c = *s2;

  if(c == '\0')
    return (char *)s1;

  for(len = strlen(s2); len <= n; n--, s1++){
    if(*s1 == c){
      for(i = 1;; i++){
        if(i == len) return (char *)s1;
        if(s1[i] != s2[i]) break;
      }
    }
  }
  return NULL;
}

void p_fatal(const char* m){
  fprintf(stderr, "%s[ fatal ] %s %s\n",color_red, m, color_reset);
  exit(EXIT_FAILURE);
}

void p_error(const char* m){
  fprintf(stderr, "%s[ error ]%s %s\n",color_red, color_reset, m);
}
