#include "../lua.h"
#include <stdint.h>

struct xor8_hash {
  uint8_t a;
};

struct xor8_hash xor8_init();
void xor8_update(uint8_t*, size_t, struct xor8_hash*);
uint8_t xor8_final(struct xor8_hash*);
uint8_t xor8(uint8_t*, size_t);

int l_xor8(lua_State*);
int l_xor8_init(lua_State*);
int l_xor8_update(lua_State*);
int l_xor8_final(lua_State*);
