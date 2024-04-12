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

  //for(int i = 0; i != 8; i++) printf("%lx ", hash[i]);
  //printf("\n");
}

struct blake256_hash {
    uint8_t* buffer;
    size_t bufflen;
    uint32_t total, *hash;
    uint64_t compressed;
};
#define blake224_hash blake256_hash

#define bs 64
struct blake256_hash blake256_init(){
    struct blake256_hash a = {.bufflen = 0, .total = 0, .compressed = 0};
    a.buffer = calloc(sizeof * a.buffer, bs);
    a.hash = calloc(sizeof * a.hash, 8);
    a.hash[0] = 0x6a09e667;
    a.hash[1] = 0xbb67ae85;
    a.hash[2] = 0x3c6ef372;
    a.hash[3] = 0xa54ff53a;
    a.hash[4] = 0x510e527f;
    a.hash[5] = 0x9b05688c;
    a.hash[6] = 0x1f83d9ab;
    a.hash[7] = 0x5be0cd19;
    return a;
}

struct blake256_hash blake256_init_l(lua_State* L){
    struct blake256_hash a = {.bufflen = 0, .total = 0, .compressed = 0};
    a.buffer = lua_newuserdata(L, sizeof * a.buffer * bs);
    a.hash = lua_newuserdata(L, sizeof * a.hash * 8);
    memset(a.buffer, 0, bs);
    a.hash[0] = 0x6a09e667;
    a.hash[1] = 0xbb67ae85;
    a.hash[2] = 0x3c6ef372;
    a.hash[3] = 0xa54ff53a;
    a.hash[4] = 0x510e527f;
    a.hash[5] = 0x9b05688c;
    a.hash[6] = 0x1f83d9ab;
    a.hash[7] = 0x5be0cd19;
    return a;
}

struct blake256_hash blake224_init(){
    struct blake256_hash a = {.bufflen = 0, .total = 0, .compressed = 0};
    a.buffer = calloc(sizeof * a.buffer, bs);
    a.hash = calloc(sizeof * a.hash, 8);
    a.hash[0] = 0xc1059ed8;
    a.hash[1] = 0x367cd507;
    a.hash[2] = 0x3070dd17;
    a.hash[3] = 0xf70e5939;
    a.hash[4] = 0xffc00b31;
    a.hash[5] = 0x68581511;
    a.hash[6] = 0x64f98fa7;
    a.hash[7] = 0xbefa4fa4;
    return a;
}

struct blake256_hash blake224_init_l(lua_State* L){
    struct blake256_hash a = {.bufflen = 0, .total = 0, .compressed = 0};
    a.buffer = lua_newuserdata(L, sizeof * a.buffer * bs);
    a.hash = lua_newuserdata(L, sizeof * a.hash * 8);
    memset(a.buffer, 0, bs);
    a.hash[0] = 0xc1059ed8;
    a.hash[1] = 0x367cd507;
    a.hash[2] = 0x3070dd17;
    a.hash[3] = 0xf70e5939;
    a.hash[4] = 0xffc00b31;
    a.hash[5] = 0x68581511;
    a.hash[6] = 0x64f98fa7;
    a.hash[7] = 0xbefa4fa4;
    return a;
}

void blake256_round(struct blake256_hash* hash){
  compress256(hash->hash, (char*)hash->buffer, hash->compressed * 8);
} 

#define blake224_update blake256_update
void blake256_update(uint8_t* input, size_t len, struct blake256_hash* hash){
  hash->total += len;
  size_t total_add = len + hash->bufflen;
  size_t read = 0;
  if(total_add < bs){
    memcpy(hash->buffer + hash->bufflen, input, len);
    hash->bufflen += len;
    return;
  }

  for(; total_add >= bs;){
    memcpy(hash->buffer + hash->bufflen, input + read, bs - hash->bufflen);
    total_add -= bs;
    hash->bufflen = 0;
    read += bs;
    hash->compressed += 64;
    blake256_round(hash);
  }

  memset(hash->buffer, 0, bs);

  if(0 != total_add){
    memcpy(hash->buffer, input + read, total_add);
    hash->bufflen = total_add;
  }
}

void _blake256_final(struct blake256_hash* hash, char* out_stream){
  hash->compressed += hash->bufflen;

  hash->buffer[hash->bufflen] = 0x80;

  if(hash->bufflen > 55) {
    //too large, needs another buffer
    memset(hash->buffer + hash->bufflen + 1, 0, 64 - hash->bufflen);
    blake256_round(hash);
    hash->compressed = 0;
    memset(hash->buffer, 0, 64);
  }

  size_t lhhh = 8*hash->total;
  U32TO8_BIG(hash->buffer + bs - 8, 0x0);
  U32TO8_BIG(hash->buffer + bs - 4, hash->total << 3);
  /*for(int i = 0; i != bs; i++) printf("%x ", hash->buffer[i]);
    printf("\n");*/
  blake256_round(hash);
}

