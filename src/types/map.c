#include "map.h"
#include "../hash/fnv.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#define mod_inc 4

uint64_t hash(char* c, size_t len){
    return fnv_1((uint8_t*)c, len, v_a);
}

void map_dump(map_t* M){
    printf("---\n%i %i\n- **\n",M->mod, M->len);
    for(int i = 0; i != M->mod; i++){
        if(M->M[i].used){
            printf("%i | %s : %p\n",i,M->M[i].key->c, M->M[i].value);
        } 
    }
}

map_t* map_initl(size_t len){
    map_t* awa = calloc(sizeof * awa, 1);
    awa->M = calloc(sizeof * awa->M, len);
    //for(int i = 0; i != len; i++) awa->M[i].used = 0;
    awa->len = 0;
    awa->mod = len;
    return awa;
}

map_t* map_init(){
    return map_initl(4);
}

void map_expand(map_t** _M){
    map_t* M = *_M;
    map_t* remade = map_initl(M->mod * 4);
    for(int i = 0; i != M->mod; i++){
        //what happens if the map_set calls map_regraph??? idk
        if(M->M[i].used)
            map_set(&remade, M->M[i].key->c, M->M[i].value);
    }

    *_M = remade;
}

void map_set(map_t** _M, char* key, void* value){
    map_t* M = *_M;
    uint64_t h = hash(key, strlen(key));

    if(M->len + 1 >= M->mod){
        expand:
        map_expand(&M);
    }
    uint64_t ind = h % M->mod;
    uint64_t oind = ind;

    //iterates until there is a free space
    for(int count = 0; M->M[ind].used && M->M[ind].hash != h && strcmp(M->M[ind].key->c, key) != 0; count++){
        ind++;
        if(ind >= M->mod) ind = 0;
        if(ind == oind || count > 10) goto expand;
    }

    M->M[ind].hash = h;
    M->M[ind].key = str_init(key);
    M->M[ind].value = value;
    M->M[ind].used = 1;
    M->len++;

    *_M = M;
}

int map_geti(map_t* M, char* key){
    uint64_t h = hash(key, strlen(key));
    uint64_t ind = h % M->mod;

    for(uint64_t initial = ind; ind != initial - 1;){
        if(M->M[ind].key == NULL) return -1;
        //printf("%s\n",M->M[ind].key->c);
        if(M->M[ind].hash == h && strcmp(M->M[ind].key->c, key)==0) return ind;
        ind++;
        if(ind >= M->mod) ind = 0;
    }
    return -1;
}

void* map_get(map_t* M, char* key){
    int r = map_geti(M, key);
    //printf("%i\n",r);
    return r == -1? NULL : M->M[r].value;
}

void map_remove(map_t* p, char* key, enum free_type free){
    int ind = map_geti(p, key);
    if(ind == -1) return;
    p->M[ind].used = 0;
    p->M[ind].hash = 0;
    str_free(p->M[ind].key);
    free_method(p->M[ind].value, free);
}

void map_lclear(map_t* M){
    free(M->M);
    free(M);
}

void map_clear(map_t* M, enum free_type free){
    for(int i = 0; i != M->mod; i++){
        if(M->M[i].used){
            str_free(M->M[i].key);
            free_method(M->M[i].value, free);
        }
    }
    map_lclear(M);
}

int main(){
    int i = 5;
    int b = 24;
    int c = 9;
    map_t* m = map_init();
    
    map_set(&m, "wowa", &b);
    printf("%i\n",*(int*)map_get(m, "wowa"));
    map_set(&m, "aw", &i);
    map_set(&m, "awc", &i);
    map_set(&m, "awa", &i);
    map_set(&m, "aww", &i);
    printf("%i\n",*(int*)map_get(m, "wowa"));

    map_clear(m, NONE);

    return 0;
}