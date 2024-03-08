#include <stdio.h>
#include <stdint.h>
#include "cityhash.h"

uint32_t rot32(uint32_t val, int shift) {
    return ((val >> shift) | (val << (32 - shift)));
}

uint32_t fmix(uint32_t h){
    h ^= h >> 16;
    h *= 0x85ebca6b;
    h ^= h >> 13;
    h *= 0xc2b2ae35;
    h ^= h >> 16;
    return h;
}

uint32_t mur(uint32_t a, uint32_t h) {
    a *= c1;
    a = rot32(a, 17);
    a *= c2;
    h ^= a;
    h = rot32(h, 19);
    return h * 5 + 0xe6546b64;
}

uint32_t hash32len0to4(uint8_t* in, size_t len){
    uint32_t b = 0, c = 9;
    for(int i = 0; i != len; i++){
        b = b * c1 + (uint32_t)in[i];
        c ^= b;
    }
    return fmix(mur(b, mur((uint32_t)len, c)));
}

uint32_t UNALIGNED_LOAD32(uint8_t *p) {
    return *(uint32_t*)p;
    /* original google code:p
    uint32_t result;
    memcpy(&result, p, sizeof(result));
    return result;
    */
}

uint32_t hash32len5to12(uint8_t* in, size_t len){
    uint32_t a = (uint32_t)(len), b = a * 5, c = 9, d = b;
    a += UNALIGNED_LOAD32(in);
    b += UNALIGNED_LOAD32(in + len - 4);
    c += UNALIGNED_LOAD32(in + ((len >> 1) & 4));
    return fmix(mur(c, mur(b, mur(a, d))));
}

uint32_t hash32len13to24(uint8_t* in, size_t len){
    uint32_t a = UNALIGNED_LOAD32(in - 4 + (len >> 1));
    uint32_t b = UNALIGNED_LOAD32(in + 4);
    uint32_t c = UNALIGNED_LOAD32(in + len - 8);
    uint32_t d = UNALIGNED_LOAD32(in + (len >> 1));
    uint32_t e = UNALIGNED_LOAD32(in);
    uint32_t f = UNALIGNED_LOAD32(in + len - 4);
    uint32_t h = (uint32_t)len;

    return fmix(mur(f, mur(e, mur(d, mur(c, mur(b, mur(a, h)))))));
}

uint32_t cityhash32(uint8_t* in, size_t len){
    if(len <= 24){
        if(len <= 12){
            if(len <= 4) return hash32len0to4(in, len);
            else return hash32len5to12(in, len);
        }else return hash32len13to24(in, len);
    }
    
    uint32_t h = (uint32_t)len, g = c1 * h, f = g;
    uint32_t a0 = rot32(UNALIGNED_LOAD32(in + len - 4) * c1, 17) * c2;
    uint32_t a1 = rot32(UNALIGNED_LOAD32(in + len - 8) * c1, 17) * c2;
    uint32_t a2 = rot32(UNALIGNED_LOAD32(in + len - 16) * c1, 17) * c2;
    uint32_t a3 = rot32(UNALIGNED_LOAD32(in + len - 12) * c1, 17) * c2;
    uint32_t a4 = rot32(UNALIGNED_LOAD32(in + len - 20) * c1, 17) * c2;
    h ^= a0;
    h = rot32(h, 19);
    h = h * 5 + 0xe6546b64;
    h ^= a2;
    h = rot32(h, 19);
    h = h * 5 + 0xe6546b64;
    g ^= a1;
    g = rot32(g, 19);
    g = g * 5 + 0xe6546b64;
    g ^= a3;
    g = rot32(g, 19);
    g = g * 5 + 0xe6546b64;
    f += a4;
    f = rot32(f, 19);
    f = f * 5 + 0xe6546b64;

    for(int i = (len - 1)/20; i != 0; i--){
        uint32_t a0 = rot32(UNALIGNED_LOAD32(in) * c1, 17) * c2;
        uint32_t a1 = UNALIGNED_LOAD32(in + 4);
        uint32_t a2 = rot32(UNALIGNED_LOAD32(in + 8) * c1, 17) * c2;
        uint32_t a3 = rot32(UNALIGNED_LOAD32(in + 12) * c1, 17) * c2;
        uint32_t a4 = UNALIGNED_LOAD32(in + 16);
        h ^= a0;
        h = rot32(h, 18);
        h = h * 5 + 0xe6546b64;
        f += a1;
        f = rot32(f, 19);
        f = f * c1;
        g += a2;
        g = rot32(g, 18);
        g = g * 5 + 0xe6546b64;
        h ^= a3 + a1;
        h = rot32(h, 19);
        h = h * 5 + 0xe6546b64;
        g ^= a4;
        g = __builtin_bswap32(g) * 5;
        h += a4 * 5;
        h = __builtin_bswap32(h);
        f += a0;
        //PERMUTE3(f, h, g);
        uint32_t temp = f;
        f = h; h = temp;
        temp = f;
        f = g; g = temp;
        //
        in += 20;
    }

    g = rot32(g, 11) * c1;
    g = rot32(g, 17) * c1;
    f = rot32(f, 11) * c1;
    f = rot32(f, 17) * c1;
    h = rot32(h + g, 19);
    h = h * 5 + 0xe6546b64;
    h = rot32(h, 17) * c1;
    h = rot32(h + f, 19);
    h = h * 5 + 0xe6546b64;
    h = rot32(h, 17) * c1;

    return h;
}

