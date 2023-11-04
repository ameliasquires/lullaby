#include "../crypto.h"
#include <stdio.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <inttypes.h>

uint32_t i_crc32(uint8_t *data, size_t len){
    uint32_t crc = 0xFFFFFFFF;

    for(int i = 0; i < len; i++){
        uint32_t extract = data[i];
        for(int z = 0; z < 8; z++){
            uint32_t b = (extract^crc)&1;
            crc>>=1;
            if(b) crc^=0xEDB88320;
            extract>>=1;
        }
    }
    return -(crc+1);
}

uint16_t i_crc16(uint8_t *aa, size_t len){
    uint16_t crc = 0x0;
    
    for(int i = 0; i != len; i++){
        uint8_t a = aa[i];
        crc ^= a;
        for (int z = 0; z < 8; z++){
            if (crc & 1) crc = (crc >> 1) ^ 0xA001;
            else crc = (crc >> 1);
        }
    }
    return crc;
}

uint8_t i_crc8(uint8_t *aa, size_t len){
    //crc8 maxim
    uint8_t crc = 0x00;
    
    for(int i = 0; i != len; i++){
        uint8_t a = aa[i];

        for (int z = 0; z < 8; z++){
            uint8_t b = (crc ^ a) & 1;
            crc >>= 1;
            if(b) crc ^= 0x8c;
            a >>=1;
        }
    }
    return crc;
}

int l_crc8(lua_State* L){
  
  size_t len = 0;
  uint8_t* a = (uint8_t*)luaL_checklstring(L, 1, &len);
  
  char digest[8];

  uint8_t u = i_crc8(a, len);
  sprintf(digest,"%x",u);
  lua_pushstring(L, digest);

  return 1;
}

int l_crc16(lua_State* L){
  
  size_t len = 0;
  uint8_t* a = (uint8_t*)luaL_checklstring(L, 1, &len);
  
  char digest[16];

  uint16_t u = i_crc16(a, len);
  sprintf(digest,"%x",u);
  lua_pushstring(L, digest);

  return 1;
}

int l_crc32(lua_State* L){
  
  size_t len = 0;
  uint8_t* a = (uint8_t*)luaL_checklstring(L, 1, &len);
  
  char digest[32];

  uint32_t u = i_crc32(a, len);
  sprintf(digest,"%x",u);
  lua_pushstring(L, digest);

  return 1;
}
