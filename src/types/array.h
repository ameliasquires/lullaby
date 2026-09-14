#pragma once
#include "stdlib.h"

typedef struct {
  void** arr;
  size_t len, size;
} array_t;

array_t* array_init();

void array_alloc(array_t* a, size_t extra);

void array_expand(array_t* a);

void array_push(array_t* a, void* val);

void* array_popi(array_t* a, size_t ind);

void* array_pop(array_t* a);

void array_free(array_t* a);
