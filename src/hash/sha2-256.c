#include "../crypto.h"
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <stdio.h>
#include <inttypes.h>

const uint64_t k[80] = {0x428a2f98d728ae22, 0x7137449123ef65cd, 0xb5c0fbcfec4d3b2f, 0xe9b5dba58189dbbc, 0x3956c25bf348b538, 
  0x59f111f1b605d019, 0x923f82a4af194f9b, 0xab1c5ed5da6d8118, 0xd807aa98a3030242, 0x12835b0145706fbe, 
  0x243185be4ee4b28c, 0x550c7dc3d5ffb4e2, 0x72be5d74f27b896f, 0x80deb1fe3b1696b1, 0x9bdc06a725c71235, 
  0xc19bf174cf692694, 0xe49b69c19ef14ad2, 0xefbe4786384f25e3, 0x0fc19dc68b8cd5b5, 0x240ca1cc77ac9c65, 
  0x2de92c6f592b0275, 0x4a7484aa6ea6e483, 0x5cb0a9dcbd41fbd4, 0x76f988da831153b5, 0x983e5152ee66dfab, 
  0xa831c66d2db43210, 0xb00327c898fb213f, 0xbf597fc7beef0ee4, 0xc6e00bf33da88fc2, 0xd5a79147930aa725, 
  0x06ca6351e003826f, 0x142929670a0e6e70, 0x27b70a8546d22ffc, 0x2e1b21385c26c926, 0x4d2c6dfc5ac42aed, 
  0x53380d139d95b3df, 0x650a73548baf63de, 0x766a0abb3c77b2a8, 0x81c2c92e47edaee6, 0x92722c851482353b, 
  0xa2bfe8a14cf10364, 0xa81a664bbc423001, 0xc24b8b70d0f89791, 0xc76c51a30654be30, 0xd192e819d6ef5218, 
  0xd69906245565a910, 0xf40e35855771202a, 0x106aa07032bbd1b8, 0x19a4c116b8d2d0c8, 0x1e376c085141ab53, 
  0x2748774cdf8eeb99, 0x34b0bcb5e19b48a8, 0x391c0cb3c5c95a63, 0x4ed8aa4ae3418acb, 0x5b9cca4f7763e373, 
  0x682e6ff3d6b2b8a3, 0x748f82ee5defb2fc, 0x78a5636f43172f60, 0x84c87814a1f0ab72, 0x8cc702081a6439ec, 
  0x90befffa23631e28, 0xa4506cebde82bde9, 0xbef9a3f7b2c67915, 0xc67178f2e372532b, 0xca273eceea26619c, 
  0xd186b8c721c0c207, 0xeada7dd6cde0eb1e, 0xf57d4f7fee6ed178, 0x06f067aa72176fba, 0x0a637dc5a2c898a6, 
  0x113f9804bef90dae, 0x1b710b35131c471b, 0x28db77f523047d84, 0x32caab7b40c72493, 0x3c9ebe0a15c9bebc, 
  0x431d67c49c100d4c, 0x4cc5d4becb3e42b6, 0x597f299cfc657e2a, 0x5fcb6fab3ad6faec, 0x6c44198c4a475817};

void endian_swap128(__uint128_t *x){
  uint8_t *y = (uint8_t*)x;
  for (size_t low = 0, high = sizeof(__uint128_t) - 1; high > low; low++, high--){
    y[low]  ^= y[high];
    y[high] ^= y[low];
    y[low]  ^= y[high];
  }
}

void endian_swap64(uint64_t *x){
  uint8_t *y = (uint8_t*)x;
  for (size_t low = 0, high = sizeof(uint64_t) - 1; high > low; low++, high--){
    y[low]  ^= y[high];
    y[high] ^= y[low];
    y[low]  ^= y[high];
  }
}

#define bs 128

