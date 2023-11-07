#include "../crypto.h"
#include <stdio.h>
#include <stdint.h>

uint32_t i_adler32(uint8_t *aa, size_t len){
    uint16_t a = 1, b = 0;

    for(int i = 0; i != len; i++){
        a += aa[i];
        b += a;
    }

    return b * 65536 + a;
}

int l_adler32(lua_State* L){
  size_t len = 0;
  uint8_t* a = (uint8_t*)luaL_checklstring(L, 1, &len);
  
  char digest[32];

  uint32_t u = i_adler32(a, len);
  sprintf(digest,"%08x",u);
  lua_pushstring(L, digest);

  return 1;
}
