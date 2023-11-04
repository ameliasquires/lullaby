#include "../crypto.h"
#include <stdio.h>
#include <stdint.h>

uint8_t i_xor8(uint8_t *aa, size_t len){
    uint8_t a = 0;

    for(int i = 0; i != len; i++)
        a += aa[i] & 0xff;
    
    return ((a ^ 0xff) + 1) & 0xff;
}

int l_xor8(lua_State* L){
  
  size_t len = 0;
  uint8_t* a = (uint8_t*)luaL_checklstring(L, 1, &len);
  
  char digest[8];

  uint8_t u = i_xor8(a, len);
  sprintf(digest,"%x",u);
  lua_pushstring(L, digest);

  return 1;
}
