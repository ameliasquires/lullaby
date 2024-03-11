#include "../lua.h"
#include <stdint.h>

enum metrohash_version {
    v1, v2
};

uint64_t metrohash64(uint8_t* in, size_t len, uint32_t seed, enum metrohash_version v);
void metrohash128(uint8_t* in, size_t len, uint32_t seed, uint64_t *a, uint64_t *b, enum metrohash_version ver);

int l_metrohash64_v1(lua_State*);
int l_metrohash64_v2(lua_State*);
int l_metrohash128_v1(lua_State*);
int l_metrohash128_v2(lua_State*);
