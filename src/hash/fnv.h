#include "../lua.h"
#include "stdint.h"

enum fnv_version {
  v_1, v_a, v_0
};

struct fnv_1_hash {
  enum fnv_version A;
  uint64_t hash;
};

uint64_t fnv_1(uint8_t*, size_t, enum fnv_version);
struct fnv_1_hash fnv_1_init(enum fnv_version);
void fnv_1_update(uint8_t*, size_t, struct fnv_1_hash*);
uint64_t fnv_1_final(struct fnv_1_hash*);

int l_fnv_1(lua_State*);
int l_fnv_1_init(lua_State*);
int l_fnv_1_update(lua_State*);
int l_fnv_1_final(lua_State*);

int l_fnv_a(lua_State*);
int l_fnv_a_init(lua_State*);
int l_fnv_a_update(lua_State*);
int l_fnv_a_final(lua_State*);

int l_fnv_0(lua_State*);
int l_fnv_0_init(lua_State*);
int l_fnv_0_update(lua_State*);
int l_fnv_0_final(lua_State*);