//64 version

uint64_t UNALIGNED_LOAD64(uint8_t *p) {
    return *(uint64_t*)p;
    /*
    uint64_t result;
    memcpy(&result, p, sizeof(result));
    return result;
    //*/
}

uint64_t rot64(uint64_t val, int shift) {
    return ((val >> shift) | (val << (64 - shift)));
}

uint64_t hashlen16(uint64_t u, uint64_t v, uint64_t mul) {
    uint64_t a = (u ^ v) * mul;
    a ^= (a >> 47);
    uint64_t b = (v ^ a) * mul;
    b ^= (b >> 47);
    b *= mul;
    return b;
}

uint64_t shiftmix(uint64_t val) {
  return val ^ (val >> 47);
}

uint64_t hashlen0to16(uint8_t* in, size_t len){
    if(len >= 8){
        uint64_t mul = k2 + len * 2;
        uint64_t a = UNALIGNED_LOAD64(in) + k2;
        uint64_t b = UNALIGNED_LOAD64(in + len - 8);
        uint64_t c = rot64(b, 37) * mul + a;
        uint64_t d = (rot64(a, 25) + b) * mul;
        return hashlen16(c, d, mul);
    } 
    if(len >= 4){
        uint64_t mul = k2 + len * 2;
        uint64_t a = UNALIGNED_LOAD32(in);
        return hashlen16(len + (a << 3), UNALIGNED_LOAD32(in + len - 4), mul);
    }
    if(len > 0){
        uint8_t a = (uint8_t)in[0];
        uint8_t b = (uint8_t)in[len >> 1];
        uint8_t c = (uint8_t)in[len - 1];
        uint32_t y = ((uint32_t)a) + (((uint32_t)b) << 8);
        uint32_t z = ((uint32_t)len) + ((uint32_t)(c) << 2);
        return shiftmix(y * k2 ^ z * k0) * k2;
    }
    return k2;
}
uint64_t hashlen17to32(uint8_t* in, size_t len){
    uint64_t mul = k2 + len * 2;
    uint64_t a = UNALIGNED_LOAD64(in) * k1;
    uint64_t b = UNALIGNED_LOAD64(in + 8);
    uint64_t c = UNALIGNED_LOAD64(in + len - 8) * mul;
    uint64_t d = UNALIGNED_LOAD64(in + len - 16) * k2;
    return hashlen16(rot64(a + b, 43) + rot64(c, 30) + d,
                    a + rot64(b + k2, 18) + c, mul);
}

uint64_t hashlen33to64(uint8_t* in, size_t len){
    uint64_t mul = k2 + len * 2;
    uint64_t a = UNALIGNED_LOAD64(in) * k2;
    uint64_t b = UNALIGNED_LOAD64(in + 8);
    uint64_t c = UNALIGNED_LOAD64(in + len - 24);
    uint64_t d = UNALIGNED_LOAD64(in + len - 32);
    uint64_t e = UNALIGNED_LOAD64(in + 16) * k2;
    uint64_t f = UNALIGNED_LOAD64(in + 24) * 9;
    uint64_t g = UNALIGNED_LOAD64(in + len - 8);
    uint64_t h = UNALIGNED_LOAD64(in + len - 16) * mul;
    uint64_t u = rot64(a + g, 43) + (rot64(b, 30) + c) * 9;
    uint64_t v = ((a + g) ^ d) + f + 1;
    uint64_t w = __builtin_bswap64((u + v) * mul) + h;
    uint64_t x = rot64(e + f, 42) + c;
    uint64_t y = (__builtin_bswap64((v + w) * mul) + g) * mul;
    uint64_t z = e + f + c;
    a = __builtin_bswap64((x + z) * mul + y) + b;
    b = shiftmix((z + a) * mul + d + h) * mul;
    return b + x;
}

