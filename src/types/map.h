
#ifndef _MAP_H
#define _MAP_H

#include <stdint.h>
#include "str.h"
#include "parray.h"

typedef struct {
    void* value;
    str* key;
    uint64_t hash;
    int used;
} melem_t;

typedef struct {
    melem_t* M;
    int len;
    int mod;
} map_t;

map_t* map_init();
void map_set(map_t**, char*, void*);
void* map_get(map_t* , char*);
int map_geti(map_t* , char*);
void map_remove(map_t* p, char* key, enum free_type free);
void map_clear(map_t*, enum free_type);
void map_lclear(map_t*);

#endif //_MAP_H