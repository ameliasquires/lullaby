#include <stdio.h>
#include "str.h"
#include <stdlib.h>
#include <string.h>

#include "../lua.h"
#include "parray.h"

void free_method(void* v, enum free_type free_meth){
    if(v != NULL){
        if(free_meth == FREE) free(v);
        else if(free_meth == STR) str_free(v);
    }
}

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

int parray_geti(parray_t* p, char* key){
    for(int i = 0; i != p->len; i++){
        if(strcmp(p->P[i].key->c, key) == 0){
            return i;
        }
    }
    return -1;
}

void parray_remove(parray_t* p, char* key, enum free_type free){
    int ind = parray_geti(p, key);
    if(ind == -1) return;

    str_free(p->P[ind].key);
    free_method(p->P[ind].value, free);

    for(int i = ind; i < p->len - 1; i++) p->P[i] = p->P[i+1];
    p->len--;
    p->P = realloc(p->P, sizeof * p->P * (p->len + 1));
}

void parray_lclear(parray_t* p){
    free(p->P);
    free(p);
}

void parray_clear(parray_t* p, enum free_type clear_val){
    for(int i = 0; i != p->len; i++){
        str_free(p->P[i].key);
        free_method(p->P[i].value, clear_val);
    }
    parray_lclear(p);
}

int fmatch(char* s, char* p) {
    int slen = strlen(s);
    int plen = strlen(p);
    int sidx = 0, pidx = 0, lastWildcardIdx = -1, sBacktrackIdx = -1, nextToWildcardIdx = -1;
    for(;sidx < slen;) {
        if (pidx < plen && (p[pidx] == '?' || p[pidx] == s[sidx])) { 
            sidx++;
            pidx++;
        } else if (pidx < plen && p[pidx] == '*') { 
            lastWildcardIdx = pidx;
            nextToWildcardIdx = ++pidx;
            sBacktrackIdx = sidx;
        } else if (lastWildcardIdx == -1) { 
            return 0;
        } else { 
            pidx = nextToWildcardIdx;
            sidx = sBacktrackIdx++;
        }
    }
    for(int i = pidx; i < plen; i++) if(p[i] != '*') return 0;
    return 1;
}

parray_t* parray_find(parray_t* p, char* match){
    parray_t* ret = malloc(sizeof * ret);
    ret->P = malloc(sizeof * ret->P * p->len);
    ret->len = 0;
    for(int i = 0; i != p->len; i++){
        if(fmatch(match, p->P[i].key->c)){
            ret->P[ret->len] = p->P[i];
            ret->len++;
        }
    }
    return ret;
}
