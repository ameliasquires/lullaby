#include "../util.h"
#include "../crypto.h"

#include <stdio.h>
#include <stdint.h>

uint32_t pjw(uint8_t* in, size_t len){
    uint32_t hash = 0;
    uint32_t high;
    
    for(int i = 0; i != len; i++){
        hash = (hash << 4) + *in++;
        if((high = (hash & 0xf0000000)))
            hash ^= high >> 24;
        hash &= ~high; 
    }

    return hash;
}

int l_pjw(lua_State* L){ 
  size_t len = 0;
  uint8_t* a = (uint8_t*)luaL_checklstring(L, 1, &len);

  char digest[32];

  uint32_t u = pjw(a, len);
  sprintf(digest,"%08x",u);
  lua_pushstring(L, digest);
  return 1;
}
