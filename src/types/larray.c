#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>

#include "larray.h"

#define inc 4

larray_t* larray_initl(int len){
    larray_t* l = calloc(1, sizeof * l);
    l->size = len;
    l->arr = calloc(len, sizeof * l->arr);
    return l;
}

larray_t* larray_init(){
    return larray_initl(inc);
}

void larray_expand(larray_t** _l){
    larray_t* l = *_l;
    larray_t* remade = larray_initl(l->size * 4);
    for(int i = 0; i != l->size; i++){
        //what happens if the map_set calls map_regraph??? idk
        if(l->arr[i].used)
            larray_set(&remade, l->arr[i].idx, l->arr[i].value);
    }

    *_l = remade;
}

void larray_set(larray_t** _l, uint64_t idx, void* value){
    larray_t* l = *_l;

    if(l->len + 1 >= l->size){
        expand:
        larray_expand(&l);
    }

    uint64_t oind, ind = oind = idx % l->size;

    for(int count = 0; l->arr[ind].used && l->arr[ind].idx != idx; count++){
        ind++;
        if(ind >= l->size) ind = 0;
        if(ind == oind || count > 10) goto expand;
    }

    l->arr[ind].idx = idx;
    l->arr[ind].value = value;
    l->arr[ind].used = 1;
    l->len++;

    *_l = l;
}

int larray_geti(larray_t* l, uint64_t idx){
    uint64_t ind = idx % l->size;

    for(uint64_t initial = ind; ind != initial - 1;){
        if(!l->arr[ind].used) return -1;
        //printf("%s\n",M->M[ind].key->c);
        if(l->arr[ind].idx == idx) return ind;
        ind++;
        if(ind >= l->size) ind = 0;
    }
    return -1;
}

void* larray_get(larray_t* l, uint64_t idx){
    int r = larray_geti(l, idx);

    return r == -1 ? NULL : l->arr[r].value;
}

void larray_clear(larray_t* l){
    free(l->arr);
    free(l);
}

