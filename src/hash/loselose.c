#include "../util.h"
#include "../crypto.h"
#include <stdio.h>
#include <stdint.h>

struct loselose_hash loselose_init(){
  return (struct loselose_hash){.hash = 0};
}

void loselose_update(uint8_t* in, size_t len, struct loselose_hash* hash){
  for(int i = 0; i != len; i++){
    hash->hash += (uint64_t)*in;
    in++;
  }
}

uint64_t loselose_final(struct loselose_hash* hash){
  return hash->hash;
}

uint64_t loselose(uint8_t* in, size_t len){
  struct loselose_hash a = loselose_init();
  loselose_update(in, len, &a);
  return loselose_final(&a);
}

common_hash_init_update(loselose);

int l_loselose_final(lua_State* L){
  lua_pushstring(L, "ud");
  lua_gettable(L, 1);

  struct loselose_hash* a = (struct loselose_hash*)lua_touserdata(L, -1);
  uint64_t u = loselose_final(a);
  char digest[64];
  sprintf(digest,"%08lx",u);
  lua_pushstring(L, digest);
  return 1;
}

int l_loselose(lua_State* L){ 
  if(lua_gettop(L) == 0) return l_loselose_init(L);
  size_t len = 0;
  uint8_t* a = (uint8_t*)luaL_checklstring(L, 1, &len);

  char digest[64];

  uint64_t u = loselose(a, len);
  sprintf(digest,"%08lx",u);
  lua_pushstring(L, digest);
  return 1;
}
