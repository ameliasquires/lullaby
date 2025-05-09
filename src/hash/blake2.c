#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <stdlib.h>
#include "../crypto.h"

void mix2b(uint64_t* a, uint64_t* b, uint64_t* c, uint64_t* d, int64_t x, int64_t y){
    *a = *a + *b + x;
    *d = rotr64((*d ^  *a), 32);

    *c += *d;
    *b = rotr64((*b ^ *c), 24);

    *a += *b + y;
    *d = rotr64((*d ^ *a), 16);

    *c += *d;
    *b = rotr64((*b ^ *c), 63);
}

void mix2s(uint32_t* a, uint32_t* b, uint32_t* c, uint32_t* d, int32_t x, int32_t y){
    *a = *a + *b + x;
    *d = rotr32((*d ^  *a), 16);

    *c += *d;
    *b = rotr32((*b ^ *c), 12);

    *a += *b + y;
    *d = rotr32((*d ^ *a), 8);

    *c += *d;
    *b = rotr32((*b ^ *c), 7);
}

#define baking(type) \
    mix2##type(&v[0], &v[4], &v[8],  &v[12], m[s[0]], m[s[1]]);\
    mix2##type(&v[1], &v[5], &v[9],  &v[13], m[s[2]], m[s[3]]);\
    mix2##type(&v[2], &v[6], &v[10], &v[14], m[s[4]], m[s[5]]);\
    mix2##type(&v[3], &v[7], &v[11], &v[15], m[s[6]], m[s[7]]);\
    \
    mix2##type(&v[0], &v[5], &v[10], &v[15], m[s[8]],  m[s[9]]);\
    mix2##type(&v[1], &v[6], &v[11], &v[12], m[s[10]], m[s[11]]);\
    mix2##type(&v[2], &v[7], &v[8],  &v[13], m[s[12]], m[s[13]]);\
    mix2##type(&v[3], &v[4], &v[9],  &v[14], m[s[14]], m[s[15]]);

void compress2b(uint64_t* hash, uint8_t* inp, uint64_t compressed, int final){
    uint64_t v[16], s[16], m[16];

    #pragma unroll
    for(int i = 0; i != 8; i++)
        v[i] = hash[i];

    for(int i = 0; i != 16; i++)
        m[i] = ((uint64_t*)inp)[i];

    v[8] = sha512_iv.h0;
    v[9] = sha512_iv.h1;
    v[10] = sha512_iv.h2;
    v[11] = sha512_iv.h3;
    v[12] = sha512_iv.h4;
    v[13] = sha512_iv.h5;
    v[14] = sha512_iv.h6;
    v[15] = sha512_iv.h7;

    v[12] ^= compressed;
    v[13] ^= 0;
    
    if(final)
        v[14] ^= 0xFFFFFFFFFFFFFFFF;

    for(int i = 0; i != 12; i++){
        for(int j = 0; j != 16; j++){
            s[j] = blake2b_sigma[i%10][j];
        }

        baking(b);
    }
    
    for (int i = 0; i < 8; i++) {
        hash[i] = hash[i] ^ v[i] ^ v[i + 8];
    }
}

void compress2s(uint32_t* hash, uint8_t* inp, uint32_t compressed, int final){
    uint32_t v[16], s[16], m[16];

    #pragma unroll
    for(int i = 0; i != 8; i++)
        v[i] = hash[i];

    for(int i = 0; i != 16; i++)
        m[i] = ((uint32_t*)inp)[i];

    v[8] = sha512_iv.h0 >> 32;
    v[9] = sha512_iv.h1 >> 32;
    v[10] = sha512_iv.h2 >> 32;
    v[11] = sha512_iv.h3 >> 32;
    v[12] = sha512_iv.h4 >> 32;
    v[13] = sha512_iv.h5 >> 32;
    v[14] = sha512_iv.h6 >> 32;
    v[15] = sha512_iv.h7 >> 32;

    v[12] ^= compressed; //make this 64bit
    v[13] ^= 0;
    
    if(final)
        v[14] ^= 0xFFFFFFFFFFFFFFFF >> 32;

    for(int i = 0; i != 10; i++){
        for(int j = 0; j != 16; j++){
            s[j] = blake2b_sigma[i][j];
        }

        baking(s);
    }
    
    for (int i = 0; i < 8; i++) {
        hash[i] = hash[i] ^ v[i] ^ v[i + 8];
    }
}

