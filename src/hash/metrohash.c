#include "../crypto.h"
#include <stdio.h>
#include <stdint.h>

#define u64(a) (*(uint64_t*)a)
#define u32(a) (*(uint32_t*)a)
#define u16(a) (*(uint16_t*)a)
#define u8(a) (*(uint8_t*)a)

uint64_t metrohash64(uint8_t* in, size_t len, uint32_t seed, enum metrohash_version v){
    uint64_t k0, k1, k2, k3, inner_r, inner_r2;
    if(v == v1){
        k0 = 0xC83A91E1;
        k1 = 0x8648DBDB;
        k2 = 0x7BDEC03B;
        k3 = 0x2F5870A5;
        inner_r = 33;
        inner_r2 = 33;
    } else {
        k0 = 0xD6D018F5;
        k1 = 0xA2AA033B;   
        k2 = 0x62992FC1;
        k3 = 0x30BC5B29; 
        inner_r = 30;
        inner_r2 = 29;
    }

    uint8_t* end = in + len;
    uint64_t hash = ((((uint64_t)seed) + k2) * k0) + len;

    if(len >= 32){
        uint64_t v[4];
        v[0] = hash;
        v[1] = hash;
        v[2] = hash;
        v[3] = hash;

        for(; in <= (end - 32);){
            v[0] += u64(in) * k0; in += 8; v[0] = rot64(v[0],29) + v[2];
            v[1] += u64(in) * k1; in += 8; v[1] = rot64(v[1],29) + v[3];
            v[2] += u64(in) * k2; in += 8; v[2] = rot64(v[2],29) + v[0];
            v[3] += u64(in) * k3; in += 8; v[3] = rot64(v[3],29) + v[1];
        }

        v[2] ^= rot64(((v[0] + v[3]) * k0) + v[1], inner_r) * k1;
        v[3] ^= rot64(((v[1] + v[2]) * k1) + v[0], inner_r) * k0;
        v[0] ^= rot64(((v[0] + v[2]) * k0) + v[3], inner_r) * k1;
        v[1] ^= rot64(((v[1] + v[3]) * k1) + v[2], inner_r) * k0;
        hash += v[0] ^ v[1];
    }
    
    if ((end - in) >= 16){
        uint64_t v0 = hash + (u64(in) * (v == v1? k0 : k2)); in += 8; v0 = rot64(v0,inner_r2) * (v == v1? k1 : k3);
        uint64_t v1 = hash + (u64(in) * (v == v1? k1 : k2)); in += 8; v1 = rot64(v1,inner_r2) * (v == v1? k2 : k3);
        v0 ^= rot64(v0 * k0, (v == v1? 35 : 34)) + v1;
        v1 ^= rot64(v1 * k3, (v == v1? 35 : 34)) + v0;
        hash += v1;
    }

    if ((end - in) >= 8){
        hash += u64(in) * k3; in += 8;
        hash ^= rot64(hash, (v == v1? 33 : 36)) * k1;
        
    }
    
    if ((end - in) >= 4){
        hash += u32(in) * k3; in += 4;
        hash ^= rot64(hash, 15) * k1;
    }
    
    if ((end - in) >= 2){
        hash += u16(in) * k3; in += 2;
        hash ^= rot64(hash, (v == v1? 13 : 15)) * k1;
    }
    
    if ((end - in) >= 1){
        hash += u8(in) * k3;
        hash ^= rot64(hash, (v == v1? 25 : 23)) * k1;
    }
    
    hash ^= rot64(hash, (v == v1? 33 : 28));
    hash *= k0;
    hash ^= rot64(hash, (v == v1? 33 : 29));
    
    return hash;
}

