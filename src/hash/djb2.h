#include "../lua.h"
#include <stdint.h>

uint32_t djb2(uint8_t*, size_t);
int l_djb2(lua_State*);