struct blake2b_hash {
    uint8_t *buffer, *key;
    size_t bufflen, keylen;
    uint64_t total, *hash;
    uint64_t compressed, digest_len;
};

void blake2b_round(struct blake2b_hash* hash, int last){
  compress2b(hash->hash, hash->buffer, hash->compressed, last);
} 

#define bs_2 128
struct blake2b_hash blake2b_init(char* key, int key_len, int digest_len){
    struct blake2b_hash a = {.bufflen = key_len, .total = 0, .compressed = 0, .keylen = key_len, .digest_len = digest_len};
    a.buffer = calloc(sizeof * a.buffer, bs_2);
    a.hash = calloc(sizeof * a.hash, 8);
    a.key = calloc(sizeof* a.key, key_len);
    memcpy(a.key, key, key_len);
    memcpy(a.buffer, key, key_len);

    a.hash[0] = sha512_iv.h0;
    a.hash[0] ^= digest_len;
    a.hash[0] ^= key_len << 8;
    a.hash[0] ^= 0x01010000;
    a.hash[1] = sha512_iv.h1;
    a.hash[2] = sha512_iv.h2;
    a.hash[3] = sha512_iv.h3;
    a.hash[4] = sha512_iv.h4;
    a.hash[5] = sha512_iv.h5;
    a.hash[6] = sha512_iv.h6;
    a.hash[7] = sha512_iv.h7;
    if(key_len != 0){
        a.compressed = 128;
        blake2b_round(&a, 0);
        memset(a.buffer, 0, bs_2);
        a.bufflen = 0;
    }
    return a;
}

struct blake2b_hash blake2b_init_l(lua_State* L, char* key, int key_len, int digest_len){
    struct blake2b_hash a = {.bufflen = key_len, .total = 0, .compressed = 0, .keylen = key_len, .digest_len = digest_len};
    a.buffer = calloc(sizeof * a.buffer, bs_2);
    a.hash = calloc(sizeof * a.hash, 8);
    a.key = calloc(sizeof* a.key, key_len);
    memset(a.buffer, 0, bs_2);
    memcpy(a.key, key, key_len);
    memcpy(a.buffer, key, key_len);

    a.hash[0] = sha512_iv.h0;
    a.hash[0] ^= digest_len;
    a.hash[0] ^= key_len << 8;
    a.hash[0] ^= 0x01010000;
    a.hash[1] = sha512_iv.h1;
    a.hash[2] = sha512_iv.h2;
    a.hash[3] = sha512_iv.h3;
    a.hash[4] = sha512_iv.h4;
    a.hash[5] = sha512_iv.h5;
    a.hash[6] = sha512_iv.h6;
    a.hash[7] = sha512_iv.h7;
    if(key_len != 0){
        a.compressed = 128;
        blake2b_round(&a, 0);
        memset(a.buffer, 0, bs_2);
        a.bufflen = 0;
    }
    return a;
}

int blake2b_free_l(lua_State* L){
  struct blake2b_hash* h = lua_touserdata(L, -1);
  free(h->buffer);
  free(h->hash);
  free(h->key);
  return 0;
}

int blake2s_free_l(lua_State* L){
  struct blake2b_hash* h = lua_touserdata(L, -1);
  free(h->buffer);
  free(h->hash);
  free(h->key);
  return 0;
}

