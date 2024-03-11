#include "../util.h"
#include "../crypto.h"
#include <stdint.h>
#include <string.h>

static const int sc_numVars = 12;
static const uint64_t sc_const = 0xdeadbeefdeadbeefLL;
static const size_t sc_blockSize = sc_numVars*8;
static const size_t sc_bufSize = 2*sc_blockSize;

uint64_t i_rot64(uint64_t x, int k){
        return (x << k) | (x >> (64 - k));
}

void short_mix(uint64_t* h0, uint64_t* h1, uint64_t* h2, uint64_t* h3){
        *h2 = i_rot64(*h2,50);  *h2 += *h3;  *h0 ^= *h2;
        *h3 = i_rot64(*h3,52);  *h3 += *h0;  *h1 ^= *h3;
        *h0 = i_rot64(*h0,30);  *h0 += *h1;  *h2 ^= *h0;
        *h1 = i_rot64(*h1,41);  *h1 += *h2;  *h3 ^= *h1;
        *h2 = i_rot64(*h2,54);  *h2 += *h3;  *h0 ^= *h2;
        *h3 = i_rot64(*h3,48);  *h3 += *h0;  *h1 ^= *h3;
        *h0 = i_rot64(*h0,38);  *h0 += *h1;  *h2 ^= *h0;
        *h1 = i_rot64(*h1,37);  *h1 += *h2;  *h3 ^= *h1;
        *h2 = i_rot64(*h2,62);  *h2 += *h3;  *h0 ^= *h2;
        *h3 = i_rot64(*h3,34);  *h3 += *h0;  *h1 ^= *h3;
        *h0 = i_rot64(*h0,5);   *h0 += *h1;  *h2 ^= *h0;
        *h1 = i_rot64(*h1,36);  *h1 += *h2;  *h3 ^= *h1;
}

void short_end(uint64_t* h0, uint64_t* h1, uint64_t* h2, uint64_t* h3){
        *h3 ^= *h2;  *h2 = i_rot64(*h2,15);  *h3 += *h2;
        *h0 ^= *h3;  *h3 = i_rot64(*h3,52);  *h0 += *h3;
        *h1 ^= *h0;  *h0 = i_rot64(*h0,26);  *h1 += *h0;
        *h2 ^= *h1;  *h1 = i_rot64(*h1,51);  *h2 += *h1;
        *h3 ^= *h2;  *h2 = i_rot64(*h2,28);  *h3 += *h2;
        *h0 ^= *h3;  *h3 = i_rot64(*h3,9);   *h0 += *h3;
        *h1 ^= *h0;  *h0 = i_rot64(*h0,47);  *h1 += *h0;
        *h2 ^= *h1;  *h1 = i_rot64(*h1,54);  *h2 += *h1;
        *h3 ^= *h2;  *h2 = i_rot64(*h2,32);  *h3 += *h2;
        *h0 ^= *h3;  *h3 = i_rot64(*h3,25);  *h0 += *h3;
        *h1 ^= *h0;  *h0 = i_rot64(*h0,63);  *h1 += *h0;
}
void spooky_short(uint8_t* in, size_t len, uint64_t* hash1, uint64_t* hash2, enum spooky_version v){
    uint64_t buffer[2*sc_numVars];
    union {
        const uint8_t *p8; 
        uint32_t *p32;
        uint64_t *p64; 
        size_t i; 
    } u;

    u.p8 = (const uint8_t*)in;

    size_t remainder = len%32;
    uint64_t a=*hash1;
    uint64_t b=*hash2;
    uint64_t c=sc_const;
    uint64_t d=sc_const;

    if(len > 12){
        const uint64_t *end = u.p64 + (len/32)*4;

        for(; u.p64 < end; u.p64 += 4){
            c += u.p64[0];
            d += u.p64[1];
            short_mix(&a,&b,&c,&d);
            a += u.p64[2];
            b += u.p64[3];
        }

        if(remainder >= 16){
            c += u.p64[0];
            d += u.p64[1];
            short_mix(&a,&b,&c,&d);
            u.p64 += 2;
            remainder -= 16;
        }
    }

    d = (((uint64_t)len) << 56) + (d * (v == spv2));
    switch(remainder){
        case 15:
            d += ((uint64_t)u.p8[14]) << 48;
        case 14:
            d += ((uint64_t)u.p8[13]) << 40;
        case 13:
            d += ((uint64_t)u.p8[12]) << 32;
        case 12:
            d += u.p32[2];
            c += u.p64[0];
            break;
        case 11:
            d += ((uint64_t)u.p8[10]) << 16;
        case 10:
            d += ((uint64_t)u.p8[9]) << 8;
        case 9:
            d += (uint64_t)u.p8[8];
        case 8:
            c += u.p64[0];
            break;
        case 7:
            c += ((uint64_t)u.p8[6]) << 48;
        case 6:
            c += ((uint64_t)u.p8[5]) << 40;
        case 5:
            c += ((uint64_t)u.p8[4]) << 32;
        case 4:
            c += u.p32[0];
            break;
        case 3:
            c += ((uint64_t)u.p8[2]) << 16;
        case 2:
            c += ((uint64_t)u.p8[1]) << 8;
        case 1:
            c += (uint64_t)u.p8[0];
            break;
        case 0:
            c += sc_const;
            d += sc_const;
    }
    short_end(&a,&b,&c,&d);
    *hash1 = a;
    *hash2 = b;
}

