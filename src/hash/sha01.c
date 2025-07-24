#include "../crypto.h"
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

#define bs 64

struct sha01_hash {
  uint8_t* buffer;
  uint32_t h0, h1, h2, h3, h4;
  size_t bufflen;
  size_t total;
  uint8_t version;
};

#define sha0_hash sha01_hash 
#define sha1_hash sha01_hash

struct sha01_hash sha01_init(uint8_t ver){
    struct sha01_hash a = {.h0 = 0x67452301, .h1 = 0xEFCDAB89, .h2 = 0x98BADCFE, .h3 = 0x10325476, .h4 = 0xC3D2E1F0,
        .total = 0, .bufflen = 0, .version = ver};
    a.buffer = calloc(sizeof * a.buffer, bs);
    return a;
}

int sha01_free_l(lua_State* L){
  struct sha01_hash* h = lua_touserdata(L, -1);
  free(h->buffer);
  return 0;
}

struct sha01_hash sha01_init_l(uint8_t ver, lua_State* L){
    struct sha01_hash a = {.h0 = 0x67452301, .h1 = 0xEFCDAB89, .h2 = 0x98BADCFE, .h3 = 0x10325476, .h4 = 0xC3D2E1F0,
        .total = 0, .bufflen = 0, .version = ver};
    a.buffer = calloc(sizeof * a.buffer, bs);
    memset(a.buffer, 0, bs);
    return a;
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
    sha01_round(hash);
  }

  memset(hash->buffer, 0, bs);

  if(0 != total_add){
    memcpy(hash->buffer, input + read, total_add);
    hash->bufflen = total_add;
  }
}

void sha01_final(struct sha01_hash* hash, char* out_stream){
  uint8_t old[bs];
  struct sha01_hash old_hash;
  memcpy(&old_hash, hash, sizeof * hash);
  memcpy(old, hash->buffer, bs);

  hash->buffer[hash->bufflen] = 0x80;

  if(hash->bufflen > 55) {
    //too large, needs another buffer
    memset(hash->buffer + hash->bufflen + 1, 0, 64 - hash->bufflen);
    sha01_round(hash);
    memset(hash->buffer, 0, 64);
  }

  size_t lhhh = 8*hash->total;
  for(int i = 0; i != 8; i++)
        hash->buffer[63 - i] = (uint8_t) (lhhh >> (i * 8) & 0xFF);
  sha01_round(hash);

  sprintf(out_stream,"%02x%02x%02x%02x%02x",hash->h0,hash->h1,hash->h2,hash->h3,hash->h4);

  memcpy(hash, &old_hash, sizeof * hash);
  memcpy(hash->buffer, old, bs);
}

struct sha01_hash sha0_init(){
    return sha01_init(0);
}

struct sha01_hash sha1_init(){
    return sha01_init(1);
}

void sha0_update(uint8_t* input, size_t len, struct sha01_hash* hash){
    sha01_update(input, len, hash);
}

void sha1_update(uint8_t* input, size_t len, struct sha01_hash* hash){
    sha01_update(input, len, hash);
}

void sha0_final(struct sha01_hash* hash, char* out_stream){
    sha01_final(hash, out_stream);
}

void sha1_final(struct sha01_hash* hash, char* out_stream){
    sha01_final(hash, out_stream);
}

void sha0(uint8_t* a, size_t len, char* out_stream){
    struct sha01_hash aa = sha0_init();
    sha0_update(a, len, &aa);
    sha0_final(&aa, out_stream);
    free(aa.buffer);
}

void sha1(uint8_t* a, size_t len, char* out_stream){
    struct sha01_hash aa = sha1_init();
    sha1_update(a, len, &aa);
    sha1_final(&aa, out_stream);
    free(aa.buffer);
}

//common_hash_clone(sha1);
lua_common_hash_clone_oargs(sha1, sha1, l_sha1_init(L), {
    uint8_t* old = b->buffer;
    *b = *a;
    b->buffer = old;
    memcpy(b->buffer, a->buffer, bs * sizeof * b->buffer);
});

lua_common_hash_init_ni(sha1, sha1, sha01_init_l(1, L), sha01_free_l);
lua_common_hash_update(sha1, sha1);

//common_hash_clone(sha0);
lua_common_hash_clone_oargs(sha0, sha0, l_sha0_init(L), {
    uint8_t* old = b->buffer;
    *b = *a;
    b->buffer = old;
    memcpy(b->buffer, a->buffer, bs * sizeof * b->buffer);
});
lua_common_hash_init_ni(sha0, sha0, sha01_init_l(0, L), sha01_free_l);
lua_common_hash_update(sha0, sha0);

int l_sha1_final(lua_State* L){
  struct sha01_hash* a = (struct sha01_hash*)lua_touserdata(L, 1);

  char digest[160];
  sha1_final(a, digest);

  lua_pushstring(L, digest);
  return 1;
}

int l_sha0_final(lua_State* L){
    return l_sha1_final(L);
}

int l_sha1(lua_State* L){
  if(lua_gettop(L) == 0) return l_sha1_init(L);
  size_t len = 0;
  char* a = (char*)luaL_checklstring(L, 1, &len);

  char digest[160];

  sha1((uint8_t*)a, len, digest);
  lua_pushstring(L, digest);

  return 1;
};

int l_sha0(lua_State* L){
  if(lua_gettop(L) == 0) return l_sha0_init(L);
  size_t len = 0;
  char* a = (char*)luaL_checklstring(L, 1, &len);
  
  char digest[160];

  sha0((uint8_t*)a, len, digest);
  lua_pushstring(L, digest);

  return 1;
};
