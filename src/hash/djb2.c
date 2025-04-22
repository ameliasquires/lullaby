#include "../crypto.h"
#include <stdio.h>
#include <stdint.h>
#include <inttypes.h>

int djb2_free_l(lua_State* L){
  return 0;
}

struct djb2_hash djb2_init(){
  return (struct djb2_hash){.hash = 5381};
}

void djb2_update(uint8_t * in, size_t len, struct djb2_hash * hash){
  for(int i = 0; i != len; i++){
    hash->hash = ((hash->hash << 5) + hash->hash) + (uint32_t)*in;
    in++;
  }
}

uint32_t djb2_final(struct djb2_hash * hash){
  return hash->hash;
}

uint32_t djb2(uint8_t * in, size_t len){
  struct djb2_hash a = djb2_init();
  djb2_update(in, len, &a);
  return djb2_final(&a);
}

common_hash_clone(djb2);

common_hash_init_update(djb2);

int l_djb2_final(lua_State* L){
  struct djb2_hash* a = (struct djb2_hash*)lua_touserdata(L, 1);
  uint32_t u = djb2_final(a);
  char digest[64];
  sprintf(digest,"%08"PRIx32,u);
  lua_pushstring(L, digest);
  return 1;
}

int l_djb2(lua_State* L){ 
  if(lua_gettop(L) == 0) return l_djb2_init(L);
  size_t len = 0;
  uint8_t* a = (uint8_t*)luaL_checklstring(L, 1, &len);
  
  char digest[64];

  uint32_t u = djb2(a, len);
  sprintf(digest,"%08"PRIx32,u);
  lua_pushstring(L, digest);
  return 1;
}