void mix(const uint64_t *data, 
        uint64_t* s0, uint64_t* s1, uint64_t* s2, uint64_t* s3,
        uint64_t* s4, uint64_t* s5, uint64_t* s6, uint64_t* s7,
        uint64_t* s8, uint64_t* s9, uint64_t* s10,uint64_t* s11){
      *s0 += data[0];    *s2 ^= *s10;    *s11 ^= *s0;    *s0 = i_rot64(*s0,11);    *s11 += *s1;
      *s1 += data[1];    *s3 ^= *s11;    *s0 ^= *s1;    *s1 = i_rot64(*s1,32);    *s0 += *s2;
      *s2 += data[2];    *s4 ^= *s0;    *s1 ^= *s2;    *s2 = i_rot64(*s2,43);    *s1 += *s3;
      *s3 += data[3];    *s5 ^= *s1;    *s2 ^= *s3;    *s3 = i_rot64(*s3,31);    *s2 += *s4;
      *s4 += data[4];    *s6 ^= *s2;    *s3 ^= *s4;    *s4 = i_rot64(*s4,17);    *s3 += *s5;
      *s5 += data[5];    *s7 ^= *s3;    *s4 ^= *s5;    *s5 = i_rot64(*s5,28);    *s4 += *s6;
      *s6 += data[6];    *s8 ^= *s4;    *s5 ^= *s6;    *s6 = i_rot64(*s6,39);    *s5 += *s7;
      *s7 += data[7];    *s9 ^= *s5;    *s6 ^= *s7;    *s7 = i_rot64(*s7,57);    *s6 += *s8;
      *s8 += data[8];    *s10 ^= *s6;    *s7 ^= *s8;    *s8 = i_rot64(*s8,55);    *s7 += *s9;
      *s9 += data[9];    *s11 ^= *s7;    *s8 ^= *s9;    *s9 = i_rot64(*s9,54);    *s8 += *s10;
      *s10 += data[10];    *s0 ^= *s8;    *s9 ^= *s10;    *s10 = i_rot64(*s10,22);    *s9 += *s11;
      *s11 += data[11];    *s1 ^= *s9;    *s10 ^= *s11;    *s11 = i_rot64(*s11,46);    *s10 += *s0;
}

