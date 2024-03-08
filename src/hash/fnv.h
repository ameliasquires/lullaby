#include "../lua.h"
#include "stdint.h"

enum fnv_version {
    v_1, v_a, v_0
};

uint64_t fnv_1(uint8_t*, size_t, enum fnv_version);

int l_fnv_1(lua_State*);
int l_fnv_a(lua_State*);
int l_fnv_0(lua_State*);