void sha512_round(struct sha512_hash* hash){
  uint64_t *msg = ((uint64_t*)&hash->buffer[0]);
  for(int i = 0; i < 16; i++)
    endian_swap64(msg++);

  uint64_t* M = ((uint64_t*)(hash->buffer));
  uint64_t W[80];

  //i dont really understand this 0->16 part
  int z = 0;
  uint64_t *m = &M[(z * 16)];
  for(int i = 0; i < 16; ++i){
    W[i] = *m;
    m++;
  }

  for(int i = 16; i != 80; i++){
    W[i] = (rotr64(W[i - 2],19) ^ rotr64(W[i - 2], 61) ^ (W[i - 2] >> 6)) 
      + W[i - 7] + (rotr64(W[i - 15],1) ^ rotr64(W[i - 15],8) ^ (W[i - 15] >> 7)) + W[i - 16];
  }

  uint64_t a = hash->h0;
  uint64_t b = hash->h1;
  uint64_t c = hash->h2;
  uint64_t d = hash->h3;
  uint64_t e = hash->h4;
  uint64_t f = hash->h5;
  uint64_t g = hash->h6;
  uint64_t h = hash->h7;

  for(int i = 0; i != 80; i++){
    uint64_t S1 = rotr64(e, 14) ^ rotr64(e, 18) ^ rotr64(e, 41);
    uint64_t ch = (e & f) ^ ((~e) & g);
    uint64_t temp1 = h + S1 + ch + k[i] + W[i];

    uint64_t S0 = rotr64(a, 28) ^ rotr64(a, 34) ^ rotr64(a, 39);
    uint64_t maj = (a & b) ^ (a & c) ^ (b & c);
    uint64_t temp2 = S0 + maj;

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

struct sha512_hash sha512_t_init(struct iv sha_iv){
  struct sha512_hash a = {.h0 = sha_iv.h0, .h1 = sha_iv.h1, .h2 = sha_iv.h2, .h3 = sha_iv.h3, .h4 = sha_iv.h4, .h5 = sha_iv.h5, .h6 = sha_iv.h6, .h7 = sha_iv.h7,
    .total = 0, .bufflen = 0};
  a.buffer = calloc(sizeof * a.buffer, bs);
  return a;
}

int sha512_t_free_l(lua_State* L){
  struct sha512_hash* h = lua_touserdata(L, -1);
  free(h->buffer);
  return 0;
}

struct sha512_hash sha512_t_init_l(struct iv sha_iv, lua_State* L){
  struct sha512_hash a = {.h0 = sha_iv.h0, .h1 = sha_iv.h1, .h2 = sha_iv.h2, .h3 = sha_iv.h3, .h4 = sha_iv.h4, .h5 = sha_iv.h5, .h6 = sha_iv.h6, .h7 = sha_iv.h7,
    .total = 0, .bufflen = 0};
  a.buffer = calloc((sizeof * a.buffer), bs);
  memset(a.buffer, 0, bs);
  return a;
}

struct sha512_hash sha512_init(){
  return sha512_t_init(sha512_iv);
}

struct sha512_hash sha384_init(){
  return sha512_t_init(sha384_iv);
}

char old[512];
void sha512_update(uint8_t* input, size_t len, struct sha512_hash* hash){
  hash->total += len;
  size_t total_add = len + hash->bufflen;
  size_t read = 0;
  if(total_add < bs){
    memcpy(hash->buffer + hash->bufflen, input, len);
    hash->bufflen += len;
    memcpy(old, hash->buffer, hash->bufflen);
    return;
  }

  for(; total_add >= bs;){
    memcpy(hash->buffer + hash->bufflen, input + read, bs - hash->bufflen);
    total_add -= bs;
    hash->bufflen = 0;
    read += bs;
    sha512_round(hash);
  }

  memset(hash->buffer, 0, bs);

  if(0 != total_add){
    memcpy(hash->buffer, input + read, total_add);
    hash->bufflen = total_add;
  }
}

void _sha512_t_final(struct sha512_hash* hash){
  hash->buffer[hash->bufflen] = 0x80;

  if(hash->bufflen > bs - 16) {
    //too large, needs another buffer
    memset(hash->buffer + hash->bufflen + 1, 0, bs - hash->bufflen);
    sha512_round(hash);
    memset(hash->buffer, 0, bs);
  }

  __uint128_t bigL = hash->total*8;
  endian_swap128(&bigL);
  memcpy(&hash->buffer[128 - sizeof(__uint128_t)], &bigL, sizeof(__uint128_t));

  sha512_round(hash);
}

void sha512_final(struct sha512_hash* hash, char* out_stream){
  uint8_t old[bs];
  struct sha512_hash old_hash;
  memcpy(&old_hash, hash, sizeof * hash);
  memcpy(old, hash->buffer, bs);

  _sha512_t_final(hash);

  sprintf((char*)out_stream, "%016"PRIx64"%016"PRIx64"%016"PRIx64"%016"PRIx64"%016"PRIx64"%016"PRIx64"%016"PRIx64"%016"PRIx64
      , hash->h0, hash->h1, hash->h2, hash->h3, hash->h4, hash->h5, hash->h6, hash->h7);
  /*sprintf((char*)out_stream, "%s%016llx", out_stream, hash->h1);
    sprintf((char*)out_stream, "%s%016llx", out_stream, hash->h2);
    sprintf((char*)out_stream, "%s%016llx", out_stream, hash->h3);
    sprintf((char*)out_stream, "%s%016llx", out_stream, hash->h4);
    sprintf((char*)out_stream, "%s%016llx", out_stream, hash->h5);
    sprintf((char*)out_stream, "%s%016llx", out_stream, hash->h6);
    sprintf((char*)out_stream, "%s%016llx", out_stream, hash->h7);*/

  memcpy(hash, &old_hash, sizeof * hash);
  memcpy(hash->buffer, old, bs);
}

void sha384_final(struct sha512_hash* hash, char* out_stream){
  uint8_t old[bs];
  struct sha512_hash old_hash;
  memcpy(&old_hash, hash, sizeof * hash);
  memcpy(old, hash->buffer, bs);
  _sha512_t_final(hash);


  sprintf((char*)out_stream, "%016"PRIx64"%016"PRIx64"%016"PRIx64"%016"PRIx64"%016"PRIx64"%016"PRIx64, hash->h0, hash->h1, hash->h2, hash->h3, hash->h4, hash->h5);
  /*sprintf((char*)out_stream, "%s%016llx", out_stream, hash->h1);
    sprintf((char*)out_stream, "%s%016llx", out_stream, hash->h2);
    sprintf((char*)out_stream, "%s%016llx", out_stream, hash->h3);
    sprintf((char*)out_stream, "%s%016llx", out_stream, hash->h4);
    sprintf((char*)out_stream, "%s%016llx", out_stream, hash->h5);*/

  memcpy(hash, &old_hash, sizeof * hash);
  memcpy(hash->buffer, old, bs);
}

void sha512(uint8_t* in, size_t len, char* out){
  struct sha512_hash a = sha512_init();
  sha512_update(in, len, &a);
  sha512_final(&a, out);
  free(a.buffer);
}

void sha384(uint8_t* in, size_t len, char* out){
  struct sha512_hash a = sha384_init();
  sha384_update(in, len, &a);
  sha384_final(&a, out);
  free(a.buffer);
}

void sha512_t(uint8_t* in, size_t len, int t, char* out){
  struct sha512_hash a = sha512_t_init(sha_iv_gen(t));
  sha512_update(in, len, &a);
  sha512_final(&a, out);
  out[t/4] = '\0';
  free(a.buffer);
}

struct iv sha_iv_gen(int i){
  struct iv oh = {.h0 = sha512_iv.h0 ^ 0xa5a5a5a5a5a5a5a5, .h1 = sha512_iv.h1 ^ 0xa5a5a5a5a5a5a5a5, .h2 = sha512_iv.h2 ^ 0xa5a5a5a5a5a5a5a5,
    .h3 = sha512_iv.h3 ^ 0xa5a5a5a5a5a5a5a5, .h4 = sha512_iv.h4 ^ 0xa5a5a5a5a5a5a5a5, .h5 = sha512_iv.h5 ^ 0xa5a5a5a5a5a5a5a5,
    .h6 = sha512_iv.h6 ^ 0xa5a5a5a5a5a5a5a5, .h7 = sha512_iv.h7 ^ 0xa5a5a5a5a5a5a5a5};

  uint8_t in[12];
  sprintf((char*)in, "SHA-512/%i", i);
  struct sha512_hash a = sha512_t_init(oh);
  sha512_update(in, strlen((char*)in), &a);
  _sha512_t_final(&a);
  free(a.buffer);
  return (struct iv){.h0 = a.h0, .h1 = a.h1, .h2 = a.h2, .h3 = a.h3, .h4 = a.h4, .h5 = a.h5, .h6 = a.h6, .h7 = a.h7};
}

//common_hash_clone(sha512);
lua_common_hash_clone_oargs(sha512, sha512, l_sha512_init(L), {
    uint8_t* old = b->buffer;
    *b = *a;
    b->buffer = old;
    memcpy(b->buffer, a->buffer, bs * sizeof * b->buffer);
    });
lua_common_hash_init_ni(sha512, sha512, sha512_t_init_l(sha512_iv, L), sha512_t_free_l);
lua_common_hash_update(sha512, sha512);

int l_sha512_final(lua_State* L){
  struct sha512_hash* a = (struct sha512_hash*)lua_touserdata(L, 1);

  char digest[512] = {0};
  sha512_final(a, digest);

  lua_pushstring(L, digest);
  return 1;
}

//common_hash_clone(sha384);
lua_common_hash_clone_oargs(sha384, sha384, l_sha384_init(L), {
    uint8_t* old = b->buffer;
    *b = *a;
    b->buffer = old;
    memcpy(b->buffer, a->buffer, bs * sizeof * b->buffer);
    });
lua_common_hash_init_ni(sha384, sha384, sha512_t_init_l(sha384_iv, L), sha512_t_free_l);
lua_common_hash_update(sha384, sha384);

int l_sha384_final(lua_State* L){
  struct sha512_hash* a = (struct sha512_hash*)lua_touserdata(L, 1);

  char digest[384] = {0};
  sha384_final(a, digest);

  lua_pushstring(L, digest);
  return 1;
}

int l_sha512_t_clone(lua_State* L){
  struct sha512_hash* a = (struct sha512_hash*)lua_touserdata(L, -1);
  lua_pushinteger(L, a->t);
  l_sha512_t_init(L);
  struct sha512_hash* b = (struct sha512_hash*)lua_touserdata(L, -1);

  uint8_t* old = b->buffer;
  *b = *a;
  b->buffer = old;
  memcpy(b->buffer, a->buffer, bs * sizeof * b->buffer);
  return 1;
}

lua_common_hash_meta(sha512_t);
int l_sha512_t_init(lua_State* L){
  int tt = luaL_checkinteger(L, -1);
  lua_newtable(L);

  struct sha512_hash* a = (struct sha512_hash*)lua_newuserdata(L, sizeof * a);\
                          int ud = lua_gettop(L);
  *a = sha512_t_init_l(sha_iv_gen(tt), L);
  a->t = tt;

  lua_common_hash_meta_def(sha512_t, sha512_t_free_l);

  lua_pushvalue(L, ud);
  return 1;
}

lua_common_hash_update(sha512, sha512_t);

int l_sha512_t_final(lua_State* L){
  struct sha512_hash* a = (struct sha512_hash*)lua_touserdata(L, 1);

  char digest[512] = {0};
  sha512_final(a, digest);
  digest[a->t/4] = '\0';

  lua_pushstring(L, digest);
  return 1;
}

int l_sha512(lua_State* L){ 
  if(lua_gettop(L) == 0) return l_sha512_init(L);
  size_t len = 0;
  uint8_t* a = (uint8_t*)luaL_checklstring(L, 1, &len);
  char digest[512] = {0};

  sha512(a, len, digest);
  lua_pushstring(L, digest);
  return 1;
}

int l_sha384(lua_State* L){ 
  if(lua_gettop(L) == 0) return l_sha384_init(L);
  size_t len = 0;
  uint8_t* a = (uint8_t*)luaL_checklstring(L, 1, &len);

  char digest[384] = {0};

  sha384(a, len, digest);
  lua_pushstring(L, digest);
  return 1;
}

int l_sha512_t(lua_State* L){ 
  if(lua_gettop(L) == 1) return l_sha512_t_init(L);
  size_t len = 0;
  uint8_t* a = (uint8_t*)luaL_checklstring(L, 1, &len);
  uint64_t t = luaL_checkinteger(L, 2);

  char digest[512] = {0};

  sha512_t(a, len, t, digest);
  lua_pushstring(L, digest);
  return 1;
}
