#include "map.h"
#include "../hash/fnv.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#define mod_inc 4

uint64_t hash(char* c, size_t len){
    return fnv_1((uint8_t*)c, len, v_a);
}

map_t* map_init(){
    map_t* awa = malloc(sizeof * awa);
    awa->M = malloc(sizeof * awa->M * 4);
    awa->len = 0;
    awa->mod = 4;
    return awa;
}

void map_regraph(map_t* M){
    map_t* remade = map_init();
    for(int i = 0; i != M->len; i++){
        //what happens if the map_set calls map_regraph??? idk
        map_set(remade, M->M[i].key->c, M->M[i].value);
    }
    M = remade;
}

void map_set(map_t* M, char* key, void* value){
    uint64_t h = hash(key, strlen(key));
    if(M->len >= M->mod){
        expand:
        M->mod *= 4;
        M->M = realloc(M->M, sizeof * M->M * M->mod);
        //regraph it
        map_regraph(M);
    }
    uint64_t ind = h % M->mod;
    
    for(int count = 0; M->M[ind].key != NULL && M->M[ind].hash != h && strcmp(M->M[ind].key->c, key) != 0; count++){
        ind++;
        if(ind >= M->mod) ind = 0;
        if(count > 5) goto expand;
    }

    M->M[ind].hash = h;
    M->M[ind].key = str_init(key);
    M->M[ind].value = value;
}

int map_geti(map_t* M, char* key){
    uint64_t h = hash(key, strlen(key));
    uint64_t ind = h % M->mod;
    //melem_t sel = M->M[];

    for(uint64_t initial = ind; ind != initial - 1;){
        if(M->M[ind].key == NULL) return -1;
        if(M->M[ind].hash == h && strcmp(M->M[ind].key->c, key)==0) return ind;
        ind++;
        if(ind >= M->mod) ind = 0;
    }
    return -1;
}

void* map_get(map_t* M, char* key){
    int r = map_geti(M, key);
    printf("%i\n",r);
    return r == -1? NULL : M->M[r].value;
}

void map_remove(map_t* p, char* key, enum free_type free);
void map_clear(map_t*, enum free_type);
void map_lclear(map_t*);

void map_dump(map_t* M){
    for(int i = 0; i != M->mod; i++){
        if(M->M[i].key != NULL){
            printf("%i | %s : %p\n",i,M->M[i].key->c, M->M[i].value);
        } 
    }
}
int main(){
    int i = 5;
    int b = 24;
    int c = 9;
    map_t* m = map_init();
    for(int i = 65; i != 91; i++){
        //printf("%c\n",i);
        int* ww = malloc(sizeof * ww * 55);
        ww[0] = i;
        map_set(m, ((char[]){i, 0}), ww);
    }
    map_dump(m);
    
    printf("%i\n",*(int*)map_get(m, "B"));

    return 0;
}