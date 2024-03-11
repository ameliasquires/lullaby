#include "../lua.h"
#include <stdint.h>

uint64_t sdbm(uint8_t* in, size_t len);
int l_sdbm(lua_State*);
