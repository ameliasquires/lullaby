#include "../crypto.h"
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

void i_sha2xx(enum version v, char* out_stream, char* input){
    uint32_t h0, h1, h2, h3, h4, h5, h6, h7;
    
    if(v == sha256){
        h0 = 0x6a09e667;
        h1 = 0xbb67ae85;
        h2 = 0x3c6ef372;
        h3 = 0xa54ff53a;
        h4 = 0x510e527f;
        h5 = 0x9b05688c;
        h6 = 0x1f83d9ab;
        h7 = 0x5be0cd19;
    } else if (v == sha224){
        h0 = 0xc1059ed8;
        h1 = 0x367cd507;
        h2 = 0x3070dd17;
        h3 = 0xf70e5939;
        h4 = 0xffc00b31;
        h5 = 0x68581511;
        h6 = 0x64f98fa7;
        h7 = 0xbefa4fa4;
    }
    
    const uint32_t k[64] = {0x428a2f98, 0x71374491, 0xb5c0fbcf, 0xe9b5dba5, 0x3956c25b, 0x59f111f1, 0x923f82a4, 0xab1c5ed5,
        0xd807aa98, 0x12835b01, 0x243185be, 0x550c7dc3, 0x72be5d74, 0x80deb1fe, 0x9bdc06a7, 0xc19bf174,
        0xe49b69c1, 0xefbe4786, 0x0fc19dc6, 0x240ca1cc, 0x2de92c6f, 0x4a7484aa, 0x5cb0a9dc, 0x76f988da,
        0x983e5152, 0xa831c66d, 0xb00327c8, 0xbf597fc7, 0xc6e00bf3, 0xd5a79147, 0x06ca6351, 0x14292967,
        0x27b70a85, 0x2e1b2138, 0x4d2c6dfc, 0x53380d13, 0x650a7354, 0x766a0abb, 0x81c2c92e, 0x92722c85,
        0xa2bfe8a1, 0xa81a664b, 0xc24b8b70, 0xc76c51a3, 0xd192e819, 0xd6990624, 0xf40e3585, 0x106aa070,
        0x19a4c116, 0x1e376c08, 0x2748774c, 0x34b0bcb5, 0x391c0cb3, 0x4ed8aa4a, 0x5b9cca4f, 0x682e6ff3,
        0x748f82ee, 0x78a5636f, 0x84c87814, 0x8cc70208, 0x90befffa, 0xa4506ceb, 0xbef9a3f7, 0xc67178f2};
       
    int len = 0;
    for(int i = 0; input[i]!='\0'; i++) len++;
    int tlen = ((((len + 8) /64) + 1) * 64) - 8;
    
    uint8_t* by = NULL;
    by = calloc(tlen * 80 +  64, 1);
    
    memcpy(by, input, len);
    by[len] = 0x80;
    
    size_t blen = 8*len;
    for(int i = 0; i != 8; i++)
        by[tlen + 7 - i] = (uint8_t) (blen >> (i * 8) & 0xFF);
    
    uint32_t hat = 0;
    for(int z = 0; z < tlen; z+=(1024/8)){
        uint32_t W[64];
        memset(W, 0, 64 * sizeof (uint32_t));

        for (int i = 0; i < 16; i++)
		    W[i] = (by[i * 4] << 24) | (by[i * 4 + 1] << 16) | (by[i * 4 + 2] << 8) | (by[i * 4 + 3]);
		
        for(int i = 16; i != 64; i++){
            uint32_t s0 = i_rr(W[i - 15], 7) ^ i_rr(W[i - 15], 18) ^ (W[i - 15] >> 3);
            uint32_t s1 = i_rr(W[i - 2], 17) ^ i_rr(W[i - 2], 19) ^ (W[i - 2] >> 10);
            W[i] = W[i - 16] + s0 + W[i - 7] + s1;
        }
        
        uint32_t a = h0;
        uint32_t b = h1;
        uint32_t c = h2;
        uint32_t d = h3;
        uint32_t e = h4;
        uint32_t f = h5;
        uint32_t g = h6;
        uint32_t h = h7;
        
        for(int i = 0; i != 64; i++){
            uint32_t S1 = i_rr(e, 6) ^ i_rr(e, 11) ^ i_rr(e, 25);
            uint32_t ch = (e & f) ^ ((~e) & g);
            uint32_t temp1 = h + S1 + ch + k[i] + W[i];
            
            uint32_t S0 = i_rr(a, 2) ^ i_rr(a, 13) ^ i_rr(a, 22);
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
        
        h0 += a;
        h1 += b;
        h2 += c;
        h3 += d;
        h4 += e;
        h5 += f;
        h6 += g;
        h7 += h;
        
    }
    
    if(v == sha256)
        sprintf(out_stream,"%x%x%x%x%x%x%x%x",h0,h1,h2,h3,h4,h5,h6,h7);
    else if(v == sha224)
        sprintf(out_stream,"%x%x%x%x%x%x%x",h0,h1,h2,h3,h4,h5,h6);
    return;
}

int l_sha256(lua_State* L){
  
  int len = 0;
  char* a = (char*)luaL_checklstring(L, 1, NULL);
  
  char digest[256];

  i_sha2xx(sha256, digest, a);
  lua_pushstring(L, digest);

  return 1;
};

int l_sha224(lua_State* L){
  
  int len = 0;
  char* a = (char*)luaL_checklstring(L, 1, NULL);
  
  char digest[224];

  i_sha2xx(sha224, digest, a);
  lua_pushstring(L, digest);

  return 1;
};
