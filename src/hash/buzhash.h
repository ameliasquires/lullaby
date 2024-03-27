#include "../lua.h"
#include <stdint.h>

uint8_t buzhash8(uint8_t*, size_t);
uint16_t buzhash16(uint8_t*, size_t);

int l_setbuzhash(lua_State*);
int l_buzhash8(lua_State*);
int l_buzhash16(lua_State*);
