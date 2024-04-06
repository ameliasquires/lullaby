#include "../crypto.h"
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

static const uint32_t K[] = {0xd76aa478, 0xe8c7b756, 0x242070db, 0xc1bdceee, 0xf57c0faf,
        0x4787c62a, 0xa8304613, 0xfd469501, 0x698098d8, 0x8b44f7af, 0xffff5bb1, 0x895cd7be,
        0x6b901122, 0xfd987193, 0xa679438e, 0x49b40821, 0xf61e2562, 0xc040b340, 0x265e5a51,
        0xe9b6c7aa, 0xd62f105d, 0x02441453, 0xd8a1e681, 0xe7d3fbc8, 0x21e1cde6, 0xc33707d6,
        0xf4d50d87, 0x455a14ed, 0xa9e3e905, 0xfcefa3f8, 0x676f02d9, 0x8d2a4c8a, 0xfffa3942,
        0x8771f681, 0x6d9d6122, 0xfde5380c, 0xa4beea44, 0x4bdecfa9, 0xf6bb4b60, 0xbebfbc70,
        0x289b7ec6, 0xeaa127fa, 0xd4ef3085, 0x04881d05, 0xd9d4d039, 0xe6db99e5, 0x1fa27cf8,
        0xc4ac5665, 0xf4292244, 0x432aff97, 0xab9423a7, 0xfc93a039, 0x655b59c3, 0x8f0ccc92, 
        0xffeff47d, 0x85845dd1, 0x6fa87e4f, 0xfe2ce6e0, 0xa3014314, 0x4e0811a1,
        0xf7537e82, 0xbd3af235, 0x2ad7d2bb, 0xeb86d391};

static const uint32_t s[] = {7, 12, 17, 22, 7, 12, 17, 22, 7, 12, 17, 22, 7, 12, 17, 22,
            5, 9, 14, 20, 5, 9, 14, 20, 5, 9, 14, 20, 5, 9, 14, 20,
            4, 11, 16, 23, 4, 11, 16, 23, 4, 11, 16, 23, 4, 11, 16, 23,
            6, 10, 15, 21, 6, 10, 15, 21, 6, 10, 15, 21, 6, 10, 15, 21};

#define bs 64

struct md5_hash md5_init(){
  struct md5_hash a = {.a0 = 0x67452301, .b0 = 0xefcdab89, .c0 = 0x98badcfe, .d0 = 0x10325476, .total = 0, .bufflen = 0};
  a.buffer = calloc(sizeof * a.buffer, bs);
  return a;
}


struct md5_hash md5_init_l(lua_State* L){
  struct md5_hash a = {.a0 = 0x67452301, .b0 = 0xefcdab89, .c0 = 0x98badcfe, .d0 = 0x10325476, .total = 0, .bufflen = 0};
  a.buffer = lua_newuserdata(L, sizeof * a.buffer * bs);
  memset(a.buffer, 0, bs);
  return a;
}

void md5_round(struct md5_hash* hash){
  uint32_t* M = (uint32_t *)(hash->buffer); 

  uint32_t A = hash->a0;
  uint32_t B = hash->b0;
  uint32_t C = hash->c0;
  uint32_t D = hash->d0;

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
    B = B + rotl32(F, s[i]);
    A = temp;
  }

  hash->a0 += A;
  hash->b0 += B;
  hash->c0 += C;
  hash->d0 += D;
}

void md5_update(uint8_t* input, size_t len, struct md5_hash* hash){
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
    read += bs;
    hash->bufflen = 0;
    md5_round(hash);
  }

  memset(hash->buffer, 0, bs);
  if(total_add != 0){
    memcpy(hash->buffer, input + read, total_add);
    hash->bufflen = total_add;
  }
}

void md5_final(struct md5_hash* hash, char out_stream[64]){
  uint8_t old[bs];
  struct md5_hash old_hash;
  memcpy(&old_hash, hash, sizeof * hash);
  memcpy(old, hash->buffer, bs);

  hash->buffer[hash->bufflen] = 0x80;

  if(hash->bufflen > 55) {
    //too large, needs another buffer
    memset(hash->buffer + hash->bufflen + 1, 0, 64 - hash->bufflen);
    md5_round(hash);
    memset(hash->buffer, 0, 64);
  }

  uint32_t lhhh = 8*hash->total;
  memcpy(hash->buffer + 56, &lhhh, sizeof(lhhh));
  md5_round(hash);

  sprintf(out_stream,"%2.2x%2.2x%2.2x%2.2x%2.2x%2.2x%2.2x%2.2x%2.2x%2.2x%2.2x%2.2x%2.2x%2.2x%2.2x%2.2x", 
      ((uint8_t*)&hash->a0)[0], ((uint8_t*)&hash->a0)[1], ((uint8_t*)&hash->a0)[2], ((uint8_t*)&hash->a0)[3],
      ((uint8_t*)&hash->b0)[0], ((uint8_t*)&hash->b0)[1], ((uint8_t*)&hash->b0)[2], ((uint8_t*)&hash->b0)[3],
      ((uint8_t*)&hash->c0)[0], ((uint8_t*)&hash->c0)[1], ((uint8_t*)&hash->c0)[2], ((uint8_t*)&hash->c0)[3],
      ((uint8_t*)&hash->d0)[0], ((uint8_t*)&hash->d0)[1], ((uint8_t*)&hash->d0)[2], ((uint8_t*)&hash->d0)[3]);

  memcpy(hash->buffer, old, bs);
  memcpy(hash, &old_hash, sizeof * hash);

}

lua_common_hash_init_ni(md5, md5, md5_init_l(L));
lua_common_hash_update(md5, md5);
//common_hash_init_update(md5);

int l_md5_final(lua_State* L){
  struct md5_hash* a = (struct md5_hash*)lua_touserdata(L, 1);

  char digest[128] = {0};
  md5_final(a, digest);
  lua_pushstring(L, digest);
  return 1;
}

void md5(uint8_t* input, size_t len, char out_stream[64]){
  struct md5_hash aa = md5_init();
  md5_update(input, len, &aa);
  md5_final(&aa, out_stream);
}

int l_md5(lua_State* L){
  if(lua_gettop(L) == 0) return l_md5_init(L);
  size_t len = 0;
  uint8_t* a = (uint8_t*)luaL_checklstring(L, 1, &len);

  char digest[128] = {0};
  md5(a, len, digest);
  lua_pushstring(L, digest);

  return 1;
};
