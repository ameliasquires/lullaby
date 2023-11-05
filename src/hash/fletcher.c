#include "../crypto.h"
#include <stdio.h>
#include <stdint.h>

uint8_t i_fletcher8(uint8_t *aa, size_t len){
    uint8_t s1 = 0, s2 = 0;

    for(int i = 0; i != len; i++){
        s1 = (s1 + aa[i]) % 15;
        s2 = (s2 + s1) % 15;
    }
    return (s2 << 4) | s1;
}

uint16_t i_fletcher16(uint8_t *aa, size_t len){
    uint16_t s1 = 0, s2 = 0;

    for(int i = 0; i != len; i++){
        s1 = (s1 + aa[i]) % 255;
        s2 = (s2 + s1) % 255;
    }
    return (s2 << 8) | s1;
}

uint32_t i_fletcher32(uint8_t *aa, size_t len){
    uint32_t s1 = 0, s2 = 0;

    for(int i = 0; i != len; i++){
        s1 = (s1 + aa[i]) % 65535;
        s2 = (s2 + s1) % 65535;
    }
    return (s2 << 16) | s1;
}

int l_fletcher32(lua_State* L){
  
  size_t len = 0;
  uint8_t* a = (uint8_t*)luaL_checklstring(L, 1, &len);
  
  char digest[32];

  uint32_t u = i_fletcher32(a, len);
  sprintf(digest,"%08x",u);
  lua_pushstring(L, digest);

  return 1;
}

int l_fletcher16(lua_State* L){
  
  size_t len = 0;
  uint8_t* a = (uint8_t*)luaL_checklstring(L, 1, &len);
  
  char digest[16];

  uint16_t u = i_fletcher16(a, len);
  sprintf(digest,"%04x",u);
  lua_pushstring(L, digest);

  return 1;
}

int l_fletcher8(lua_State* L){
  
  size_t len = 0;
  uint8_t* a = (uint8_t*)luaL_checklstring(L, 1, &len);
  
  char digest[8];

  uint8_t u = i_fletcher8(a, len);
  sprintf(digest,"%02x",u);
  lua_pushstring(L, digest);

  return 1;
}
