#include "../lua.h"

struct sha256_hash {
    uint8_t* buffer;
    size_t bufflen;
    uint64_t total;
    uint32_t h0, h1, h2, h3, h4, h5, h6, h7;
};

int l_sha256(lua_State*);
int l_sha256_init(lua_State*);
int l_sha256_update(lua_State*);
int l_sha256_final(lua_State*);

int l_sha224(lua_State*);
int l_sha224_init(lua_State*);
int l_sha224_update(lua_State*);
int l_sha224_final(lua_State*);