void metrohash128(uint8_t* in, size_t len, uint32_t seed, uint64_t *a, uint64_t *b, enum metrohash_version ver){
    uint64_t k0 = 0xC83A91E1;
    uint64_t k1 = 0x8648DBDB;
    uint64_t k2 = 0x7BDEC03B;
    uint64_t k3 = 0x2F5870A5;

    if(ver == v2){
        k0 = 0xD6D018F5;
        k1 = 0xA2AA033B;
        k2 = 0x62992FC1;
        k3 = 0x30BC5B29; 
    }

    uint8_t * end = in + len;

    uint64_t v[4];
    
    v[0] = ((((uint64_t)seed) - k0) * k3) + len;
    v[1] = ((((uint64_t)seed) + k1) * k2) + len;

    if(len >= 32){
        v[2] = ((((uint64_t)seed) + k0) * k2) + len;
        v[3] = ((((uint64_t)seed) - k1) * k3) + len;

        for(;in <= end - 32;){
            v[0] += u64(in) * k0; in += 8; v[0] = rot64(v[0],29) + v[2];
            v[1] += u64(in) * k1; in += 8; v[1] = rot64(v[1],29) + v[3];
            v[2] += u64(in) * k2; in += 8; v[2] = rot64(v[2],29) + v[0];
            v[3] += u64(in) * k3; in += 8; v[3] = rot64(v[3],29) + v[1];
        }

        v[2] ^= rot64(((v[0] + v[3]) * k0) + v[1], ver == v1 ? 26 : 33) * k1;
        v[3] ^= rot64(((v[1] + v[2]) * k1) + v[0], ver == v1 ? 26 : 33) * k0;
        v[0] ^= rot64(((v[0] + v[2]) * k0) + v[3], ver == v1 ? 26 : 33) * k1;
        v[1] ^= rot64(((v[1] + v[3]) * k1) + v[2], ver == v1 ? 26 : 33) * k0;
    }

    if ((end - in) >= 16){
        v[0] += u64(in) * k2; in += 8; v[0] = rot64(v[0],ver == v1 ? 33 : 29) * k3;
        v[1] += u64(in) * k2; in += 8; v[1] = rot64(v[1],ver == v1 ? 33 : 29) * k3;
        v[0] ^= rot64((v[0] * k2) + v[1], ver == v1 ? 17 : 29) * k1;
        v[1] ^= rot64((v[1] * k3) + v[0], ver == v1 ? 17 : 29) * k0;
    }
    
    if ((end - in) >= 8){
        v[0] += u64(in) * k2; in += 8; v[0] = rot64(v[0],ver == v1 ? 33 : 29) * k3;
        v[0] ^= rot64((v[0] * k2) + v[1], ver == v1 ? 20 : 29) * k1;
    }
    
    if ((end - in) >= 4){
        v[1] += u32(in) * k2; in += 4; v[1] = rot64(v[1],ver == v1 ? 33 : 29) * k3;
        v[1] ^= rot64((v[1] * k3) + v[0], ver == v1 ? 18 : 25) * k0;
    }
    
    if ((end - in) >= 2){
        v[0] += u16(in) * k2; in += 2; v[0] = rot64(v[0],ver == v1 ? 33 : 29) * k3;
        v[0] ^= rot64((v[0] * k2) + v[1], ver == v1 ? 24 : 30) * k1;
    }
    
    if ((end - in) >= 1){
        v[1] += u8(in) * k2; v[1] = rot64(v[1],ver == v1 ? 33 : 29) * k3;
        v[1] ^= rot64((v[1] * k3) + v[0], ver == v1 ? 24 : 18) * k0;
    }
    
    v[0] += rot64((v[0] * k0) + v[1], ver == v1 ? 13 : 33);
    v[1] += rot64((v[1] * k1) + v[0], ver == v1 ? 37 : 33);
    v[0] += rot64((v[0] * k2) + v[1], ver == v1 ? 13 : 33);
    v[1] += rot64((v[1] * k3) + v[0], ver == v1 ? 37 : 33);

    //printf("%llx %llx",v[0],v[1]);
    *a = v[0];
    *b = v[1];
}

int l_metrohash64_v1(lua_State* L){ 
  size_t len = 0;
  uint8_t* a = (uint8_t*)luaL_checklstring(L, 1, &len);
  int argv = lua_gettop(L); 
  uint64_t seed = 0;
  if(argv > 1) seed = luaL_checkinteger(L, 2);

  char digest[64];

  uint64_t u = metrohash64(a, len, seed, v1);
  sprintf(digest,"%016llx",u);
  lua_pushstring(L, digest);
  return 1;
}

int l_metrohash64_v2(lua_State* L){ 
  size_t len = 0;
  uint8_t* a = (uint8_t*)luaL_checklstring(L, 1, &len);
  int argv = lua_gettop(L);
  uint64_t seed = 0;
  if(argv > 1) seed = luaL_checkinteger(L, 2);

  char digest[64];

  uint64_t u = metrohash64(a, len, seed, v2);
  sprintf(digest,"%016llx",u);
  lua_pushstring(L, digest);
  return 1;
}

int l_metrohash128_v1(lua_State* L){ 
  size_t len = 0;
  uint8_t* a = (uint8_t*)luaL_checklstring(L, 1, &len);
  int argv = lua_gettop(L);
  uint64_t seed = 0;
  if(argv > 1) seed = luaL_checkinteger(L, 2);

  char digest[64];

  uint64_t u1, u2;
  metrohash128(a, len, seed, &u1, &u2, v1);
  sprintf(digest,"%016llx%016llx",u1,u2);
  lua_pushstring(L, digest);
  return 1;
}

int l_metrohash128_v2(lua_State* L){ 
  size_t len = 0;
  uint8_t* a = (uint8_t*)luaL_checklstring(L, 1, &len);
  int argv = lua_gettop(L);
  uint64_t seed = 0;
  if(argv > 1) seed = luaL_checkinteger(L, 2);

  char digest[64];

  uint64_t u1, u2;
  metrohash128(a, len, seed, &u1, &u2, v2);
  sprintf(digest,"%016llx%016llx",u1,u2);
  lua_pushstring(L, digest);
  return 1;
}
