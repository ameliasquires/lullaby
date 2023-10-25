#include "../lua.h"

enum version {
    sha256, sha224
};

int l_sha256(lua_State*);
int l_sha224(lua_State*);
