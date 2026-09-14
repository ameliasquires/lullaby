#include "stdio.h"
#include "stdlib.h"
#include "array.h"

#define linear_expand 16

array_t* array_init(){
  array_t* a = calloc(1, sizeof * a);
  a->arr = calloc(a->size = linear_expand, sizeof * a->arr);

  return a;
}

void array_alloc(array_t* a, size_t extra){
  a->size += extra;
  a->arr = realloc(a->arr, a->size);
}

void array_expand(array_t* a){
  array_alloc(a, linear_expand);
}

void array_push(array_t* a, void* val){
  if(a->len + 1 > a->size){
    array_expand(a);
  }

  a->arr[a->len] = val;
  a->len++; 
}

void* array_popi(array_t* a, size_t ind){
  if(ind > a->len) return NULL;

  void* o = a->arr[ind];
  for(size_t i = ind; i < a->len - 1; i++){
    a->arr[i] = a->arr[i + 1]; 
  }
  a->arr[a->len - 1] = NULL;
  a->len--;

  return o;
}

void* array_pop(array_t* a){
  return array_popi(a, a->len - 1);
}

void array_free(array_t* a){
  free(a->arr);
  free(a);
}
