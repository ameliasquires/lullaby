#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <stdlib.h>
#include <math.h>
#include "../crypto.h"
//#include "blake2.h"
#include "../util.h"

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

void blake2b(char* inp, int inp_len, char* key, int key_len, int dig_len, char* buffer){
    uint64_t hash[8];
    
    uint64_t iv0 = hash[0] = sha512_iv.h0;
    uint64_t iv1 = hash[1] = sha512_iv.h1;
    uint64_t iv2 = hash[2] = sha512_iv.h2;
    uint64_t iv3 = hash[3] = sha512_iv.h3;
    uint64_t iv4 = hash[4] = sha512_iv.h4;
    uint64_t iv5 = hash[5] = sha512_iv.h5;
    uint64_t iv6 = hash[6] = sha512_iv.h6;
    uint64_t iv7 = hash[7] = sha512_iv.h7;

    uint64_t alen = inter(inp_len, 128) + 128;

    
    //add padding
    char* padded = calloc(alen + (128 * (key_len > 0)), sizeof * padded);

    if(key_len > 0){
        memcpy(padded, key, key_len);
        inp_len += 128;
    }

    memcpy(padded + (128 * (key_len > 0)), inp, inp_len - (128 * (key_len > 0)));

    hash[0] ^= dig_len;
    hash[0] ^= key_len << 8;
    hash[0] ^= 0x01010000;

    uint64_t compressed = 0, bytes_remaining = inp_len;

    int i = 0;
    for(;bytes_remaining > 128; i += 2){
        bytes_remaining -= 128;
        compressed += 128;

        compress2b(hash, (uint8_t*)padded, compressed, 0);
        padded += 128;
    }

    compressed += bytes_remaining;

    compress2b(hash, (uint8_t*)padded, compressed, 1);
    for(int i = 0; i != dig_len; i++)sprintf(buffer, "%s%02x", buffer, (((uint8_t*)hash)[i]));
}

void blake2s(char* inp, int inp_len, char* key, int key_len, int dig_len, char* buffer){
    uint32_t hash[8];
    
    uint32_t iv0 = hash[0] = sha512_iv.h0 >> 32;
    uint32_t iv1 = hash[1] = sha512_iv.h1 >> 32;
    uint32_t iv2 = hash[2] = sha512_iv.h2 >> 32;
    uint32_t iv3 = hash[3] = sha512_iv.h3 >> 32;
    uint32_t iv4 = hash[4] = sha512_iv.h4 >> 32;
    uint32_t iv5 = hash[5] = sha512_iv.h5 >> 32;
    uint32_t iv6 = hash[6] = sha512_iv.h6 >> 32;
    uint32_t iv7 = hash[7] = sha512_iv.h7 >> 32;

    uint32_t alen = inter(inp_len, 64) + 64;

    //add padding
    char* padded = calloc(alen + (64 * (key_len > 0)), sizeof * padded);
    if(key_len > 0){
        memcpy(padded, key, key_len);
        inp_len += 64;
    }
    memcpy(padded + (64 * (key_len > 0)), inp, inp_len - (64 * (key_len > 0)));

    hash[0] ^= dig_len;
    hash[0] ^= key_len << 8;
    hash[0] ^= 0x01010000;

    uint64_t compressed = 0, bytes_remaining = inp_len;

    int i = 0;
    for(;bytes_remaining > 64; i += 2){
        bytes_remaining -= 64;
        compressed += 64;

        compress2s(hash, (uint8_t*)padded, compressed, 0);
        padded += 64;
    }

    compressed += bytes_remaining;

    compress2s(hash, (uint8_t*)padded, compressed, 1);
    for(int i = 0; i != dig_len; i++)sprintf(buffer, "%s%02x", buffer, (((uint8_t*)hash)[i]));
}

int l_blake2b(lua_State* L){
    size_t len = 0;
    char* a = (char*)luaL_checklstring(L, 1, &len);
    int argv = lua_gettop(L); 

    uint64_t out_len = 64;
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

int l_blake2s(lua_State* L){
    size_t len = 0;
    char* a = (char*)luaL_checklstring(L, 1, &len);
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