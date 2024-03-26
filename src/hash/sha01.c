#include "../crypto.h"
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

void i_sha01(uint8_t version, char* out_stream, int len, const char* input){
    if(!out_stream||version > 2) return;
    uint32_t h0 = 0x67452301;
    uint32_t h1 = 0xEFCDAB89;
    uint32_t h2 = 0x98BADCFE;
    uint32_t h3 = 0x10325476;
    uint32_t h4 = 0xC3D2E1F0;
     
    int tlen = ((((len + 8) /64) + 1) * 64) - 8;
    
    uint8_t* by = NULL;
    by = calloc(tlen * 80 +  64, 1);
    
    memcpy(by, input, len);
    by[len] = 0x80;
    
    size_t blen = 8*len;
    for(int i = 0; i != 8; i++)
        by[tlen + 7 - i] = (uint8_t) (blen >> (i * 8) & 0xFF);
        
    uint32_t hat = 0;
    for(int z = 0; z < tlen; z+=(512/8)){
        uint32_t W[80];
        memset(W, 0, 80 * sizeof (uint32_t));
        
        for(int i = 0; i != 16; i++){
            int t = 24;
            for(int x = 0;t>=0; x++){
                W[i] += (((uint32_t)by[hat]) << t);
                hat++;
                t-=8;
            }
        }
        for(int i = 16; i != 80; i++)
            W[i] = rotl32(W[i - 3] ^ W[i - 8] ^ W[i - 14] ^ W[i - 16], version);
            
        
        uint32_t a = h0;
        uint32_t b = h1;
        uint32_t c = h2;
        uint32_t d = h3;
        uint32_t e = h4;
        
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
        
        h0 += a;
        h1 += b;
        h2 += c;
        h3 += d;
        h4 += e;
        
        
    }
    sprintf(out_stream,"%02x%02x%02x%02x%02x",h0,h1,h2,h3,h4);
    free(by);
}

int l_sha1(lua_State* L){
  
  size_t len = 0;
  char* a = (char*)luaL_checklstring(L, 1, &len);
  
  char digest[160];

  i_sha01(1, digest, len, a);
  lua_pushstring(L, digest);

  return 1;
};

int l_sha0(lua_State* L){
  
  size_t len = 0;
  char* a = (char*)luaL_checklstring(L, 1, &len);
  
  char digest[160];

  i_sha01(0, digest, len, a);
  lua_pushstring(L, digest);

  return 1;
};