void WeakHashLen32WithSeeds(uint64_t w, uint64_t x, uint64_t y, uint64_t z, uint64_t a, uint64_t b, uint64_t*p1, int64_t*p2) {
  a += w;
  b = rot64(b + a + z, 21);
  uint64_t c = a;
  a += x;
  a += y;
  b += rot64(a, 44);
  *p1 = a + z;
  *p2 = b + c;
  //return make_pair(a + z, b + c);
}

void pWeakHashLen32WithSeeds(uint8_t* s, uint64_t a, uint64_t b, uint64_t* p1, int64_t* p2) {
  WeakHashLen32WithSeeds(UNALIGNED_LOAD64(s), UNALIGNED_LOAD64(s + 8),
        UNALIGNED_LOAD64(s + 16), UNALIGNED_LOAD64(s + 24), a, b, p1, p2);
}

uint64_t hash128to64(uint64_t f, uint64_t s) {
    uint64_t kMul = 0x9ddfea08eb382d69ULL;
    uint64_t a = (f ^ s) * kMul;
    a ^= (a >> 47);
    uint64_t b = (s ^ a) * kMul;
    b ^= (b >> 47);
    b *= kMul;
    return b;
}

uint64_t HashLen16_2(uint64_t u, uint64_t v) {
  return hash128to64(u, v);
}

uint64_t cityhash64(uint8_t* in, size_t len){
    if(len <= 32){
        if(len <= 16) return hashlen0to16(in, len);
        else return hashlen17to32(in, len);
    } else if(len <= 64) return hashlen33to64(in, len);
    
    uint64_t x = UNALIGNED_LOAD64(in + len - 40);
    uint64_t y = UNALIGNED_LOAD64(in + len - 16) + UNALIGNED_LOAD64(in + len - 56);
    uint64_t z = HashLen16_2(UNALIGNED_LOAD64(in + len - 48) + len, UNALIGNED_LOAD64(in + len - 24));
    uint64_t v1, w1;
    int64_t v2, w2;
    pWeakHashLen32WithSeeds(in + len - 64, len, z, &v1, &v2);
    pWeakHashLen32WithSeeds(in + len - 32, y + k1, x, &w1, &w2);
    x = x * k1 + UNALIGNED_LOAD64(in);
    
    for(int i = (len - 1) /64; i != 0; i--){
        x = rot64(x + y + v1 + UNALIGNED_LOAD64(in + 8), 37) * k1;
        y = rot64(y + v2 + UNALIGNED_LOAD64(in + 48), 42) * k1;
        x ^= w2;
        y += v1 + UNALIGNED_LOAD64(in + 40);
        z = rot64(z + w1, 33) * k1;
        pWeakHashLen32WithSeeds(in, v2 * k1, x + w1, &v1, &v2);
        pWeakHashLen32WithSeeds(in + 32, z + w2, y + UNALIGNED_LOAD64(in + 16), &w1, &w2);
        //std::swap(z, x);
        uint64_t temp = z;
        z = x;
        x = temp;
        in += 64;
    }
    //printf("%llu %llu %llu\n",x,y,z);
    return HashLen16_2(HashLen16_2(v1, w1) + shiftmix(y) * k1 + z,
                   HashLen16_2(v2, w2) + x);
}

void citymurmur(uint8_t* in, size_t len, uint64_t f, uint64_t s, uint64_t* o1, uint64_t* o2){
    uint64_t a = f;
    uint64_t b = s;
    uint64_t c = 0;
    uint64_t d = 0;

    if (len <= 16) {
        a = shiftmix(a * k1) * k1;
        c = b * k1 + hashlen0to16(in, len);
        d = shiftmix(a + (len >= 8 ? UNALIGNED_LOAD64(in) : c));
    } else {
        c = HashLen16_2(UNALIGNED_LOAD64(in + len - 8) + k1, a);
        d = HashLen16_2(b + len, c + UNALIGNED_LOAD64(in + len - 16));
        a += d;

        for(; len > 16; len -=16) {
        a ^= shiftmix(UNALIGNED_LOAD64(in) * k1) * k1;
        a *= k1;
        b ^= a;
        c ^= shiftmix(UNALIGNED_LOAD64(in + 8) * k1) * k1;
        c *= k1;
        d ^= c;
        s += 16;
        len -= 16;
        }
    }
    a = HashLen16_2(a, c);
    b = HashLen16_2(d, b);
    //return uint128(a ^ b, HashLen16(b, a));
    *o2 = a ^ b;
    *o1 = HashLen16_2(b, a);
}

