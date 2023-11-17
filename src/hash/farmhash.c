#include "../i_util.h"
#include "../crypto.h" //include city hash too
#include <stdint.h>

uint32_t farmhash32len13to24(uint8_t* in, size_t len) {
    uint32_t seed = 0;
    uint32_t a = UNALIGNED_LOAD32(in - 4 + (len >> 1));
    uint32_t b = UNALIGNED_LOAD32(in + 4);
    uint32_t c = UNALIGNED_LOAD32(in + len - 8);
    uint32_t d = UNALIGNED_LOAD32(in + (len >> 1));
    uint32_t e = UNALIGNED_LOAD32(in);
    uint32_t f = UNALIGNED_LOAD32(in + len - 4);
    uint32_t h = d * c1 + len + seed;
    a = rot32(a, 12) + f;
    h = mur(c, h) + a;
    a = rot32(a, 3) + c;
    h = mur(e, h) + a;
    a = rot32(a + f, 12) + d;
    h = mur(b ^ seed, h) + a;
    return fmix(h);
}

uint32_t farmhash32(uint8_t* in, size_t len){
    if(len <= 24){
        if(len <= 12){
            if(len <= 4) return hash32len0to4(in, len);
            else return hash32len5to12(in, len);
        } else return farmhash32len13to24(in, len);
    }

    uint32_t h = len, g = c1 * len, f = g;
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
    f = rot32(f, 19) + 113;
    size_t iters = (len - 1) / 20;
    for(int i = (len - 1) / 20; i != 0; i--){
        uint32_t a = UNALIGNED_LOAD32(in);
        uint32_t b = UNALIGNED_LOAD32(in + 4);
        uint32_t c = UNALIGNED_LOAD32(in + 8);
        uint32_t d = UNALIGNED_LOAD32(in + 12);
        uint32_t e = UNALIGNED_LOAD32(in + 16);
        h += a;
        g += b;
        f += c;
        h = mur(d, h) + e;
        g = mur(c, g) + a;
        f = mur(b + e * c1, f) + d;
        f += g;
        g += f;
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

uint64_t farmhashlen33to64(uint8_t *s, size_t len) {
  uint64_t mul = k2 + len * 2;
  uint64_t a = UNALIGNED_LOAD64(s) * k2;
  uint64_t b = UNALIGNED_LOAD64(s + 8);
  uint64_t c = UNALIGNED_LOAD64(s + len - 8) * mul;
  uint64_t d = UNALIGNED_LOAD64(s + len - 16) * k2;
  uint64_t y = rot64(a + b, 43) + rot64(c, 30) + d;
  uint64_t z = hashlen16(y, a + rot64(b + k2, 18) + c, mul);
  uint64_t e = UNALIGNED_LOAD64(s + 16) * mul;
  uint64_t f = UNALIGNED_LOAD64(s + 24);
  uint64_t g = (y + UNALIGNED_LOAD64(s + len - 32)) * mul;
  uint64_t h = (z + UNALIGNED_LOAD64(s + len - 24)) * mul;
  return hashlen16(rot64(e + f, 43) + rot64(g, 30) + h,
                   e + rot64(f + a, 18) + g, mul);
}

uint64_t farmhash64(uint8_t* in, size_t len){
    if(len <= 32){
        if(len <= 16) return hashlen0to16(in, len);
        else return hashlen17to32(in, len);
    } else if(len <= 64) return farmhashlen33to64(in, len);

    uint64_t seed = 81;
    uint64_t x = seed;
    uint64_t y = seed * k1 + 113;
    uint64_t z = shiftmix(y * k2 + 113) * k2;

    uint64_t v1 = 0, w1 = 0;
    int64_t  v2 = 0, w2 = 0; 
    x = x * k2 + UNALIGNED_LOAD64(in);
    uint8_t* end = in + ((len - 1) / 64) * 64;
    uint8_t* last64 = end + ((len - 1) & 63) - 63;
    //assert(in + len - 64 == last64);
    for(; in != end; in += 64) {
        x = rot64(x + y + v1 + UNALIGNED_LOAD64(in + 8), 37) * k1;
        y = rot64(y + v2 + UNALIGNED_LOAD64(in + 48), 42) * k1;
        x ^= w2;
        y += v1 + UNALIGNED_LOAD64(in + 40);
        z = rot64(z + w1, 33) * k1;
        pWeakHashLen32WithSeeds(in, v2 * k1, x + w1, &v1, &v2);
        pWeakHashLen32WithSeeds(in + 32, z + w2, y + UNALIGNED_LOAD64(in + 16), &w1, &w2);
        uint64_t temp = z;
        z = x; x = temp;
    }
    uint64_t mul = k1 + ((z & 0xff) << 1);

    in = last64;
    w1 += ((len - 1) & 63);
    v1 += w1;
    w1 += v1;
    x = rot64(x + y + v1 + UNALIGNED_LOAD64(in + 8), 37) * mul;
    y = rot64(y + v2 + UNALIGNED_LOAD64(in + 48), 42) * mul;
    x ^= w2 * 9;
    y += v1 * 9 + UNALIGNED_LOAD64(in + 40);
    z = rot64(z + w1, 33) * mul;
    pWeakHashLen32WithSeeds(in, v2 * mul, x + w1, &v1, &v2);
    pWeakHashLen32WithSeeds(in + 32, z + w2, y + UNALIGNED_LOAD64(in + 16), &w1, &w2);
    uint64_t temp = z;
    z = x; x = temp;
    return hashlen16(hashlen16(v1, w1, mul) + shiftmix(y) * k0 + z,
                    hashlen16(v2, w2, mul) + x,
                    mul);
    return 1;
}

int l_farmhash32(lua_State* L){ 
  size_t len = 0;
  uint8_t* a = (uint8_t*)luaL_checklstring(L, 1, &len);
  
  char digest[32];

  uint32_t u = farmhash32(a, len);
  sprintf(digest,"%04x",u);
  lua_pushstring(L, digest);
  return 1;
}

int l_farmhash64(lua_State* L){ 
  size_t len = 0;
  uint8_t* a = (uint8_t*)luaL_checklstring(L, 1, &len);
  
  char digest[64];

  uint64_t u = farmhash64(a, len);
  sprintf(digest,"%08lx",u);
  lua_pushstring(L, digest);
  return 1;
}
