#include "../i_util.h"
#include "../crypto.h"
#include <stdio.h>
#include <stdint.h>

uint64_t djb2(uint8_t* in, size_t len){
    uint64_t hash = 5381;

    for(int i = 0; i != len; i++){
        hash = ((hash << 5) + hash) + (uint64_t)*in;
        in++;
    }

    return hash;
}

int l_djb2(lua_State* L){ 
  size_t len = 0;
  uint8_t* a = (uint8_t*)luaL_checklstring(L, 1, &len);
  
  char digest[64];

  uint64_t u = djb2(a, len);
  sprintf(digest,"%08lx",u);
  lua_pushstring(L, digest);
  return 1;
}
