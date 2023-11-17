#include "../i_util.h"
#include "../crypto.h"
#include <stdio.h>
#include <stdint.h>

uint32_t jenkins_oaat(uint8_t* in, size_t len){
    uint32_t hash = 0;

    for(int i = 0; i != len;){
        hash += in[i++];
        hash += hash << 10;
        hash ^= hash >> 6;
    }
    hash += hash << 3;
    hash ^= hash >> 11;
    hash += hash << 15;

    return hash;
}

int l_oaat(lua_State* L){ 
  size_t len = 0;
  uint8_t* a = (uint8_t*)luaL_checklstring(L, 1, &len);

  char digest[64];

  uint32_t u = jenkins_oaat(a, len);
  sprintf(digest,"%04x",u);
  lua_pushstring(L, digest);
  return 1;
}