void blake2b_update(uint8_t* input, size_t len, struct blake2b_hash* hash){
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
        hash->compressed += 64;
        blake2b_round(hash, 0);
    }

    memset(hash->buffer, 0, bs_2);

    if(0 != total_add){
        memcpy(hash->buffer, input + read, total_add);
        hash->bufflen = total_add;
    }
}

void blake2b_final(struct blake2b_hash* hash, char* out_stream){
  uint8_t old[bs_2];
  uint64_t hashh[8];
  struct blake2b_hash old_hash;

  memcpy(&old_hash, hash, sizeof * hash);
  memcpy(old, hash->buffer, bs_2 * sizeof * old);
  memcpy(hashh, hash->hash, 8 * sizeof * hashh);

  hash->compressed += hash->bufflen;

  blake2b_round(hash, 1);

  for(int i = 0; i != hash->digest_len; i++)sprintf(out_stream + i * 2, "%02x", (((uint8_t*)hash->hash)[i]));
    
  memcpy(hash, &old_hash, sizeof * hash);
  memcpy(hash->buffer, old, bs_2 * sizeof * old);
  memcpy(hash->hash, hashh, 8 * sizeof * hashh);
}

void blake2b(uint8_t* inp, int len, char* key, int key_len, int dig_len, char* out){
    struct blake2b_hash aa = blake2b_init(key, key_len, dig_len);
    blake2b_update(inp, len, &aa);
    blake2b_final(&aa, out);
    free(aa.buffer);
    free(aa.hash);
    free(aa.key);
}

struct blake2s_hash {
    uint8_t *buffer, *key;
    size_t bufflen, keylen;
    uint32_t total, *hash;
    uint64_t compressed, digest_len;
};

void blake2s_round(struct blake2s_hash* hash, int last){
  compress2s(hash->hash, hash->buffer, hash->compressed, last);
} 

#define bs 64
struct blake2s_hash blake2s_init(char* key, int key_len, int digest_len){
    struct blake2s_hash a = {.bufflen = key_len, .total = 0, .compressed = 0, .keylen = key_len, .digest_len = digest_len};
    a.buffer = calloc(sizeof * a.buffer, bs);
    a.hash = calloc(sizeof * a.hash, 8);
    a.key = calloc(sizeof* a.key, key_len);
    memcpy(a.key, key, key_len);
    memcpy(a.buffer, key, key_len);

    a.hash[0] = sha512_iv.h0 >> 32;
    a.hash[0] ^= digest_len;
    a.hash[0] ^= key_len << 8;
    a.hash[0] ^= 0x01010000;
    a.hash[1] = sha512_iv.h1 >> 32;
    a.hash[2] = sha512_iv.h2 >> 32;
    a.hash[3] = sha512_iv.h3 >> 32;
    a.hash[4] = sha512_iv.h4 >> 32;
    a.hash[5] = sha512_iv.h5 >> 32;
    a.hash[6] = sha512_iv.h6 >> 32;
    a.hash[7] = sha512_iv.h7 >> 32;
    if(key_len != 0){
        a.compressed = 64;
        blake2s_round(&a, 0);
        memset(a.buffer, 0, bs);
        a.bufflen = 0;
    }
    return a;
}

struct blake2s_hash blake2s_init_l(lua_State* L, char* key, int key_len, int digest_len){
    struct blake2s_hash a = {.bufflen = key_len, .total = 0, .compressed = 0, .keylen = key_len, .digest_len = digest_len};
    a.buffer = calloc(sizeof * a.buffer, bs);
    a.hash = calloc(sizeof * a.hash, 8);
    a.key = calloc(sizeof* a.key, key_len);
    memcpy(a.key, key, key_len);
    memset(a.buffer, 0, bs);
    memcpy(a.buffer, key, key_len);
    
