#include "../util.h"
#include "../crypto.h"
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <stdio.h>

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

void sha512_gen(uint64_t* out_stream, uint8_t* input, size_t len, struct iv sha_iv){
    uint64_t h0 = sha_iv.h0, h1 = sha_iv.h1, h2 = sha_iv.h2, h3 = sha_iv.h3, h4 = sha_iv.h4, h5 = sha_iv.h5, h6 = sha_iv.h6, h7 = sha_iv.h7;

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
       

    size_t blen = len*8;
    int ulen = 0;

    size_t l = len * 8;
    size_t k2 = (896 - ( (l  + 1) % 1024 )) % 1024;
    
    ulen = ((l + k2 + 1) / 8) + 16;
    uint64_t tlen = ulen/128;
    uint8_t* by = calloc(ulen, sizeof * by);
    for (size_t i = 0; i < len; i++)
        by[i] = input[i];
    by[len] = 0x80;

    //this part is very lame
    __uint128_t bigL = l;
    endian_swap128(&bigL);
    memcpy(&by[ulen - sizeof(__uint128_t)], &bigL, sizeof(__uint128_t));

    uint64_t *msg = ((uint64_t*)&by[0]);
    for (int i = 0; i < tlen * 16; i++)
        endian_swap64(msg++);

    for(int z = 0; z < (int)tlen; z++){
        uint64_t* M = ((uint64_t*)(by));
        uint64_t W[80];

        //i dont really understand this 0->16 part
        uint64_t *m = &M[(z * 16)];
        for (int i = 0; i < 16; ++i){
            W[i] = *m;
            ++m;
        }

        for (int i = 16; i != 80; i++){
            W[i] = (rotr64(W[i - 2],19) ^ rotr64(W[i - 2], 61) ^ (W[i - 2] >> 6)) 
                + W[i - 7] + (rotr64(W[i - 15],1) ^ rotr64(W[i - 15],8) ^ (W[i - 15] >> 7)) + W[i - 16];
        }

        uint64_t a = h0;
        uint64_t b = h1;
        uint64_t c = h2;
        uint64_t d = h3;
        uint64_t e = h4;
        uint64_t f = h5;
        uint64_t g = h6;
        uint64_t h = h7;
        
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
        
        h0 += a;
        h1 += b;
        h2 += c;
        h3 += d;
        h4 += e;
        h5 += f;
        h6 += g;
        h7 += h;
        
    }
    out_stream[0] = h0;
    out_stream[1] = h1;
    out_stream[2] = h2;
    out_stream[3] = h3;
    out_stream[4] = h4;
    out_stream[5] = h5;
    out_stream[6] = h6;
    out_stream[7] = h7;
    free(by);
    return;
}

struct iv sha_iv_gen(int i){
    struct iv oh = {.h0 = sha512_iv.h0 ^ 0xa5a5a5a5a5a5a5a5, .h1 = sha512_iv.h1 ^ 0xa5a5a5a5a5a5a5a5, .h2 = sha512_iv.h2 ^ 0xa5a5a5a5a5a5a5a5,
        .h3 = sha512_iv.h3 ^ 0xa5a5a5a5a5a5a5a5, .h4 = sha512_iv.h4 ^ 0xa5a5a5a5a5a5a5a5, .h5 = sha512_iv.h5 ^ 0xa5a5a5a5a5a5a5a5,
        .h6 = sha512_iv.h6 ^ 0xa5a5a5a5a5a5a5a5, .h7 = sha512_iv.h7 ^ 0xa5a5a5a5a5a5a5a5};
    
    uint64_t nh[8] = {0};
    uint8_t in[12];
    sprintf((char*)in, "SHA-512/%i",i);
    sha512_gen(nh, in, strlen((char*)in), oh);
    return (struct iv){.h0 = nh[0], .h1 = nh[1], .h2 = nh[2], .h3 = nh[3], .h4 = nh[4], .h5 = nh[5], .h6 = nh[6], .h7 = nh[7]};
}

void sha2_512_t(uint8_t* out, uint8_t* in, size_t len, int t){
    if(t%8!=0) return;
    uint64_t out_stream[8] = {0};
    sha512_gen(out_stream, in, len, sha_iv_gen(t));
    for(int i = 0; i != 8; i++) sprintf((char*)out, "%s%016llx", out, out_stream[i]);
    out[t/4] = '\0';
}

void sha2_512(uint8_t* out, uint8_t* in, size_t len){
    uint64_t out_stream[8] = {0};
    sha512_gen(out_stream, in, len, sha512_iv);
    for(int i = 0; i != 8; i++) sprintf((char*)out, "%s%016llx", out, out_stream[i]);
}

void sha2_384(uint8_t* out, uint8_t* in, size_t len){
    uint64_t out_stream[8] = {0};
    sha512_gen(out_stream, in, len, sha384_iv);
    for(int i = 0; i != 6; i++) sprintf((char*)out, "%s%016llx", out, out_stream[i]);
}

int l_sha512(lua_State* L){ 
  size_t len = 0;
  uint8_t* a = (uint8_t*)luaL_checklstring(L, 1, &len);

  char digest[512] = {0};

  sha2_512((uint8_t*)digest, a, len);
  lua_pushstring(L, digest);
  return 1;
}

int l_sha384(lua_State* L){ 
  size_t len = 0;
  uint8_t* a = (uint8_t*)luaL_checklstring(L, 1, &len);

  char digest[384] = {0};

  sha2_384((uint8_t*)digest, a, len);
  lua_pushstring(L, digest);
  return 1;
}

int l_sha512_t(lua_State* L){ 
  size_t len = 0;
  uint8_t* a = (uint8_t*)luaL_checklstring(L, 1, &len);
  uint64_t t = luaL_checkinteger(L, 2);

  char digest[512] = {0};

  sha2_512_t((uint8_t*)digest, a, t, len);
  lua_pushstring(L, digest);
  return 1;
}
