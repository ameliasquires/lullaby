#include <stdio.h>
#include "i_str.h"
#include <stdlib.h>
#include <string.h>

#include "parray.h"

parray_t* parray_init(){
    parray_t* awa = malloc(sizeof * awa);
    awa->P = malloc(sizeof * awa->P);
    awa->len = 0;
    return awa;
}

void parray_set(parray_t* p, char* key, void* value){
    for(int i = 0; i != p->len; i++){
        if(strcmp(p->P[i].key->c, key) == 0){
            p->P[p->len - 1].value = value;
            return;
        }
    }

    p->len++;
    p->P = realloc(p->P, sizeof * p->P * (p->len + 1));
    p->P[p->len - 1].key = str_init(key);
    p->P[p->len - 1].value = value;
}

void* parray_get(parray_t* p, char* key){
    for(int i = 0; i != p->len; i++){
        if(strcmp(p->P[i].key->c, key) == 0){
            return p->P[i].value;
        }
    }
    return NULL;
}

void parray_clear(parray_t* p, int clear_val){
    for(int i = 0; i != p->len; i++){
        str_free(p[i].P->key);
        if(clear_val) free(p[i].P->value);
    }
    free(p->P);
    free(p);
}
