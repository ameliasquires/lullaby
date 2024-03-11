#include "../lua.h"
#include <stdint.h>

uint64_t fasthash64(uint8_t* in, size_t len, uint64_t seed);
uint32_t fasthash32(uint8_t *buf, size_t len, uint32_t seed);

int l_fasthash32(lua_State*);
int l_fasthash64(lua_State*);
