#include "../lua.h"
#include <stdint.h>

struct djb2_hash {
  uint32_t hash;
};

struct djb2_hash djb2_init();
void djb2_update(uint8_t*, size_t, struct djb2_hash*);
uint32_t djb2_final(struct djb2_hash*);
uint32_t djb2(uint8_t*, size_t);

int l_djb2(lua_State*);
int l_djb2_init(lua_State*);
int l_djb2_update(lua_State*);
int l_djb2_final(lua_State*);
