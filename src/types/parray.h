
#ifndef __PARRAY_H
#define __PARRAY_H

typedef struct {
    void* value;
    str* key;
} pelem_t;

typedef struct {
    pelem_t* P;
    int len;
} parray_t;

enum free_type {
    NONE = 0, FREE = 1, STR = 2
};

parray_t* parray_init();
parray_t* parray_initl(int len);

void parray_set(parray_t*, char*, void*);
void parray_push(parray_t*, char*, void*);
void* parray_get(parray_t* , char*);
int parray_geti(parray_t* , char*);
void parray_remove(parray_t* p, char* key, enum free_type free);
void parray_clear(parray_t*, enum free_type);
void parray_lclear(parray_t*);
parray_t* parray_find(parray_t*, char*);
void free_method(void*, enum free_type);

#endif //parray_h
