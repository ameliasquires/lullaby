#include "../lua.h"
#include <stdint.h>

uint8_t i_buzhash8(uint8_t*, size_t);
uint16_t i_buzhash16(uint8_t*, size_t);

int l_setbuzhash(lua_State*);
int l_buzhash8(lua_State*);
int l_buzhash16(lua_State*);