    a.hash[0] = sha512_iv.h0 >> 32;
    a.hash[0] ^= digest_len;
    a.hash[0] ^= key_len << 8;
    a.hash[0] ^= 0x01010000;
    a.hash[1] = sha512_iv.h1 >> 32;
    a.hash[2] = sha512_iv.h2 >> 32;
    a.hash[3] = sha512_iv.h3 >> 32;
    a.hash[4] = sha512_iv.h4 >> 32;
    a.hash[5] = sha512_iv.h5 >> 32;
    a.hash[6] = sha512_iv.h6 >> 32;
    a.hash[7] = sha512_iv.h7 >> 32;
    if(key_len != 0){
        a.compressed = 64; //TODO: allow for keys larger than 64 chars
        blake2s_round(&a, 0);
        memset(a.buffer, 0, bs);
        a.bufflen = 0;
    }
    return a;
}

void blake2s_update(uint8_t* input, size_t len, struct blake2s_hash* hash){
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
        blake2s_round(hash, 0);
    }

    memset(hash->buffer, 0, bs);

    if(0 != total_add){
        memcpy(hash->buffer, input + read, total_add);
        hash->bufflen = total_add;
    }
}

void blake2s_final(struct blake2s_hash* hash, char* out_stream){
  uint8_t old[bs];
  uint32_t hashh[8];
  struct blake2s_hash old_hash;
  memcpy(&old_hash, hash, sizeof * hash);
  memcpy(old, hash->buffer, bs);
  memcpy(hashh, hash->hash, 8 * sizeof * hashh);

    hash->compressed += hash->bufflen;

  if(hash->bufflen > 55) {
    //too large, needs another buffer
    memset(hash->buffer + hash->bufflen + 1, 0, 64 - hash->bufflen);
    blake2s_round(hash, 0);
    hash->compressed = 0;
    memset(hash->buffer, 0, 64);
  }

  blake2s_round(hash, 1);

  for(int i = 0; i != hash->digest_len; i++)sprintf(out_stream + i * 2, "%02x", (((uint8_t*)hash->hash)[i]));
    
  memcpy(hash, &old_hash, sizeof * hash);
  memcpy(hash->buffer, old, bs);
  memcpy(hash->hash, hashh, 8 * sizeof * hashh);
}

void blake2s(uint8_t* inp, int len, char* key, int key_len, int dig_len, char* out){
    struct blake2s_hash aa = blake2s_init(key, key_len, dig_len);
    blake2s_update(inp, len, &aa);
    blake2s_final(&aa, out);
    free(aa.buffer);
    free(aa.hash);
    free(aa.key);
}

int l_blake2b_clone(lua_State* L){

  struct blake2b_hash* a = (struct blake2b_hash*)lua_touserdata(L, -1);

  lua_pushinteger(L, a->digest_len);
  lua_pushlstring(L, (char*)a->key, a->keylen);
  l_blake2b_init(L);

  struct blake2b_hash* b = (struct blake2b_hash*)lua_touserdata(L, -1);

  memcpy(b->hash, a->hash, 8 * sizeof * b->hash);
  memcpy(b->buffer, a->buffer, bs * sizeof * b->buffer);
  
  b->keylen = a->keylen;
  b->digest_len = a->digest_len;
  b->keylen = a->keylen;
  b->total = a->total;
  b->bufflen = a->bufflen;
  b->compressed = a->compressed;
  b->total = a->total;
  return 1;
}

lua_common_hash_meta(blake2b);

int l_blake2b_init(lua_State* L){
  char* key = NULL;
  size_t keylen = 0, outlen = 64;

  if(lua_gettop(L) > 1){
     key = (char*)luaL_checklstring(L, -1, &keylen);
     outlen = luaL_checkinteger(L, -2);
  } else if(lua_gettop(L) > 0) outlen = luaL_checkinteger(L, -1);

  struct blake2b_hash* a = (struct blake2b_hash*)lua_newuserdata(L, sizeof * a);
  int ud = lua_gettop(L);
  *a = blake2b_init_l(L, key, keylen, outlen);
  lua_common_hash_meta_def(blake2b, blake2b_free_l);
  lua_pushvalue(L, ud);
  return 1;
}

