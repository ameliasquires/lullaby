#include "../util.h"
#include "../crypto.h"
#include <stdio.h>
#include <stdint.h>


uint64_t fnv_1(uint8_t* in, size_t len, enum fnv_version A){
    uint64_t hash = (A != v_0) * 0xcbf29ce484222325;
    uint64_t prime = 0x100000001b3;

    for(int i = 0; i != len; i++){
        switch(A){
            case v_1:
            case v_0:
                hash *= prime;
                hash ^= in[i];
                break;
            case v_a:
                hash ^= in[i];
                hash *= prime;
                break;
        }
    }

    return hash;
}

int l_fnv_0(lua_State* L){ 
  size_t len = 0;
  uint8_t* a = (uint8_t*)luaL_checklstring(L, 1, &len);

  char digest[64];

  uint64_t u = fnv_1(a, len, v_0);
  sprintf(digest,"%16llx",u);
  lua_pushstring(L, digest);
  return 1;
}

int l_fnv_1(lua_State* L){ 
  size_t len = 0;
  uint8_t* a = (uint8_t*)luaL_checklstring(L, 1, &len);

  char digest[64];

  uint64_t u = fnv_1(a, len, v_1);
  sprintf(digest,"%16llx",u);
  lua_pushstring(L, digest);
  return 1;
}

int l_fnv_a(lua_State* L){ 
  size_t len = 0;
  uint8_t* a = (uint8_t*)luaL_checklstring(L, 1, &len);

  char digest[64];

  uint64_t u = fnv_1(a, len, v_a);
  sprintf(digest,"%16llx",u);
  lua_pushstring(L, digest);
  return 1;
}
