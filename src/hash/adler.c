#include "../crypto.h"
#include <stdio.h>
#include <stdint.h>

struct adler32_hash adler32_init(){
  return (struct adler32_hash){.a = 1, .b = 0};
}

void adler32_update(uint8_t* aa, size_t len, struct adler32_hash* hash){
  for(int i = 0; i != len; i++){
    hash->a += aa[i];
    hash->b += hash->a;
  }
}

uint32_t adler32_final(struct adler32_hash* hash){
  return hash->b * 65536 + hash->a;
}

uint32_t adler32(uint8_t* aa, size_t len){
  struct adler32_hash dig = adler32_init();
  adler32_update(aa, len, &dig);
  return adler32_final(&dig);
}

int l_adler32_init(lua_State* L){
  lua_newtable(L);
  int t = lua_gettop(L);

  struct adler32_hash* a = (struct adler32_hash*)lua_newuserdata(L, sizeof * a);
  int ud = lua_gettop(L);
  *a = adler32_init();

  luaI_tsetv(L, t, "ud", ud);
  luaI_tsetcf(L, t, "update", l_adler32_update);
  luaI_tsetcf(L, t, "final", l_adler32_final);

  lua_pushvalue(L, t);
  return 1;
}

int l_adler32_update(lua_State* L){
  lua_pushstring(L, "ud");
  lua_gettable(L, 1);

  struct adler32_hash* a = (struct adler32_hash*)lua_touserdata(L, -1);
  size_t len = 0;
  uint8_t* b = (uint8_t*)luaL_checklstring(L, 2, &len);

  adler32_update(b, len, a);

  lua_pushvalue(L, 1);
  return 1;
}

int l_adler32_final(lua_State* L){
  lua_pushstring(L, "ud");
  lua_gettable(L, 1);

  struct adler32_hash* a = (struct adler32_hash*)lua_touserdata(L, -1);
  uint32_t u = adler32_final(a);
  char digest[32];
  sprintf(digest,"%08x",u);
  lua_pushstring(L, digest);
  return 1;
}

int l_adler32(lua_State* L){
  size_t len = 0;
  uint8_t* a = (uint8_t*)luaL_checklstring(L, 1, &len);
  
  char digest[32];

  uint32_t u = adler32(a, len);

  sprintf(digest,"%08x",u);
  lua_pushstring(L, digest);

  return 1;
}