void cityhash128withseed(uint8_t* in, size_t len, uint64_t f, uint64_t s, uint64_t* o1, uint64_t* o2){
    if(len < 128){
        citymurmur(in, len, k0, k1, o2, o1);
        return;
    }

    uint64_t v1, w1;
    int64_t v2, w2;
    uint64_t x = f;
    uint64_t y = s;
    uint64_t z = len * k1;

    v1 = rot64(y ^ k1, 49) * k1 + UNALIGNED_LOAD64(in);
    v2 = rot64(v1, 42) * k1 + UNALIGNED_LOAD64(in + 8);
    w1 = rot64(y + z, 35) * k1 + x;
    w2 = rot64(x + UNALIGNED_LOAD64(in + 88), 53) * k1;

    for(; len >= 128; len-=128){
        x = rot64(x + y + v1 + UNALIGNED_LOAD64(in + 8), 37) * k1;
        y = rot64(y + v2 + UNALIGNED_LOAD64(in + 48), 42) * k1;
        x ^= w2;
        y += v1 + UNALIGNED_LOAD64(in + 40);
        z = rot64(z + w1, 33) * k1;
        pWeakHashLen32WithSeeds(in, v2 * k1, x + w1, &v1, &v2);
        pWeakHashLen32WithSeeds(in + 32, z + w2, y + UNALIGNED_LOAD64(in + 16), &w1, &w2);
        uint64_t temp = z;
        z = x;
        x = temp;
        in += 64;
        //
        x = rot64(x + y + v1 + UNALIGNED_LOAD64(in + 8), 37) * k1;
        y = rot64(y + v2 + UNALIGNED_LOAD64(in + 48), 42) * k1;
        x ^= w2;
        y += v1 + UNALIGNED_LOAD64(in + 40);
        z = rot64(z + w1, 33) * k1;
        pWeakHashLen32WithSeeds(in, v2 * k1, x + w1, &v1, &v2);
        pWeakHashLen32WithSeeds(in + 32, z + w2, y + UNALIGNED_LOAD64(in + 16), &w1, &w2);
        temp = z;
        z = x;
        x = temp;
        in += 64;
    }
    x += rot64(v1 + z, 49) * k0;
    y = y * k0 + rot64(w2, 37);
    z = z * k0 + rot64(w1, 27);
    w1 *= 9;
    v1 *= k0;

    for (size_t tail_done = 0; tail_done < len; ) {
        tail_done += 32;
        y = rot64(x + y, 42) * k0 + v2;
        w1 += UNALIGNED_LOAD64(in + len - tail_done + 16);
        x = x * k0 + w1;
        z += w2 + UNALIGNED_LOAD64(in + len - tail_done);
        w2 += v1;
        pWeakHashLen32WithSeeds(in + len - tail_done, v1 + z, v2, &v1, &v2);
        v1 *= k0;
    }

    x = HashLen16_2(x, v1);
    y = HashLen16_2(y + z, w1);
    //return uint128(HashLen16(x + v.second, w.second) + y,
    //             HashLen16(x + w.second, y + v.second));
    *o1 = HashLen16_2(x + v2, w2) + y;
    *o2 = HashLen16_2(x + w2, y + v2);

}

void cityhash128(uint8_t* in, size_t len, uint64_t* f, uint64_t* s){
    if(len >= 16) return cityhash128withseed(in + 16, len - 16,UNALIGNED_LOAD64(in), UNALIGNED_LOAD64(in + 8) + k0, f, s);
    return cityhash128withseed(in, len, k0, k1, f, s);
}

int l_cityhash32(lua_State* L){ 
  size_t len = 0;
  uint8_t* a = (uint8_t*)luaL_checklstring(L, 1, &len);
  
  char digest[32];

  uint32_t u = cityhash32(a, len);
  sprintf(digest,"%08x",u);
  lua_pushstring(L, digest);
  return 1;
}

int l_cityhash64(lua_State* L){ 
  size_t len = 0;
  uint8_t* a = (uint8_t*)luaL_checklstring(L, 1, &len);
  
  char digest[64];

  uint64_t u = cityhash64(a, len);
  sprintf(digest,"%016llx",u);
  lua_pushstring(L, digest);
  return 1;
}

int l_cityhash128(lua_State* L){ 
  size_t len = 0;
  uint8_t* a = (uint8_t*)luaL_checklstring(L, 1, &len);
  
  char digest[128];

  uint64_t u1, u2;
  cityhash128(a, len, &u1, &u2);
  sprintf(digest,"%08llx%08llx",u1, u2);
  lua_pushstring(L, digest);
  return 1;
}