lua_common_hash_update(blake2b, blake2b);

int l_blake2b_final(lua_State* L){
  struct blake2b_hash* a = (struct blake2b_hash*)lua_touserdata(L, -1);

  char digest[a->digest_len * 8];
  memset(digest, 0, a->digest_len * 8);
  blake2b_final(a, digest);

  lua_pushstring(L, digest);
  return 1;
}

int l_blake2b(lua_State* L){
    if(lua_gettop(L) == 0 || lua_type(L, 1) == LUA_TNUMBER) return l_blake2b_init(L);
    size_t len = 0;
    uint8_t* a = (uint8_t*)luaL_checklstring(L, 1, &len);
    int argv = lua_gettop(L); 

    uint32_t out_len = 64;
    if(argv > 1) out_len = luaL_checkinteger(L, 2);

    char* key = NULL;
    size_t key_len = 0;
    if(argv > 2) key = (char*)luaL_checklstring(L, 3, &key_len);
    
    char digest[out_len * 8];
    memset(digest, 0, out_len * 8);

    blake2b(a, len, key, key_len, out_len, digest);
    lua_pushstring(L, digest);

    return 1;
}

int l_blake2s_clone(lua_State* L){

  struct blake2s_hash* a = (struct blake2s_hash*)lua_touserdata(L, -1);

  lua_pushinteger(L, a->digest_len);
  lua_pushlstring(L, (char*)a->key, a->keylen);
  l_blake2s_init(L);

  struct blake2s_hash* b = (struct blake2s_hash*)lua_touserdata(L, -1);

  memcpy(b->hash, a->hash, 8 * sizeof * b->hash);
  memcpy(b->buffer, a->buffer, bs * sizeof * b->buffer);
  
  b->keylen = a->keylen;
  b->digest_len = a->digest_len;
  b->keylen = a->keylen;
  b->total = a->total;
  b->bufflen = a->bufflen;
  b->compressed = a->compressed;
  b->total = a->total;
  return 1;
}

lua_common_hash_meta(blake2s);

int l_blake2s_init(lua_State* L){
  char* key = NULL;
  size_t keylen = 0, outlen = 32;

  if(lua_gettop(L) > 1){
     key = (char*)luaL_checklstring(L, -1, &keylen);
     outlen = luaL_checkinteger(L, -2);
  } else if(lua_gettop(L) > 0) outlen = luaL_checkinteger(L, -1);

  struct blake2s_hash* a = (struct blake2s_hash*)lua_newuserdata(L, sizeof * a);
  int ud = lua_gettop(L);
  *a = blake2s_init_l(L, key, keylen, outlen);
  lua_common_hash_meta_def(blake2s, blake2s_free_l);
  lua_pushvalue(L, ud);
  return 1;
}

lua_common_hash_update(blake2s, blake2s);

int l_blake2s_final(lua_State* L){
  struct blake2s_hash* a = (struct blake2s_hash*)lua_touserdata(L, -1);

  char digest[a->digest_len * 8];
  memset(digest, 0, a->digest_len * 8);
  blake2s_final(a, digest);

  lua_pushstring(L, digest);
  return 1;
}

int l_blake2s(lua_State* L){
    if(lua_gettop(L) == 0 || lua_type(L, 1) == LUA_TNUMBER) return l_blake2s_init(L);
    size_t len = 0;
    uint8_t* a = (uint8_t*)luaL_checklstring(L, 1, &len);
    int argv = lua_gettop(L); 

    uint32_t out_len = 32;
    if(argv > 1) out_len = luaL_checkinteger(L, 2);

    char* key = NULL;
    size_t key_len = 0;
    if(argv > 2) key = (char*)luaL_checklstring(L, 3, &key_len);
    
    char digest[out_len * 8];
    memset(digest, 0, out_len * 8);

    blake2s(a, len, key, key_len, out_len, digest);
    lua_pushstring(L, digest);

    return 1;
}
