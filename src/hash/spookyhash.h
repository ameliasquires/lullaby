#include "../lua.h"
#include <stdint.h>

enum spooky_version {
    spv1, spv2
};

void spookyhash128(uint8_t* in, size_t len, uint64_t* hash1, uint64_t* hash2, enum spooky_version v);
uint64_t spookyhash64(uint8_t *message, size_t length, uint64_t seed, enum spooky_version v);
uint32_t spookyhash32(uint8_t *message, size_t length, uint32_t seed, enum spooky_version v);

int l_spookyhash128_v1(lua_State*);
int l_spookyhash128_v2(lua_State*);
int l_spookyhash64_v1(lua_State*);
int l_spookyhash64_v2(lua_State*);
int l_spookyhash32_v1(lua_State*);
int l_spookyhash32_v2(lua_State*);
