#include <stdio.h>
#include "str.h"
#include <stdlib.h>
#include <string.h>

#include "../lua.h"
#include "parray.h"

/**
 * @brief internal table on how to free values
 *
 * @param {void*} value to be free'd
 * @param {enum free_type} type of free
 */
void free_method(void* v, enum free_type free_meth){
  if(v != NULL){
    if(free_meth == FREE) free(v);
    else if(free_meth == STR) str_free(v);
  }
}

/**
 * @brief defines a parray_t
 *
 * @return {parray_t*} allocated parray_t
 */
parray_t* parray_init(){
  parray_t* awa = malloc(sizeof * awa);
  awa->P = malloc(sizeof * awa->P);
  awa->len = 0;
  return awa;
}

parray_t* parray_initl(int len){
  parray_t* awa = malloc(sizeof * awa);
  awa->P = malloc(sizeof * awa->P * len);
  awa->len = len;
  return awa;
}

/**
 * @brief sets value at key to value, adds a new index if new
 *
 * {parray_t*} the parray to update
 * {char*} key
 * {void*} value
 */
void parray_set(parray_t* p, char* key, void* value){
  for(int i = 0; i != p->len; i++){
    if(strcmp(p->P[i].key->c, key) == 0){
      p->P[i].value = value;
      return;
    }
  }

  p->len++;
  p->P = realloc(p->P, sizeof * p->P * (p->len + 1));
  p->P[p->len - 1].key = str_init(key);
  p->P[p->len - 1].value = value;
}

/**
 * @brief gets item with a key
 *
 * @param {parray_t*} the parray to search
 * @param {char*} key
 * @return value at index, or NULL
 */
void* parray_get(parray_t* p, char* key){
  for(int i = 0; i != p->len; i++){
    if(strcmp(p->P[i].key->c, key) == 0){
      return p->P[i].value;
    }
  }
  return NULL;
}

/**
 * @brief gets index with a key
 *
 * @param {parray_t*} the parray to search
 * @param {char*} key
 * @return index, or -1 if not found
 */
int parray_geti(parray_t* p, char* key){
  for(int i = 0; i != p->len; i++){
    if(strcmp(p->P[i].key->c, key) == 0){
      return i;
    }
  }
  return -1;
}

/**
 * @brief remove element if found
 *
 * @param {parray_t*} the parray to modify
 * @param {char*} key
 * @param {enum free_type} method to free value
 */
void parray_remove(parray_t* p, char* key, enum free_type free){
  int ind = parray_geti(p, key);
  if(ind == -1) return;

  str_free(p->P[ind].key);
  free_method(p->P[ind].value, free);

  for(int i = ind; i < p->len - 1; i++) p->P[i] = p->P[i+1];
  p->len--;
  p->P = realloc(p->P, sizeof * p->P * (p->len + 1));
}

/**
 * @brief frees base of parray_t, leaving the values
 *
 * @param {parray_t*} the parray free
 */
void parray_lclear(parray_t* p){
  free(p->P);
  free(p);
}

/**
 * @brief frees entire parray_t
 *
 * @param {parray_t*} the parray free
 * @param {enum free_type} the method to free values
 */
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

/**
 * @brief find all elements in an array that match, allowing for wildcards
 *
 * @param {parray_t*} the parray to search
 * @param {char*} the string to search for
 * @return {parray_t*} populated array of matches
 */
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
