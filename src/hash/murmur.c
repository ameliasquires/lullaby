#include "../crypto.h"
#include <stdio.h>
#include <stdint.h>

uint32_t murmur1_32(uint8_t* in, size_t len, uint32_t seed){
    uint32_t m = 0xc6a4a793;
    uint32_t hash = seed ^ (len * m);

    for(;len >= 4; len-=4){
        hash+=*(uint32_t*)in;
        hash*=m;
        hash^=hash >> 16;

        in+=4;
    }

    switch(len){
        case 3:
            hash+=in[2]<<16;
        case 2:
            hash+=in[1]<<8;
        case 1:
            hash+=in[0];
            hash*=m;
            hash^=hash>>16;
            break;
    }

    hash*=m;
    hash^=hash>>10;
    hash*=m;
    hash^=hash>>17;

    return hash;
}

uint32_t murmur2_32(uint8_t* in, size_t len, uint32_t seed){
    uint32_t m = 0x5bd1e995;
    uint32_t hash = seed ^ len;

    for(;len >= 4; len-=4){
        uint32_t k = *(uint32_t*)in;

        k*=m;
        k^=k>>24;
        k*=m;

        hash*=m;
        hash^=k;

        in+=4;
    }

    switch(len){
        case 3:
            hash+=in[2]<<16;
        case 2:
            hash+=in[1]<<8;
        case 1:
            hash+=in[0];
            hash*=m;
            break;
    }

    hash^=hash>>13;
    hash*=m;
    hash^=hash>>15;

    return hash;
}

int l_murmur1_32(lua_State* L){ 
  size_t len = 0;
  uint8_t* a = (uint8_t*)luaL_checklstring(L, 1, &len);
  int argv = lua_gettop(L);
  uint64_t seed = 0;
  if(argv > 1) seed = luaL_checkinteger(L, 2);

  char digest[64];

  uint64_t u = murmur1_32(a, len, seed);
  sprintf(digest,"%08lx",u);
  lua_pushstring(L, digest);
  return 1;
}

int l_murmur2_32(lua_State* L){ 
  size_t len = 0;
  uint8_t* a = (uint8_t*)luaL_checklstring(L, 1, &len);
  int argv = lua_gettop(L);
  uint64_t seed = 0;
  if(argv > 1) seed = luaL_checkinteger(L, 2);

  char digest[64];

  uint64_t u = murmur2_32(a, len, seed);
  sprintf(digest,"%08lx",u);
  lua_pushstring(L, digest);
  return 1;
}
