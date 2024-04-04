#include "../lua.h"
#include <stdint.h>

struct pearson_hash {
    uint8_t ret;
};

struct pearson_hash pearson_init();
void pearson_update(uint8_t*, size_t, struct pearson_hash* hash);
uint8_t pearson_final(struct pearson_hash* hash);
uint8_t pearson(uint8_t*,size_t);

int l_setpearson(lua_State* L);
int l_pearson(lua_State* L);
int l_pearson_init(lua_State* L);
int l_pearson_update(lua_State* L);
int l_pearson_final(lua_State* L);
