#include "../lua.h"

enum fnv_version {
    v_1, v_a, v_0
};

int l_fnv_1(lua_State*);
int l_fnv_a(lua_State*);
int l_fnv_0(lua_State*);
