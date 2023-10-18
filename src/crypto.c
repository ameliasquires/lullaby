#include "crypto.h"
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
static const uint32_t K[] = {0xd76aa478,
            0xe8c7b756, 0x242070db, 0xc1bdceee, 0xf57c0faf, 0x4787c62a, 0xa8304613, 0xfd469501,
            0x698098d8, 0x8b44f7af, 0xffff5bb1, 0x895cd7be, 0x6b901122, 0xfd987193, 0xa679438e,
            0x49b40821, 0xf61e2562, 0xc040b340, 0x265e5a51, 0xe9b6c7aa, 0xd62f105d, 0x2441453,
            0xd8a1e681, 0xe7d3fbc8, 0x21e1cde6, 0xc33707d6, 0xf4d50d87, 0x455a14ed, 0xa9e3e905,
            0xfcefa3f8, 0x676f02d9, 0x8d2a4c8a, 0xfffa3942, 0x8771f681, 0x6d9d6122, 0xfde5380c,
            0xa4beea44, 0x4bdecfa9, 0xf6bb4b60, 0xbebfbc70, 0x289b7ec6, 0xeaa127fa, 0xd4ef3085,
            0x4881d05, 0xd9d4d039, 0xe6db99e5, 0x1fa27cf8, 0xc4ac5665, 0xf4292244, 0x432aff97,
            0xab9423a7, 0xfc93a039, 0x655b59c3, 0x8f0ccc92, 0xffeff47d, 0x85845dd1, 0x6fa87e4f,
            0xfe2ce6e0, 0xa3014314, 0x4e0811a1, 0xf7537e82, 0xbd3af235, 0x2ad7d2bb, 0xeb86d39};
static const uint32_t s[] = {7, 12, 17, 22, 7, 12, 17, 22, 7, 12, 17, 22, 7, 12, 17, 22,
            5, 9, 14, 20, 5, 9, 14, 20, 5, 9, 14, 20, 5, 9, 14, 20,
            4, 11, 16, 23, 4, 11, 16, 23, 4, 11, 16, 23, 4, 11, 16, 23,
            6, 10, 15, 21, 6, 10, 15, 21, 6, 10, 15, 21, 6, 10, 15, 21};

int l_md5(lua_State* L){
  uint32_t a0 = 0x67452301;
  uint32_t b0 = 0xefcdab89;
  uint32_t c0 = 0x98badcfe;
  uint32_t d0 = 0x10325476;

  int len = 0;
  char* ww = "wowa";
  uint8_t* a = (uint8_t*)luaL_checklstring(L, 1, NULL);
  for(int i = 0; a[i]!='\0'; i++) len++;
  
  int tlen = ((((len + 8) /64) + 1) * 64) - 8;

  uint8_t* b = NULL;
  b = calloc(tlen + 64, 1);

  memcpy(b, a, len);
  b[len] = 0x80;

  //add padding
  uint32_t lhhh = 8*len;
  memcpy(b + tlen, &lhhh, 4);

  
  for(int z = 0; z < tlen; z+=(512/8)){
    uint32_t* M = (uint32_t *) (b + z); 

    uint32_t A = a0;
    uint32_t B = b0;
    uint32_t C = c0;
    uint32_t D = d0;

    for(int i = 0; i < 64; i++){
      uint32_t F, g;
      
      if(i < 16){
        F = (B & C) | ((~B) & D);
        g = i; 
      } else if(i < 32){
        F = (D & B) | ((~D) & C);
        g = (5*i + 1) % 16;
      } else if(i < 48){
        F = B ^ C ^ D;
        g = (3*i + 5) % 16;
      } else {
        F = C ^ (B | (~D));
        g = (7*i) % 16;
      }
 


      F = F + A + K[i] + M[g];

      uint32_t temp = D; 
      D = C;
      C = B;
      B = B + (F << s[i] | (F >> (32 - s[i])));
      A = temp;
    }

    a0 += A;
    b0 += B;
    c0 += C;
    d0 += D;

  }
  char ou[64];
  uint8_t *p; 
  p=(uint8_t *)&a0;
  sprintf(ou,"%2.2x%2.2x%2.2x%2.2x", p[0], p[1], p[2], p[3]);
 
  p=(uint8_t *)&b0;
  sprintf(ou,"%s%2.2x%2.2x%2.2x%2.2x", ou, p[0], p[1], p[2], p[3]);
 
  p=(uint8_t *)&c0;
  sprintf(ou,"%s%2.2x%2.2x%2.2x%2.2x", ou, p[0], p[1], p[2], p[3]);
 
  p=(uint8_t *)&d0;
  sprintf(ou,"%s%2.2x%2.2x%2.2x%2.2x", ou, p[0], p[1], p[2], p[3]);

  lua_pushstring(L, ou);

  free(b);

  return 1;
};

