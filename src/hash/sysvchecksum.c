#include "../crypto.h"
#include <stdio.h>
#include <stdint.h>
#include <math.h>

struct sysvchecksum_hash sysvchecksum_init(){
  return (struct sysvchecksum_hash){.check = 0};
}

void sysvchecksum_update(uint8_t* aa, size_t len, struct sysvchecksum_hash* hash){
  for(int i = 0; i != len; i++)
    hash->check += aa[i];
}

uint32_t sysvchecksum_final(struct sysvchecksum_hash* hash){
  uint32_t r = hash->check % (int)pow(2,16) + (hash->check % (int)pow(2,32)) / (int)pow(2,16);
  return (r % (int)pow(2,16)) + r / (int)pow(2,16);
}

uint32_t sysvchecksum(uint8_t* aa, size_t len){
  struct sysvchecksum_hash a = sysvchecksum_init();
  sysvchecksum_update(aa, len, &a);
  return sysvchecksum_final(&a);
}

common_hash_clone(sysvchecksum);
common_hash_init_update(sysvchecksum);

int l_sysvchecksum_final(lua_State* L){
  struct sysvchecksum_hash* a = (struct sysvchecksum_hash*)lua_touserdata(L, 1);
  uint32_t u = sysvchecksum_final(a);
  char digest[32];
  sprintf(digest,"%x",u);
  lua_pushstring(L, digest);
  return 1;
}
 
int l_sysvchecksum(lua_State* L){
  if(lua_gettop(L) == 0) return l_sysvchecksum_init(L);
  size_t len = 0;
  uint8_t* a = (uint8_t*)luaL_checklstring(L, 1, &len);
  
  char digest[32];

  uint32_t u = sysvchecksum(a, len);
  sprintf(digest,"%x",u);
  lua_pushstring(L, digest);

  return 1;
}
