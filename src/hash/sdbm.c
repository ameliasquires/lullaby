#include "../util.h"
#include "../crypto.h"
#include <stdio.h>
#include <stdint.h>

uint64_t sdbm(uint8_t* in, size_t len){
    uint64_t hash = 0;

    for(int i = 0; i != len; i++){
        hash = (uint64_t)*in + (hash << 6) + (hash << 16) - hash;
        in++;
    }

    return hash;
}

int l_sdbm(lua_State* L){ 
  size_t len = 0;
  uint8_t* a = (uint8_t*)luaL_checklstring(L, 1, &len);

  char digest[64];

  uint64_t u = sdbm(a, len);
  sprintf(digest,"%016lx",u);
  lua_pushstring(L, digest);
  return 1;
}

