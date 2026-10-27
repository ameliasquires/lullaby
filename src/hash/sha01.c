#include "../crypto.h"
#include "lua.h"
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

struct sha01_hash* sha01_init(uint8_t ver){
  struct sha01_hash* a = calloc(1, sizeof * a);
  *a = (struct sha01_hash){.h0 = 0x67452301, .h1 = 0xEFCDAB89, .h2 = 0x98BADCFE, .h3 = 0x10325476, .h4 = 0xC3D2E1F0,
    .total = 0, .bufflen = 0, .version = ver};
  a->buffer = calloc(sizeof * a->buffer, sha01_bs);
  return a;
}

struct sha01_hash* sha01_clone(struct sha01_hash* hash){
  struct sha01_hash* clone = calloc(1, sizeof * clone);
  memcpy(clone, hash, sizeof * clone);
  clone->buffer = calloc(sizeof * clone->buffer, sha01_bs);
  memcpy(clone->buffer, hash->buffer, sha01_bs);
  return clone;
}

int sha01_free(struct sha01_hash* hash){
  free(hash->buffer);
  free(hash);
  return 0;
}

void sha01_round(struct sha01_hash* hash){
  int hat = 0;
  uint32_t W[80] = {0};

  for(int i = 0; i != 16; i++){
    int t = 24;
    for(;t>=0;){
      W[i] += (((uint32_t)hash->buffer[hat]) << t);
      hat++;
      t-=8;
    }
  }
  for(int i = 16; i != 80; i++)
    W[i] = rotl32(W[i - 3] ^ W[i - 8] ^ W[i - 14] ^ W[i - 16], hash->version);

  uint32_t a = hash->h0;
  uint32_t b = hash->h1;
  uint32_t c = hash->h2;
  uint32_t d = hash->h3;
  uint32_t e = hash->h4;

  for(int i = 0; i != 80; i++){

    uint32_t f,k;
    if(0 <= i && i <= 19){
      f = (b & c) | ((~b) & d);
      k = 0x5A827999;
    } else if(20 <= i && i <= 39){
      f = b ^ c ^ d;
      k = 0x6ED9EBA1;
    } else if(40 <= i && i <= 59){
      f = (b & c) | (b & d) | (c & d);
      k = 0x8F1BBCDC;
    } else {
      f = b ^ c ^ d;
      k = 0xCA62C1D6;
    }

    uint32_t temp = rotl32(a, 5) + f + e + k + W[i];
    e = d;
    d = c;
    c = rotl32(b, 30);
    b = a;
    a = temp;
  }

  hash->h0 += a;
  hash->h1 += b;
  hash->h2 += c;
  hash->h3 += d;
  hash->h4 += e;
}

void sha01_update(uint8_t* input, size_t len, struct sha01_hash* hash){
  hash->total += len;
  size_t total_add = len + hash->bufflen;
  size_t read = 0;
  if(total_add < sha01_bs){
    memcpy(hash->buffer + hash->bufflen, input, len);
    hash->bufflen += len;
    return;
  }

  for(; total_add >= sha01_bs;){
    memcpy(hash->buffer + hash->bufflen, input + read, sha01_bs - hash->bufflen);
    total_add -= sha01_bs;
    hash->bufflen = 0;
    read += sha01_bs;
    sha01_round(hash);
  }

  memset(hash->buffer, 0, sha01_bs);

  if(0 != total_add){
    memcpy(hash->buffer, input + read, total_add);
    hash->bufflen = total_add;
  }
}

void sha01_digest(struct sha01_hash* hash, uint8_t* out){
  uint8_t old[sha01_bs];
  struct sha01_hash old_hash;
  memcpy(&old_hash, hash, sizeof * hash);
  memcpy(old, hash->buffer, sha01_bs);

  hash->buffer[hash->bufflen] = 0x80;

  if(hash->bufflen > 55) {
    //too large, needs another buffer
    memset(hash->buffer + hash->bufflen + 1, 0, 64 - hash->bufflen - 1);
    sha01_round(hash);
    memset(hash->buffer, 0, 64);
  }

  size_t lhhh = 8*hash->total;
  for(int i = 0; i != 8; i++)
    hash->buffer[63 - i] = (uint8_t) (lhhh >> (i * 8) & 0xFF);
  sha01_round(hash);

  //sprintf(out, "%08x%08x%08x%08x%08x", hash->h0,hash->h1,hash->h2,hash->h3,hash->h4);
  ((uint32_t*)out)[0] = swap32(hash->h0); 
  ((uint32_t*)out)[1] = swap32(hash->h1);
  ((uint32_t*)out)[2] = swap32(hash->h2);
  ((uint32_t*)out)[3] = swap32(hash->h3);
  ((uint32_t*)out)[4] = swap32(hash->h4); 

  memcpy(hash, &old_hash, sizeof * hash);
  memcpy(hash->buffer, old, sha01_bs);
}

// lua ---

int l_sha01_update(lua_State* L){
  lua_getfield(L, 1, "_");
  struct sha01_hash* hash = lua_touserdata(L, -1);
  struct sha01_hash* clone = sha01_clone(hash);

  size_t len;
  uint8_t* input = (uint8_t*)luaL_checklstring(L, 2, &len);

  sha01_update(input, len, clone);
  l_sha01_obj(L, clone);

  return 1;
}

int l_sha01_digest(lua_State* L){
  lua_getfield(L, 1, "_");
  struct sha01_hash* hash = lua_touserdata(L, -1);

  uint8_t digest[20];
  sha01_digest(hash, digest);

  luaI_newbytes(L, (const uint8_t*)digest, sha01_digest_len);
  return 1;
}

int l_sha01_free(lua_State* L){
  lua_getfield(L, 1, "_");
  struct sha01_hash* hash = lua_touserdata(L, -1);

  sha01_free(hash);

  return 0;
}

int l_sha01_obj(lua_State* L, struct sha01_hash* hash){
  lua_newtable(L);
  int idx = lua_gettop(L);

  lua_newtable(L);
  int mt = lua_gettop(L);

  luaI_tsetcf(L, mt, "__gc", l_sha01_free);
  luaI_tsetcf(L, mt, "__add", l_sha01_update);

  luaI_tsetlud(L, idx, "_", hash);
  luaI_tsetcf(L, idx, "update", l_sha01_update);
  luaI_tsetcf(L, idx, "digest", l_sha01_digest);

  lua_pushvalue(L, mt);
  lua_setmetatable(L, idx);

  lua_settop(L, idx);
  return 1;
}

int l_sha1(lua_State* L){
  if(lua_gettop(L) == 0) return l_sha01_obj(L, sha01_init(1));
  size_t len = 0;
  char* in = (char*)luaL_checklstring(L, 1, &len);

  uint8_t digest[sha01_digest_len];
  struct sha01_hash* hash = sha01_init(1);
  sha01_update((uint8_t*)in, len, hash);
  sha01_digest(hash, digest);

  luaI_newbytes(L, (const uint8_t*)digest, sha01_digest_len);
  sha01_free(hash);
  return 1;
}

int l_sha0(lua_State* L){
  if(lua_gettop(L) == 0) return l_sha01_obj(L, sha01_init(0));
  size_t len = 0;
  char* in = (char*)luaL_checklstring(L, 1, &len);

  uint8_t digest[sha01_digest_len];
  struct sha01_hash* hash = sha01_init(0);
  sha01_update((uint8_t*)in, len, hash);
  sha01_digest(hash, digest);

  luaI_newbytes(L, (const uint8_t*)digest, sha01_digest_len);
  sha01_free(hash);
  return 1;
}
