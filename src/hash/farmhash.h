#include "../lua.h"
#include <stdint.h>

uint32_t farmhash32(uint8_t* in, size_t len);
uint64_t farmhash64(uint8_t* in, size_t len);

int l_farmhash32(lua_State*);
int l_farmhash64(lua_State*);
