#include <string.h>
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <math.h>
#include <stdlib.h>
#include "../crypto.h"
#include "../util.h"

const uint8_t blake_sigma[][16] = {
  { 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15 },
  {14, 10, 4, 8, 9, 15, 13, 6, 1, 12, 0, 2, 11, 7, 5, 3 },
  {11, 8, 12, 0, 5, 2, 15, 13, 10, 14, 3, 6, 7, 1, 9, 4 },
  { 7, 9, 3, 1, 13, 12, 11, 14, 2, 6, 5, 10, 4, 0, 15, 8 },
  { 9, 0, 5, 7, 2, 4, 10, 15, 14, 1, 11, 12, 6, 8, 3, 13 },
  { 2, 12, 6, 10, 0, 11, 8, 3, 4, 13, 7, 5, 15, 14, 1, 9 },
  {12, 5, 1, 15, 14, 13, 4, 10, 0, 7, 6, 3, 9, 2, 8, 11 },
  {13, 11, 7, 14, 12, 1, 3, 9, 5, 0, 15, 4, 8, 6, 2, 10 },
  { 6, 15, 14, 9, 11, 3, 0, 8, 12, 2, 13, 7, 1, 4, 10, 5 },
  {10, 2, 8, 4, 7, 6, 1, 5, 15, 11, 9, 14, 3, 12, 13 , 0 },
  { 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15 },
  {14, 10, 4, 8, 9, 15, 13, 6, 1, 12, 0, 2, 11, 7, 5, 3 },
  {11, 8, 12, 0, 5, 2, 15, 13, 10, 14, 3, 6, 7, 1, 9, 4 },
  { 7, 9, 3, 1, 13, 12, 11, 14, 2, 6, 5, 10, 4, 0, 15, 8 },
  { 9, 0, 5, 7, 2, 4, 10, 15, 14, 1, 11, 12, 6, 8, 3, 13 },
  { 2, 12, 6, 10, 0, 11, 8, 3, 4, 13, 7, 5, 15, 14, 1, 9 }
};

const uint32_t blake_u256[16] = {
  0x243f6a88, 0x85a308d3, 0x13198a2e, 0x03707344,
  0xa4093822, 0x299f31d0, 0x082efa98, 0xec4e6c89,
  0x452821e6, 0x38d01377, 0xbe5466cf, 0x34e90c6c,
  0xc0ac29b7, 0xc97c50dd, 0x3f84d5b5, 0xb5470917
};

const uint64_t blake_u512[16] = {
  0x243f6a8885a308d3ULL, 0x13198a2e03707344ULL, 
  0xa4093822299f31d0ULL, 0x082efa98ec4e6c89ULL,
  0x452821e638d01377ULL, 0xbe5466cf34e90c6cULL, 
  0xc0ac29b7c97c50ddULL, 0x3f84d5b5b5470917ULL,
  0x9216d5d98979fb1bULL, 0xd1310ba698dfb5acULL, 
  0x2ffd72dbd01adfb7ULL, 0xb8e1afed6a267e96ULL,
  0xba7c9045f12c7f99ULL, 0x24a19947b3916cf7ULL, 
  0x0801f2e2858efc16ULL, 0x636920d871574e69ULL
};

#define blake_round_256(a,b,c,d,e)          \
  v[a] += (m[blake_sigma[i][e]] ^ blake_u256[blake_sigma[i][e+1]]) + v[b]; \
  v[d] = rotr32( v[d] ^ v[a],16);        \
  v[c] += v[d];           \
  v[b] = rotr32( v[b] ^ v[c],12);        \
  v[a] += (m[blake_sigma[i][e+1]] ^ blake_u256[blake_sigma[i][e]])+v[b]; \
  v[d] = rotr32( v[d] ^ v[a], 8);        \
  v[c] += v[d];           \
  v[b] = rotr32( v[b] ^ v[c], 7);

void compress256(uint32_t* hash, char *block, uint64_t compressed){
  uint32_t v[16], m[16], i;

  for(int i = 0; i < 16; i++)  m[i] = wtf((block + i * 4));

  for(int i = 0; i < 8; i++)  v[i] = hash[i];

  for(int i = 0; i != 8; i++)
    v[i + 8] = blake_u256[i];

  v[12] ^= (uint32_t)compressed;
  v[13] ^= (uint32_t)compressed;
  v[14] ^= compressed >> 32;
  v[15] ^= compressed >> 32;

  for(int i = 0; i < 14; i++){
    blake_round_256( 0,  4,  8, 12,  0 );
    blake_round_256( 1,  5,  9, 13,  2 );
    blake_round_256( 2,  6, 10, 14,  4 );
    blake_round_256( 3,  7, 11, 15,  6 );

    blake_round_256( 0,  5, 10, 15,  8 );
    blake_round_256( 1,  6, 11, 12, 10 );
    blake_round_256( 2,  7,  8, 13, 12 );
    blake_round_256( 3,  4,  9, 14, 14 );
  }

  for(int i = 0; i < 16; i++)  hash[i % 8] ^= v[i];
}