void end_partial(
        uint64_t* h0, uint64_t* h1, uint64_t* h2, uint64_t* h3,
        uint64_t* h4, uint64_t* h5, uint64_t* h6, uint64_t* h7, 
        uint64_t* h8, uint64_t* h9, uint64_t* h10,uint64_t* h11)
    {
        *h11+= *h1;    *h2 ^= *h11;   *h1 = i_rot64(*h1,44);
        *h0 += *h2;    *h3 ^= *h0;    *h2 = i_rot64(*h2,15);
        *h1 += *h3;    *h4 ^= *h1;    *h3 = i_rot64(*h3,34);
        *h2 += *h4;    *h5 ^= *h2;    *h4 = i_rot64(*h4,21);
        *h3 += *h5;    *h6 ^= *h3;    *h5 = i_rot64(*h5,38);
        *h4 += *h6;    *h7 ^= *h4;    *h6 = i_rot64(*h6,33);
        *h5 += *h7;    *h8 ^= *h5;    *h7 = i_rot64(*h7,10);
        *h6 += *h8;    *h9 ^= *h6;    *h8 = i_rot64(*h8,13);
        *h7 += *h9;    *h10^= *h7;    *h9 = i_rot64(*h9,38);
        *h8 += *h10;   *h11^= *h8;    *h10= i_rot64(*h10,53);
        *h9 += *h11;   *h0 ^= *h9;    *h11= i_rot64(*h11,42);
        *h10+= *h0;    *h1 ^= *h10;   *h0 = i_rot64(*h0,54);
}

void end_f(uint64_t* h0, uint64_t* h1, uint64_t* h2, uint64_t* h3,
        uint64_t* h4, uint64_t* h5, uint64_t* h6, uint64_t* h7, 
        uint64_t* h8, uint64_t* h9, uint64_t* h10,uint64_t* h11){
        end_partial(h0,h1,h2,h3,h4,h5,h6,h7,h8,h9,h10,h11);
        end_partial(h0,h1,h2,h3,h4,h5,h6,h7,h8,h9,h10,h11);
        end_partial(h0,h1,h2,h3,h4,h5,h6,h7,h8,h9,h10,h11);
}

#define allow_unali 1
void spookyhash128(uint8_t* in, size_t len, uint64_t* hash1, uint64_t* hash2, enum spooky_version v){
    if(len < sc_bufSize){
        spooky_short(in, len, hash1, hash2,v);
        return;
    }

    uint64_t h0,h1,h2,h3,h4,h5,h6,h7,h8,h9,h10,h11;
    uint64_t buf[sc_numVars];
    uint64_t *end;
    union 
    { 
        const uint8_t *p8; 
        uint64_t *p64; 
        size_t i; 
    } u;
    size_t remainder;

    h0=h3=h6=h9  = *hash1;
    h1=h4=h7=h10 = *hash2;
    h2=h5=h8=h11 = sc_const;

    u.p8 = (const uint8_t *)in;
    end = u.p64 + (len/sc_blockSize)*sc_numVars;

    if(allow_unali || ((u.i & 0x7) == 0)){
        for(; u.p64 < end;){
            mix(u.p64, &h0,&h1,&h2,&h3,&h4,&h5,&h6,&h7,&h8,&h9,&h10,&h11);
	        u.p64 += sc_numVars;
        }
    } else {
        //do this

    }

    remainder = (len - ((const uint8_t*)end - (const uint8_t*)in));
    memcpy(buf, end, remainder);
    memset(((uint8_t*)buf)+remainder, 0, sc_blockSize - remainder);
    ((uint8_t*)buf)[sc_blockSize - 1] = remainder;

    mix(buf, &h0,&h1,&h2,&h3,&h4,&h5,&h6,&h7,&h8,&h9,&h10,&h11);
    end_f(&h0,&h1,&h2,&h3,&h4,&h5,&h6,&h7,&h8,&h9,&h10,&h11);
    *hash1 = h0;
    *hash2 = h1;

}

