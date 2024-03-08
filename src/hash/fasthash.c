#include "../util.h"
#include "../crypto.h"
#include <stdio.h>
#include <stdint.h>
//almost entirely taken from https://github.com/ztanml/fast-hash/blob/master/fasthash.c

#define mix(h) ({					\
			(h) ^= (h) >> 23;		\
			(h) *= 0x2127599bf4325c37ULL;	\
			(h) ^= (h) >> 47; })

uint64_t fasthash64(uint8_t* in, size_t len, uint64_t seed){
    uint64_t m = 0x880355f21e6d1965ULL;
    uint64_t hash = seed ^ (len * m);
    uint64_t* data = (uint64_t*)in;
    uint64_t v;
    for(;len >= 8; len-=8){
        v=*data++;
        hash^=mix(v);
        hash*=m;

        in+=4;
    }

    uint8_t* data2 = (uint8_t*)data;
    v=0;

    switch (len & 7) {
	case 7: 
        v ^= (uint64_t)data2[6] << 48;
	case 6: 
        v ^= (uint64_t)data2[5] << 40;
	case 5:
        v ^= (uint64_t)data2[4] << 32;
	case 4: 
        v ^= (uint64_t)data2[3] << 24;
	case 3: 
        v ^= (uint64_t)data2[2] << 16;
	case 2: 
        v ^= (uint64_t)data2[1] << 8;
	case 1: 
        v ^= (uint64_t)data2[0];
		hash ^= mix(v);
		hash *= m;
	}

    return mix(hash);
}

uint32_t fasthash32(uint8_t *buf, size_t len, uint32_t seed){
    uint64_t hash = fasthash64(buf, len, seed);
	return hash - (hash >> 32);
}

int l_fasthash64(lua_State* L){ 
  size_t len = 0;
  uint8_t* a = (uint8_t*)luaL_checklstring(L, 1, &len);
  int argv = lua_gettop(L);
  uint64_t seed = 0;
  if(argv > 1) seed = luaL_checkinteger(L, 2);

  char digest[64];

  uint64_t u = fasthash64(a, len, seed);
  sprintf(digest,"%016llx",u);
  lua_pushstring(L, digest);
  return 1;
}

int l_fasthash32(lua_State* L){ 
  size_t len = 0;
  uint8_t* a = (uint8_t*)luaL_checklstring(L, 1, &len);
  int argv = lua_gettop(L);
  uint32_t seed = 0;
  if(argv > 1) seed = luaL_checkinteger(L, 2);

  char digest[32];

  uint32_t u = fasthash32(a, len, seed);
  sprintf(digest,"%04x",u);
  lua_pushstring(L, digest);
  return 1;
}