void blake256(char *out, char *in, uint64_t inlen, enum blake256_v v){
    uint32_t hash[8] = {0x6a09e667, 0xbb67ae85, 0x3c6ef372, 0xa54ff53a, 
            0x510e527f, 0x9b05688c, 0x1f83d9ab, 0x5be0cd19};
    if(v == b224){
        hash[0] = 0xc1059ed8;
        hash[1] = 0x367cd507;
        hash[2] = 0x3070dd17;
        hash[3] = 0xf70e5939;
        hash[4] = 0xffc00b31;
        hash[5] = 0x68581511;
        hash[6] = 0x64f98fa7;
        hash[7] = 0xbefa4fa4;
    }

    int aw = inter(inlen, 64);
    uint64_t compressed = 0;
    char* owo = calloc(aw, sizeof * owo);
    memcpy(owo, in, inlen);

    owo[inlen] = 0x80;

    if(inlen == 55){
        owo[inlen] = v==b256?0x81:0x80;
    } else {
        owo[aw - 9] = v==b256?0x01:0x00;
    }

    U32TO8_BIG(owo + aw - 8, 0x0);
    U32TO8_BIG(owo + aw - 4, inlen << 3);

    for(; aw >= 64;){
        compressed += 64;
        if(aw == 64) compressed = inlen;

        compress256(hash, owo, compressed * 8);
        aw -= 64;
        owo += 64;
    }

    for(int i = 0; i != (v==b256?8:7); i++){
        sprintf(out, "%s%08x",out,(hash)[i]);
    }
}

#define blake_round_512(a,b,c,d,e)          \
  v[a] += (m[blake_sigma[i][e]] ^ blake_u512[blake_sigma[i][e+1]]) + v[b];\
  v[d] = rotr64( v[d] ^ v[a],32);        \
  v[c] += v[d];           \
  v[b] = rotr64( v[b] ^ v[c],25);        \
  v[a] += (m[blake_sigma[i][e+1]] ^ blake_u512[blake_sigma[i][e]])+v[b];  \
  v[d] = rotr64( v[d] ^ v[a],16);        \
  v[c] += v[d];           \
  v[b] = rotr64( v[b] ^ v[c],11);

void compress512(uint64_t* hash, uint8_t *block, uint64_t compressed){
  uint64_t v[16], m[16], i;

   for( i = 0; i < 16; ++i )  m[i] = U8TO64_BIG( block + i * 8 );

  for(int i = 0; i < 8; i++)  v[i] = hash[i];

  for(int i = 0; i != 8; i++)
    v[i + 8] = blake_u512[i];

  v[12] ^= compressed;
  v[13] ^= compressed;
  v[14] ^= 0;
  v[15] ^= 0;


  for(i = 0; i < 16; i++){
    blake_round_512(0, 4, 8, 12, 0);
    blake_round_512(1, 5, 9, 13, 2);
    blake_round_512(2, 6, 10, 14, 4);
    blake_round_512(3, 7, 11, 15, 6);

    blake_round_512(0, 5, 10, 15, 8);
    blake_round_512(1, 6, 11, 12, 10);
    blake_round_512(2, 7, 8, 13, 12);
    blake_round_512(3, 4, 9, 14, 14);
    
  }

  for(int i = 0; i < 16; i++)  hash[i % 8] ^= v[i];
}

void blake512(char *out, char *in, uint64_t inlen, enum blake512_v v){
    uint64_t hash[8] = {0x6a09e667f3bcc908ULL, 0xbb67ae8584caa73bULL, 0x3c6ef372fe94f82bULL, 0xa54ff53a5f1d36f1ULL, 
        0x510e527fade682d1ULL, 0x9b05688c2b3e6c1fULL, 0x1f83d9abfb41bd6bULL, 0x5be0cd19137e2179ULL};
    
    if(v == b384){
        hash[0] = 0xcbbb9d5dc1059ed8ULL;
        hash[1] = 0x629a292a367cd507ULL;
        hash[2] = 0x9159015a3070dd17ULL;
        hash[3] = 0x152fecd8f70e5939ULL;
        hash[4] = 0x67332667ffc00b31ULL;
        hash[5] = 0x8eb44a8768581511ULL;
        hash[6] = 0xdb0c2e0d64f98fa7ULL;
        hash[7] = 0x47b5481dbefa4fa4ULL;
    }

    int aw = inter(inlen, 128);
    uint64_t compressed = 0;
    uint8_t* owo = calloc(aw, sizeof * owo);
    memcpy(owo, in, inlen);

    owo[inlen] = 0x80;

    if(inlen == 111){
        owo[inlen] = v==b512?0x81:0x80;
    } else {
        owo[aw - 17] = v==b512?0x01:0x00;
    }

    U64TO8_BIG(owo + aw - 8, inlen << 3);

    for(; aw >= 128;){
        compressed += 128;
        if(aw == 128) compressed = inlen;

        compress512(hash, owo, compressed * 8);
        aw -= 128;
        owo += 128;
    }

    for(int i = 0; i != (v==b512?8:6); i++){
        sprintf(out, "%s%016llx",out, (hash)[i]);
    }
}

int l_blake256(lua_State* L){
    size_t len = 0;
    char* a = (char*)luaL_checklstring(L, 1, &len);
    int argv = lua_gettop(L); 

    char digest[257] = {0};
    memset(digest, 0, 257);

    blake256(digest, a, len, b256);
    lua_pushstring(L, digest);

    return 1;
}

int l_blake224(lua_State* L){
    size_t len = 0;
    char* a = (char*)luaL_checklstring(L, 1, &len);
    int argv = lua_gettop(L); 

    char digest[257] = {0};
    memset(digest, 0, 257);

    blake256(digest, a, len, b224);
    lua_pushstring(L, digest);

    return 1;
}

int l_blake512(lua_State* L){
    size_t len = 0;
    char* a = (char*)luaL_checklstring(L, 1, &len);
    int argv = lua_gettop(L); 

    char digest[513] = {0};
    memset(digest, 0, 513);

    blake512(digest, a, len, b512);
    lua_pushstring(L, digest);

    return 1;
}

int l_blake384(lua_State* L){
    size_t len = 0;
    char* a = (char*)luaL_checklstring(L, 1, &len);
    int argv = lua_gettop(L); 

    char digest[513] = {0};
    memset(digest, 0, 513);

    blake512(digest, a, len, b384);
    lua_pushstring(L, digest);

    return 1;
}