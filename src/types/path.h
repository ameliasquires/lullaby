#include "array.h"
#include "str.h"

typedef struct {
  array_t* arr;
} path_t;

path_t* path_init();

void path_add(path_t* path, const char* str);
void path_calc(path_t* path);
void path_free(path_t* path);
str* path_tostr(path_t* path);
