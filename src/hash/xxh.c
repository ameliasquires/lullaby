#include "../crypto.h"
#include <stdio.h>
#include <stdint.h>

#define max_buffer_size32 16
#define max_buffer_size64 32
static inline uint32_t rol32(uint32_t x, unsigned char bits){
    return (x << bits) | (x >> (32 - bits));
}
static inline uint64_t rol64(uint64_t x, unsigned char bits){
    return (x << bits) | (x >> (64 - bits));
}

uint32_t i_xxhash32(uint8_t *data, uint32_t seed, size_t len){
    const uint32_t prime1 = 2654435761U;
    const uint32_t prime2 = 2246822519U;
    const uint32_t prime3 = 3266489917U;
    const uint32_t prime4 = 668265263U;
    const uint32_t prime5 = 374761393U;

    uint32_t state[4];
    uint32_t result = len;

    uint8_t* stop      = data + len;
    uint8_t* stop_block = stop - max_buffer_size32;

    state[0] = seed + prime1 + prime2;
    state[1] = seed + prime2;
    state[2] = seed;
    state[3] = seed - prime1;

    for(;data <= stop_block;){
        state[0] = rol32(state[0] + data[0] * prime2, 13) * prime1;
        state[1] = rol32(state[1] + data[1] * prime2, 13) * prime1;
        state[2] = rol32(state[2] + data[2] * prime2, 13) * prime1;
        state[3] = rol32(state[3] + data[3] * prime2, 13) * prime1;

        data += 16;
    }

    if(len >= max_buffer_size32){
        result += rol32(state[0],  1) + rol32(state[1],  7) +
                rol32(state[2], 12) + rol32(state[3], 18);
    } else result += state[2] + prime5;

    for (; data + 4 <= stop; data += 4)
      result = rol32(result + *(uint32_t*)data * prime3, 17) * prime4;

    for(;data != stop;)
      result = rol32(result + (*data++) * prime5, 11) * prime1;

    result ^= result >> 15;
    result *= prime2;
    result ^= result >> 13;
    result *= prime3;
    result ^= result >> 16;

    return result;
}

#define pr64(u1,u2) rol64((u1) + (u2) * prime2, 31) * prime1
uint64_t i_xxhash64(uint8_t *data, uint64_t seed, uint64_t len){
    const uint64_t prime1 = 11400714785074694791ULL;
    const uint64_t prime2 = 14029467366897019727ULL;
    const uint64_t prime3 = 1609587929392839161ULL;
    const uint64_t prime4 = 9650029242287828579ULL;
    const uint64_t prime5 = 2870177450012600261ULL;

    uint64_t state[4];
    uint64_t result;
    
    uint8_t* stop      = data + len;
    uint8_t* stop_block = stop - max_buffer_size64;

    state[0] = seed + prime1 + prime2;
    state[1] = seed + prime2;
    state[2] = seed;
    state[3] = seed - prime1;

    for(;data <= stop_block;){
        state[0] = pr64(state[0], data[0]);
        state[1] = pr64(state[1], data[1]);
        state[2] = pr64(state[2], data[2]);
        state[3] = pr64(state[3], data[3]);

        data += 32;
    }
    
    if(len >= max_buffer_size64){
        result = rol64(state[0],  1) + rol64(state[1],  7) +
                rol64(state[2], 12) + rol64(state[3], 18);
        result = (result ^ pr64(0, state[0])) * prime1 + prime4;
        result = (result ^ pr64(0, state[1])) * prime1 + prime4;
        result = (result ^ pr64(0, state[2])) * prime1 + prime4;
        result = (result ^ pr64(0, state[3])) * prime1 + prime4;
    } else { 
        result = state[2] + prime5;
    }
    result += len;

    for (; data + 8 <= stop; data += 8)
      result = rol64(result ^ pr64(0, *(uint64_t*)data), 27) * prime1 + prime4;

    if (data + 4 <= stop){
      result = rol64(result ^ (*(uint32_t*)data) * prime1,   23) * prime2 + prime3;
      data  += 4;
    }

    for(;data != stop;)
      result = rol64(result ^ (*data++) * prime5, 11) * prime1;

    result ^= result >> 33;
    result *= prime2;
    result ^= result >> 29;
    result *= prime3;
    result ^= result >> 32;
    return result;
}

int l_xxh64(lua_State* L){
  
  size_t len = 0;
  uint8_t* a = (uint8_t*)luaL_checklstring(L, 1, &len);
  int argv = lua_gettop(L);

  uint64_t seed = 0;
  if(argv>1) seed = luaL_checkinteger(L, 2);
  
  char digest[64];

  uint64_t u = i_xxhash64(a, seed, len);
  sprintf(digest,"%016llx",u);
  lua_pushstring(L, digest);

  return 1;
}

int l_xxh32(lua_State* L){
  
  size_t len = 0;
  uint8_t* a = (uint8_t*)luaL_checklstring(L, 1, &len);
  int argv = lua_gettop(L);

  uint32_t seed = 0;
  if(argv>1) seed = luaL_checkinteger(L, 2);
  
  char digest[32];

  uint32_t u = i_xxhash32(a, seed, len);
  sprintf(digest,"%x",u);
  lua_pushstring(L, digest);

  return 1;
}
