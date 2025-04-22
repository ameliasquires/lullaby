#include "../crypto.h"
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

#define bs 64

struct sha256_hash sha256_init(){
    struct sha256_hash a = {.h0 = 0x6a09e667, .h1 = 0xbb67ae85, .h2 = 0x3c6ef372, .h3 = 0xa54ff53a, .h4 = 0x510e527f, .h5 = 0x9b05688c, .h6 = 0x1f83d9ab, .h7 = 0x5be0cd19,
        .total = 0, .bufflen = 0};
    a.buffer = calloc(sizeof * a.buffer, bs);
    return a;
}

int sha256_free_l(lua_State* L){
  struct sha256_hash* h = lua_touserdata(L, -1);
  free(h->buffer);
  return 0;
}

#define sha224_free_l sha256_free_l

struct sha256_hash sha256_init_l(lua_State* L){
    struct sha256_hash a = {.h0 = 0x6a09e667, .h1 = 0xbb67ae85, .h2 = 0x3c6ef372, .h3 = 0xa54ff53a, .h4 = 0x510e527f, .h5 = 0x9b05688c, .h6 = 0x1f83d9ab, .h7 = 0x5be0cd19,
        .total = 0, .bufflen = 0};
    a.buffer = calloc(sizeof * a.buffer, bs);
    memset(a.buffer, 0, bs);
    return a;
}

struct sha256_hash sha224_init(){
    struct sha256_hash a = sha256_init();
    a.h0 = 0xc1059ed8;
    a.h1 = 0x367cd507;
    a.h2 = 0x3070dd17;
    a.h3 = 0xf70e5939;
    a.h4 = 0xffc00b31;
    a.h5 = 0x68581511;
    a.h6 = 0x64f98fa7;
    a.h7 = 0xbefa4fa4;
    return a;
}

struct sha256_hash sha224_init_l(lua_State* L){
    struct sha256_hash a = sha256_init_l(L);
    a.h0 = 0xc1059ed8;
    a.h1 = 0x367cd507;
    a.h2 = 0x3070dd17;
    a.h3 = 0xf70e5939;
    a.h4 = 0xffc00b31;
    a.h5 = 0x68581511;
    a.h6 = 0x64f98fa7;
    a.h7 = 0xbefa4fa4;
    return a;
}

void sha256_round(struct sha256_hash* hash){
    const uint32_t k[64] = {0x428a2f98, 0x71374491, 0xb5c0fbcf, 0xe9b5dba5, 0x3956c25b, 0x59f111f1, 0x923f82a4, 0xab1c5ed5,
        0xd807aa98, 0x12835b01, 0x243185be, 0x550c7dc3, 0x72be5d74, 0x80deb1fe, 0x9bdc06a7, 0xc19bf174,
        0xe49b69c1, 0xefbe4786, 0x0fc19dc6, 0x240ca1cc, 0x2de92c6f, 0x4a7484aa, 0x5cb0a9dc, 0x76f988da,
        0x983e5152, 0xa831c66d, 0xb00327c8, 0xbf597fc7, 0xc6e00bf3, 0xd5a79147, 0x06ca6351, 0x14292967,
        0x27b70a85, 0x2e1b2138, 0x4d2c6dfc, 0x53380d13, 0x650a7354, 0x766a0abb, 0x81c2c92e, 0x92722c85,
        0xa2bfe8a1, 0xa81a664b, 0xc24b8b70, 0xc76c51a3, 0xd192e819, 0xd6990624, 0xf40e3585, 0x106aa070,
        0x19a4c116, 0x1e376c08, 0x2748774c, 0x34b0bcb5, 0x391c0cb3, 0x4ed8aa4a, 0x5b9cca4f, 0x682e6ff3,
        0x748f82ee, 0x78a5636f, 0x84c87814, 0x8cc70208, 0x90befffa, 0xa4506ceb, 0xbef9a3f7, 0xc67178f2};
    uint32_t W[64];
    memset(W, 0, 64 * sizeof (uint32_t));

    for (int i = 0; i < 16; i++)
	    W[i] = (hash->buffer[i * 4] << 24) | (hash->buffer[i * 4 + 1] << 16) | (hash->buffer[i * 4 + 2] << 8) | (hash->buffer[i * 4 + 3]);
		
    for(int i = 16; i != 64; i++){
        uint32_t s0 = rotr32(W[i - 15], 7) ^ rotr32(W[i - 15], 18) ^ (W[i - 15] >> 3);
        uint32_t s1 = rotr32(W[i - 2], 17) ^ rotr32(W[i - 2], 19) ^ (W[i - 2] >> 10);
        W[i] = W[i - 16] + s0 + W[i - 7] + s1;
    }
        
    uint32_t a = hash->h0;
    uint32_t b = hash->h1;
    uint32_t c = hash->h2;
    uint32_t d = hash->h3;
    uint32_t e = hash->h4;
    uint32_t f = hash->h5;
    uint32_t g = hash->h6;
    uint32_t h = hash->h7;
        
    for(int i = 0; i != 64; i++){
        uint32_t S1 = rotr32(e, 6) ^ rotr32(e, 11) ^ rotr32(e, 25);
        uint32_t ch = (e & f) ^ ((~e) & g);
        uint32_t temp1 = h + S1 + ch + k[i] + W[i];
            
        uint32_t S0 = rotr32(a, 2) ^ rotr32(a, 13) ^ rotr32(a, 22);
        uint32_t maj = (a & b) ^ (a & c) ^ (b & c);
        uint32_t temp2 = S0 + maj;
            
        h = g;
        g = f;
        f = e;
        e = d + temp1;
        d = c;
        c = b;
        b = a;
        a = temp1 + temp2;
    }
        
    hash->h0 += a;
    hash->h1 += b;
    hash->h2 += c;
    hash->h3 += d;
    hash->h4 += e;
    hash->h5 += f;
    hash->h6 += g;
    hash->h7 += h;
}

