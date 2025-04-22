#include "../crypto.h"
#include <stdio.h>
#include <stdint.h>

struct bsdchecksum_hash bsdchecksum_init(){
  return (struct bsdchecksum_hash){.check = 0x0};
}

int bsdchecksum_free_l(lua_State* L){
  return 0;
}

void bsdchecksum_update(uint8_t* aa, size_t len, struct bsdchecksum_hash* hash){
  for(int i = 0; i != len; i++){
      uint8_t a = aa[i];
      hash->check = (hash->check >> 1) + ((hash->check & 1) << 15);
      hash->check += a;
      hash->check &= 0xffff;
  }
}

uint16_t bsdchecksum_final(struct bsdchecksum_hash* hash){
  return hash->check;
}

uint16_t bsdchecksum(uint8_t* a, size_t len){
  struct bsdchecksum_hash b = bsdchecksum_init();
  bsdchecksum_update(a, len, &b);
  return bsdchecksum_final(&b);
}

common_hash_clone(bsdchecksum);

common_hash_init_update(bsdchecksum);

int l_bsdchecksum_final(lua_State* L){
  struct bsdchecksum_hash* a = (struct bsdchecksum_hash*)lua_touserdata(L, 1);
  uint32_t u = bsdchecksum_final(a);
  char digest[32];
  sprintf(digest,"%i",u);
  lua_pushstring(L, digest);
  return 1;
}

int l_bsdchecksum(lua_State* L){
  if(lua_gettop(L) == 0) return l_bsdchecksum_init(L);
  size_t len = 0;
  uint8_t* a = (uint8_t*)luaL_checklstring(L, 1, &len);
  
  char digest[16];

  //uint16_t u = i_bsdchecksum(a, len);
  uint16_t u = bsdchecksum(a, len);
  sprintf(digest,"%i",u);
  lua_pushstring(L, digest);

  return 1;
}
