#include "../crypto.h"
#include <stdio.h>
#include <stdint.h>

struct sdbm_hash sdbm_init(){
  return (struct sdbm_hash){.hash = 0};
}

void sdbm_update(uint8_t* in, size_t len, struct sdbm_hash* hash){
  for(int i = 0; i != len; i++){
    hash->hash = (uint64_t)*in + (hash->hash << 6) + (hash->hash << 16) - hash->hash;
    in++;
  }
}

uint64_t sdbm_final(struct sdbm_hash* hash){
  return hash->hash;
}

uint64_t sdbm(uint8_t* in, size_t len){
  struct sdbm_hash a = sdbm_init();
  sdbm_update(in, len, &a);
  return sdbm_final(&a);
}

common_hash_init_update(sdbm);

int l_sdbm_final(lua_State* L){
  struct sdbm_hash* a = (struct sdbm_hash*)lua_touserdata(L, 1);
  uint64_t u = sdbm_final(a);
  char digest[64];
  sprintf(digest,"%016lx",u);
  lua_pushstring(L, digest);
  return 1;
}

int l_sdbm(lua_State* L){ 
  if(lua_gettop(L) == 0) return l_sdbm_init(L);
  size_t len = 0;
  uint8_t* a = (uint8_t*)luaL_checklstring(L, 1, &len);

  char digest[64];

  uint64_t u = sdbm(a, len);
  sprintf(digest,"%016lx",u);
  lua_pushstring(L, digest);
  return 1;
}

