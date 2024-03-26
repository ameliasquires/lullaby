#include "../lua.h"
#include <stdint.h>

struct adler32_hash {
  uint16_t a;
  uint16_t b;
};

struct adler32_hash adler32_init();
void adler32_update(uint8_t*, size_t, struct adler32_hash*);
uint32_t adler32_final(struct adler32_hash*);
uint32_t adler32(uint8_t*, size_t);

int l_adler32(lua_State*);
int l_adler32_init(lua_State*);
int l_adler32_update(lua_State*);
int l_adler32_final(lua_State*);
