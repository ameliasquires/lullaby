#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "path.h"

path_t* path_init(){
  path_t* path = calloc(1, sizeof * path);
  path->arr = array_init();

  return path;
}

void path_add(path_t* path, const char* str){
  if(str == NULL) return;
  if(str[0] == '/') str++;

  const char* next;
  for(;;){
    next = strstr(str, "/");
    if(next == NULL) break;
    if(next - str == 0){
      str++;
      continue;
    }

    char* item = calloc(next - str + 1, sizeof * item);
    memcpy(item, str, next - str);
    array_push(path->arr, item);

    str = next + 1;
  }


  size_t l = strlen(str);
  if(l > 0){
    char* item = calloc(l + 1, sizeof * item);
    memcpy(item, str, l);
    array_push(path->arr, item);
  }
}

void path_calc(path_t* path){
  for(ssize_t i = 0; i != path->arr->len; i++){
    if(strcmp(path->arr->arr[i], ".") == 0){
      char* d = array_popi(path->arr, i);
      free(d);
      i--;
    } else if(strcmp(path->arr->arr[i], "..") == 0){
      char* a = array_popi(path->arr, i);
      char* b = array_popi(path->arr, i - 1);

      free(a);
      if(b != NULL) free(b);

      i-=2;
    }

    if(i < -1) i = -1;
  }
}

str* path_tostr(path_t* path){
  str* s = str_init("");
  
  for(size_t i = 0; i != path->arr->len; i++){
    str_push(s, "/");
    str_push(s, path->arr->arr[i]);
  }

  if(s->len == 0) str_push(s, "/");

  return s;
}

void path_free(path_t* path){
  for(size_t i = 0; i != path->arr->len; i++){
    free(path->arr->arr[i]);
  }

  array_free(path->arr);
  free(path);
}