#define sha224_update sha256_update
void sha256_update(uint8_t* input, size_t len, struct sha256_hash* hash){
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
    sha256_round(hash);
  }

  memset(hash->buffer, 0, bs);

  if(0 != total_add){
    memcpy(hash->buffer, input + read, total_add);
    hash->bufflen = total_add;
  }
}

void _sha256_final(struct sha256_hash* hash){
  hash->buffer[hash->bufflen] = 0x80;

  if(hash->bufflen > bs - 8) {
    //too large, needs another buffer
    memset(hash->buffer + hash->bufflen + 1, 0, bs - hash->bufflen);
    sha256_round(hash);
    memset(hash->buffer, 0, bs);
  }
    
  size_t blen = 8*hash->total;
  for(int i = 0; i != 8; i++)
    hash->buffer[63 - i] = (uint8_t) (blen >> (i * 8) & 0xFF);

  sha256_round(hash);
}

void sha256_final(struct sha256_hash* hash, char* out){
    uint8_t old[bs];
    struct sha256_hash old_hash;
    memcpy(&old_hash, hash, sizeof * hash);
    memcpy(old, hash->buffer, bs);

    _sha256_final(hash);
    sprintf(out, "%x%x%x%x%x%x%x%x",hash->h0,hash->h1,hash->h2,hash->h3,hash->h4,hash->h5,hash->h6,hash->h7);

    memcpy(hash, &old_hash, sizeof * hash);
    memcpy(hash->buffer, old, bs);
}

void sha224_final(struct sha256_hash* hash, char* out){
    uint8_t old[bs];
    struct sha256_hash old_hash;
    memcpy(&old_hash, hash, sizeof * hash);
    memcpy(old, hash->buffer, bs);

    _sha256_final(hash);
    sprintf(out, "%x%x%x%x%x%x%x",hash->h0,hash->h1,hash->h2,hash->h3,hash->h4,hash->h5,hash->h6);

    memcpy(hash, &old_hash, sizeof * hash);
    memcpy(hash->buffer, old, bs);
}

void sha256(uint8_t* in, size_t len, char* out){
    struct sha256_hash a = sha256_init();
    sha256_update(in, len, &a);
    sha256_final(&a, out);
    free(a.buffer);
}

void sha224(uint8_t* in, size_t len, char* out){
    struct sha256_hash a = sha224_init();
    sha224_update(in, len, &a);
    sha224_final(&a, out);
    free(a.buffer);
}

//common_hash_clone(sha256);
lua_common_hash_clone_oargs(sha256, sha256, l_sha256_init(L), {
    uint8_t* old = b->buffer;
    *b = *a;
    b->buffer = old;
    memcpy(b->buffer, a->buffer, bs * sizeof * b->buffer);
});
lua_common_hash_init_l(sha256, sha256);
lua_common_hash_update(sha256, sha256);

int l_sha256_final(lua_State* L){
  struct sha256_hash* a = (struct sha256_hash*)lua_touserdata(L, 1);

  char digest[256];
  sha256_final(a, digest);

  lua_pushstring(L, digest);
  return 1;
}

int l_sha256(lua_State* L){
  if(lua_gettop(L) == 0) return l_sha256_init(L);
  size_t len = 0;
  uint8_t* a = (uint8_t*)luaL_checklstring(L, 1, &len);
  
  char digest[256];

  sha256(a, len, digest);
  lua_pushstring(L, digest);

  return 1;
};

#define sha224_hash sha256_hash
//common_hash_clone(sha224);
lua_common_hash_clone_oargs(sha224, sha224, l_sha224_init(L), {
    uint8_t* old = b->buffer;
    *b = *a;
    b->buffer = old;
    memcpy(b->buffer, a->buffer, bs * sizeof * b->buffer);
});
lua_common_hash_init_l(sha224, sha224);
lua_common_hash_update(sha224, sha224);

int l_sha224_final(lua_State* L){
  struct sha224_hash* a = (struct sha224_hash*)lua_touserdata(L, 1);

  char digest[224];
  sha224_final(a, digest);

  lua_pushstring(L, digest);
  return 1;
}

int l_sha224(lua_State* L){
  if(lua_gettop(L) == 0) return l_sha224_init(L);
  size_t len = 0;
  uint8_t* a = (uint8_t*)luaL_checklstring(L, 1, &len);
  
  char digest[224];

  sha224(a, len, digest);
  lua_pushstring(L, digest);

  return 1;
};