void blake256_final(struct blake256_hash* hash, char* out_stream){
  uint8_t old[bs];
  struct blake256_hash old_hash;
  memcpy(&old_hash, hash, sizeof * hash);
  memcpy(old, hash->buffer, bs);

  if(hash->bufflen == 55) hash->buffer[hash->bufflen] = 0x81;
  else hash->buffer[bs - 9] = 0x01;

  _blake256_final(hash, out_stream);

  for(int i = 0; i != 8; i++){
    sprintf(out_stream, "%s%08x",out_stream,(hash->hash)[i]);
  }

  memcpy(hash, &old_hash, sizeof * hash);
  memcpy(hash->buffer, old, bs);
}

void blake224_final(struct blake256_hash* hash, char* out_stream){
  uint8_t old[bs];
  struct blake256_hash old_hash;
  memcpy(&old_hash, hash, sizeof * hash);
  memcpy(old, hash->buffer, bs);

  if(hash->bufflen == 55) hash->buffer[hash->bufflen] = 0x80;
  else hash->buffer[bs - 9] = 0x00;

  _blake256_final(hash, out_stream);

  for(int i = 0; i != 7; i++){
    sprintf(out_stream, "%s%08x",out_stream,(hash->hash)[i]);
  }
  
  memcpy(hash, &old_hash, sizeof * hash);
  memcpy(hash->buffer, old, bs);
}

void blake256(char *out, char *in, uint64_t inlen){
  struct blake256_hash a = blake256_init();
  blake256_update((uint8_t*)in, inlen, &a);
  blake256_final(&a, out);
  free(a.buffer);
  free(a.hash);
}