uint64_t spookyhash64(uint8_t *message, size_t length, uint64_t seed, enum spooky_version v){
    uint64_t hash1 = seed;
    spookyhash128(message, length, &hash1, &seed, v);
    return hash1;
}
uint32_t spookyhash32(uint8_t *message, size_t length, uint32_t seed, enum spooky_version v){
    uint64_t hash1 = seed, hash2 = seed;
    spookyhash128(message, length, &hash1, &hash2, v);
    return (uint32_t)hash1;
}

int l_spookyhash128_v1(lua_State* L){ 
  size_t len = 0;
  uint8_t* a = (uint8_t*)luaL_checklstring(L, 1, &len);
  int argv = lua_gettop(L);
  uint64_t b = 0, c = 0;

  if(argv > 1){
    b = luaL_checkinteger(L, 2);
    c = luaL_checkinteger(L, 3);
  }
  char digest[128] = {0};
  //uint64_t b = 0;
  //uint64_t c = 0;
  spookyhash128(a, 4, &b, &c, spv1);
  
  sprintf(digest, "%016lx%016lx", b, c);
  lua_pushstring(L, digest);
  return 1;
}

int l_spookyhash128_v2(lua_State* L){ 
  size_t len = 0;
  uint8_t* a = (uint8_t*)luaL_checklstring(L, 1, &len);
  int argv = lua_gettop(L);
  uint64_t b = 0, c = 0;

  if(argv > 1){
    b = luaL_checkinteger(L, 2);
    c = luaL_checkinteger(L, 3);
  }
  char digest[128] = {0};
  //uint64_t b = 0;
  //uint64_t c = 0;
  spookyhash128(a, 4, &b, &c, spv2);
  
  sprintf(digest, "%016lx%016lx", b, c);
  lua_pushstring(L, digest);
  return 1;
}

int l_spookyhash64_v1(lua_State* L){ 
  size_t len = 0;
  uint8_t* a = (uint8_t*)luaL_checklstring(L, 1, &len);
  int argv = lua_gettop(L);
  uint64_t seed = 0;
  if(argv > 1) seed = luaL_checkinteger(L, 2);
  
  char digest[64] = {0};
  
  sprintf(digest, "%08lx", spookyhash64(a, len, seed, spv1));
  lua_pushstring(L, digest);
  return 1;
}

int l_spookyhash64_v2(lua_State* L){ 
  size_t len = 0;
  uint8_t* a = (uint8_t*)luaL_checklstring(L, 1, &len);
  int argv = lua_gettop(L);
  uint64_t seed = 0;
  if(argv > 1) seed = luaL_checkinteger(L, 2);

  char digest[64] = {0};
  
  sprintf(digest, "%08lx", spookyhash64(a, len, seed, spv2));
  lua_pushstring(L, digest);
  return 1;
}

int l_spookyhash32_v1(lua_State* L){ 
  size_t len = 0;
  uint8_t* a = (uint8_t*)luaL_checklstring(L, 1, &len);
  int argv = lua_gettop(L);
  uint32_t seed = 0;
  if(argv > 1) seed = luaL_checkinteger(L, 2);

  char digest[32] = {0};
  
  sprintf(digest, "%04x", spookyhash32(a, len, seed, spv1));
  lua_pushstring(L, digest);
  return 1;
}

int l_spookyhash32_v2(lua_State* L){ 
  size_t len = 0;
  uint8_t* a = (uint8_t*)luaL_checklstring(L, 1, &len);
  int argv = lua_gettop(L);
  uint32_t seed = 0;
  if(argv > 1) seed = luaL_checkinteger(L, 2);

  char digest[32] = {0};
  
  sprintf(digest, "%04x", spookyhash32(a, len, seed, spv2));
  lua_pushstring(L, digest);
  return 1;
}
/*
int __main(){
    uint64_t a = 0;
    uint64_t b = 0;
    spookyhash128("meow",4,&a,&b,v2);
    printf("%llx %llx %x",a,spookyhash64("meow",4,0,v2),spookyhash32("meow",4,0,v2));
    return 0;
}*/
