#include "../lua.h"
#include <stdint.h>

uint32_t murmur1_32(uint8_t* in, size_t len, uint32_t seed);
uint32_t murmur2_32(uint8_t* in, size_t len, uint32_t seed);

int l_murmur1_32(lua_State*);
int l_murmur2_32(lua_State*);