void blake224(char *out, char *in, uint64_t inlen){
  struct blake224_hash a = blake224_init();
  blake224_update((uint8_t*)in, inlen, &a);
  blake224_final(&a, out);
  free(a.buffer);
  free(a.hash);
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

struct blake512_hash {
    uint8_t* buffer;
    size_t bufflen;
    uint64_t total, *hash;
    uint64_t compressed;
};
#define blake384_hash blake512_hash
//#undef bs
#define bs_2 128

struct blake512_hash blake512_init(){
    struct blake512_hash a = {.bufflen = 0, .total = 0, .compressed = 0};
    a.buffer = calloc(sizeof * a.buffer, bs_2);
    a.hash = calloc(sizeof * a.hash, 8);
    a.hash[0] = 0x6a09e667f3bcc908ULL;
    a.hash[1] = 0xbb67ae8584caa73bULL;
    a.hash[2] = 0x3c6ef372fe94f82bULL;
    a.hash[3] = 0xa54ff53a5f1d36f1ULL;
    a.hash[4] = 0x510e527fade682d1ULL;
    a.hash[5] = 0x9b05688c2b3e6c1fULL;
    a.hash[6] = 0x1f83d9abfb41bd6bULL;
    a.hash[7] = 0x5be0cd19137e2179ULL;
    return a;
}

struct blake512_hash blake512_init_l(lua_State* L){
    struct blake512_hash a = {.bufflen = 0, .total = 0, .compressed = 0};
    a.buffer = lua_newuserdata(L, sizeof * a.buffer * bs_2);
    a.hash = lua_newuserdata(L, sizeof * a.hash * 8);
    memset(a.buffer, 0, bs_2);
    a.hash[0] = 0x6a09e667f3bcc908ULL;
    a.hash[1] = 0xbb67ae8584caa73bULL;
    a.hash[2] = 0x3c6ef372fe94f82bULL;
    a.hash[3] = 0xa54ff53a5f1d36f1ULL;
    a.hash[4] = 0x510e527fade682d1ULL;
    a.hash[5] = 0x9b05688c2b3e6c1fULL;
    a.hash[6] = 0x1f83d9abfb41bd6bULL;
    a.hash[7] = 0x5be0cd19137e2179ULL;
    return a;
}

struct blake384_hash blake384_init(){
    struct blake384_hash a = {.bufflen = 0, .total = 0, .compressed = 0};
    a.buffer = calloc(sizeof * a.buffer, bs_2);
    a.hash = calloc(sizeof * a.hash, 8);
    a.hash[0] = 0xcbbb9d5dc1059ed8ULL;
    a.hash[1] = 0x629a292a367cd507ULL;
    a.hash[2] = 0x9159015a3070dd17ULL;
    a.hash[3] = 0x152fecd8f70e5939ULL;
    a.hash[4] = 0x67332667ffc00b31ULL;
    a.hash[5] = 0x8eb44a8768581511ULL;
    a.hash[6] = 0xdb0c2e0d64f98fa7ULL;
    a.hash[7] = 0x47b5481dbefa4fa4ULL;
    return a;
}

struct blake384_hash blake384_init_l(lua_State* L){
    struct blake384_hash a = {.bufflen = 0, .total = 0, .compressed = 0};
    a.buffer = lua_newuserdata(L, sizeof * a.buffer * bs_2);
    a.hash = lua_newuserdata(L, sizeof * a.hash * 8);
    memset(a.buffer, 0, bs_2);
    a.hash[0] = 0xcbbb9d5dc1059ed8ULL;
    a.hash[1] = 0x629a292a367cd507ULL;
    a.hash[2] = 0x9159015a3070dd17ULL;
    a.hash[3] = 0x152fecd8f70e5939ULL;
    a.hash[4] = 0x67332667ffc00b31ULL;
    a.hash[5] = 0x8eb44a8768581511ULL;
    a.hash[6] = 0xdb0c2e0d64f98fa7ULL;
    a.hash[7] = 0x47b5481dbefa4fa4ULL;
    return a;
}

void blake512_round(struct blake512_hash* hash){
  compress512(hash->hash, hash->buffer, hash->compressed * 8);
} 

#define blake384_update blake512_update
void blake512_update(uint8_t* input, size_t len, struct blake512_hash* hash){
  hash->total += len;
  size_t total_add = len + hash->bufflen;
  size_t read = 0;
  if(total_add < bs_2){
    memcpy(hash->buffer + hash->bufflen, input, len);
    hash->bufflen += len;
    return;
  }

  for(; total_add >= bs_2;){
    memcpy(hash->buffer + hash->bufflen, input + read, bs_2 - hash->bufflen);
    total_add -= bs_2;
    hash->bufflen = 0;
    read += bs_2;
    hash->compressed += 128;
    blake512_round(hash);
  }

  memset(hash->buffer, 0, bs_2);

  if(0 != total_add){
    memcpy(hash->buffer, input + read, total_add);
    hash->bufflen = total_add;
  }
}

void _blake512_final(struct blake512_hash* hash, char* out_stream){
  hash->compressed += hash->bufflen;

  hash->buffer[hash->bufflen] = 0x80;

  if(hash->bufflen > bs_2 - 16) {
    //too large, needs another buffer
    memset(hash->buffer + hash->bufflen + 1, 0, 64 - hash->bufflen);
    blake512_round(hash);
    hash->compressed = 0;
    memset(hash->buffer, 0, 64);
  }

  size_t lhhh = 8*hash->total;
  U64TO8_BIG(hash->buffer + bs_2 - 8, hash->total << 3);

  blake512_round(hash);
}

void blake512_final(struct blake512_hash* hash, char* out_stream){
  uint8_t old[bs_2];
  struct blake512_hash old_hash;
  memcpy(&old_hash, hash, sizeof * hash);
  memcpy(old, hash->buffer, bs_2);

  if(hash->bufflen == 111) hash->buffer[hash->bufflen] = 0x81;
  else hash->buffer[bs_2 - 17] = 0x01;

  _blake512_final(hash, out_stream);

  for(int i = 0; i != 8; i++){
        sprintf(out_stream, "%s%016llx",out_stream, (hash->hash)[i]);
    }

  memcpy(hash, &old_hash, sizeof * hash);
  memcpy(hash->buffer, old, bs_2);
}

void blake512(uint8_t* in, size_t len, char* out){
  struct blake512_hash a = blake512_init();
  blake512_update(in, len, &a);
  blake512_final(&a, out);
  free(a.buffer);
  free(a.hash);
}

void blake384_final(struct blake384_hash* hash, char* out_stream){
  uint8_t old[bs_2];
  struct blake384_hash old_hash;
  memcpy(&old_hash, hash, sizeof * hash);
  memcpy(old, hash->buffer, bs_2);

  if(hash->bufflen == 111) hash->buffer[hash->bufflen] = 0x80;
  else hash->buffer[bs_2 - 17] = 0x00;

  _blake512_final(hash, out_stream);

  for(int i = 0; i != 6; i++){
        sprintf(out_stream, "%s%016llx",out_stream, (hash->hash)[i]);
    }

  memcpy(hash, &old_hash, sizeof * hash);
  memcpy(hash->buffer, old, bs_2);
}

void blake384(uint8_t* in, size_t len, char* out){
  struct blake384_hash a = blake384_init();
  blake384_update(in, len, &a);
  blake384_final(&a, out);
  free(a.buffer);
  free(a.hash);
}

int l_blake256_clone(lua_State* L){
  struct blake256_hash* a = (struct blake256_hash*)lua_touserdata(L, -1);
  l_blake256_init(L);
  struct blake256_hash* b = (struct blake256_hash*)lua_touserdata(L, -1);

  memcpy(b->hash, a->hash, 8 * sizeof * b->hash);
  memcpy(b->buffer, a->buffer, bs * sizeof * b->buffer);
  b->total = a->total;
  b->bufflen = a->bufflen;
  b->compressed = a->compressed;
  b->total = a->total;
  return 1;
}

common_hash_init_update(blake256);

int l_blake256_final(lua_State* L){
  struct blake256_hash* a = (struct blake256_hash*)lua_touserdata(L, 1);

  char digest[257] = {0};
  blake256_final(a, digest);

  lua_pushstring(L, digest);
  return 1;
}

int l_blake256(lua_State* L){
    if(lua_gettop(L) == 0) return l_blake256_init(L);
    size_t len = 0;
    uint8_t* a = (uint8_t*)luaL_checklstring(L, 1, &len);
    int argv = lua_gettop(L); 

    char digest[257] = {0};

    blake256(digest, (char*)a, len);

    lua_pushstring(L, digest);

    return 1;
}

int l_blake224_clone(lua_State* L){
  struct blake224_hash* a = (struct blake224_hash*)lua_touserdata(L, -1);
  l_blake224_init(L);
  struct blake224_hash* b = (struct blake224_hash*)lua_touserdata(L, -1);

  memcpy(b->hash, a->hash, 8 * sizeof * b->hash);
  memcpy(b->buffer, a->buffer, bs * sizeof * b->buffer);
  b->total = a->total;
  b->bufflen = a->bufflen;
  b->compressed = a->compressed;
  b->total = a->total;
  return 1;
}

common_hash_init_update(blake224);

int l_blake224_final(lua_State* L){
  struct blake224_hash* a = (struct blake224_hash*)lua_touserdata(L, 1);

  char digest[257] = {0};
  blake224_final(a, digest);

  lua_pushstring(L, digest);
  return 1;
}

int l_blake224(lua_State* L){
    if(lua_gettop(L) == 0) return l_blake224_init(L);
    size_t len = 0;
    char* a = (char*)luaL_checklstring(L, 1, &len);
    int argv = lua_gettop(L); 

    char digest[257] = {0};

    blake224(digest, (char*)a, len);

    lua_pushstring(L, digest);

    return 1;
}

int l_blake512_clone(lua_State* L){
  struct blake512_hash* a = (struct blake512_hash*)lua_touserdata(L, -1);
  l_blake512_init(L);
  struct blake512_hash* b = (struct blake512_hash*)lua_touserdata(L, -1);

  memcpy(b->hash, a->hash, 8 * sizeof * b->hash);
  memcpy(b->buffer, a->buffer, bs_2 * sizeof * b->buffer);
  b->total = a->total;
  b->bufflen = a->bufflen;
  b->compressed = a->compressed;
  b->total = a->total;
  return 1;
}

common_hash_init_update(blake512);

int l_blake512_final(lua_State* L){
  struct blake512_hash* a = (struct blake512_hash*)lua_touserdata(L, 1);

  char digest[513] = {0};
  blake512_final(a, digest);

  lua_pushstring(L, digest);
  return 1;
}

int l_blake512(lua_State* L){
    if(lua_gettop(L) == 0) return l_blake512_init(L);
    size_t len = 0;
    uint8_t* a = (uint8_t*)luaL_checklstring(L, 1, &len);
    int argv = lua_gettop(L); 

    char digest[513] = {0};
    //memset(digest, 0, 513);

    //blake512(digest, a, len, b512);
    blake512(a, len, digest);
    lua_pushstring(L, digest);

    return 1;
}

int l_blake384_clone(lua_State* L){
  struct blake384_hash* a = (struct blake384_hash*)lua_touserdata(L, -1);
  l_blake384_init(L);
  struct blake384_hash* b = (struct blake384_hash*)lua_touserdata(L, -1);

  memcpy(b->hash, a->hash, 8 * sizeof * b->hash);
  memcpy(b->buffer, a->buffer, bs_2 * sizeof * b->buffer);
  b->total = a->total;
  b->bufflen = a->bufflen;
  b->compressed = a->compressed;
  b->total = a->total;
  return 1;
}

common_hash_init_update(blake384);

int l_blake384_final(lua_State* L){
  struct blake384_hash* a = (struct blake384_hash*)lua_touserdata(L, 1);

  char digest[513] = {0};
  blake384_final(a, digest);

  lua_pushstring(L, digest);
  return 1;
}

int l_blake384(lua_State* L){
    if(lua_gettop(L) == 0) return l_blake384_init(L);
    size_t len = 0;
    uint8_t* a = (uint8_t*)luaL_checklstring(L, 1, &len);
    int argv = lua_gettop(L); 

    char digest[513] = {0};

    blake384(a, len, digest);
    lua_pushstring(L, digest);

    return 1;
}
