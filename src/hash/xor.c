#include "../crypto.h"
#include <stdio.h>
#include <stdint.h>

struct xor8_hash xor8_init(){
  return (struct xor8_hash){.a = 0};
}

void xor8_update(uint8_t* aa, size_t len, struct xor8_hash* hash){
  for(int i = 0; i != len; i++)
    hash->a += aa[i] & 0xff;
}

uint8_t xor8_final(struct xor8_hash* hash){
  return ((hash->a ^ 0xff) + 1) & 0xff;
}

uint8_t xor8(uint8_t* aa, size_t len){
  struct xor8_hash a = xor8_init();
  xor8_update(aa, len, &a);
  return xor8_final(&a);
}

common_hash_clone(xor8)
common_hash_init_update(xor8);

int l_xor8_final(lua_State* L){
  struct xor8_hash* a = (struct xor8_hash*)lua_touserdata(L, 1);
  uint8_t u = xor8_final(a);
  char digest[8];
  sprintf(digest,"%02x",u);
  lua_pushstring(L, digest);
  return 1;
}

int l_xor8(lua_State* L){
  if(lua_gettop(L) == 0) return l_xor8_init(L);
  size_t len = 0;
  uint8_t* a = (uint8_t*)luaL_checklstring(L, 1, &len);
  
  char digest[8];

  uint8_t u = xor8(a, len);
  sprintf(digest,"%02x",u);
  lua_pushstring(L, digest);

  return 1;
}
