#include "../lua.h"
#include <stdint.h>

struct iv {
    uint64_t h0, h1, h2, h3, h4, h5, h6, h7;
};

static const struct iv sha512_iv = {.h0 = 0x6a09e667f3bcc908, .h1 = 0xbb67ae8584caa73b, .h2 = 0x3c6ef372fe94f82b, .h3 = 0xa54ff53a5f1d36f1, 
    .h4 = 0x510e527fade682d1, .h5 = 0x9b05688c2b3e6c1f, .h6 = 0x1f83d9abfb41bd6b, .h7 = 0x5be0cd19137e2179};

static const struct iv sha384_iv = {.h0 = 0xcbbb9d5dc1059ed8, .h1 = 0x629a292a367cd507, .h2 = 0x9159015a3070dd17, .h3 = 0x152fecd8f70e5939, 
    .h4 = 0x67332667ffc00b31, .h5 = 0x8eb44a8768581511, .h6 = 0xdb0c2e0d64f98fa7, .h7 = 0x47b5481dbefa4fa4};

struct iv sha_iv_gen(int i);
void sha2_512_t(uint8_t* out, uint8_t* in, size_t len, int t);
void sha2_512(uint8_t* out, uint8_t* in, size_t len);
void sha2_384(uint8_t* out, uint8_t* in, size_t len);

int l_sha512(lua_State*);
int l_sha384(lua_State*);
int l_sha512_t(lua_State*);
