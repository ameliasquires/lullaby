#include "../lua.h"
#include <stdint.h>

struct pjw_hash {
    uint32_t hash, high;
};

struct pjw_hash pjw_init();
void pjw_update(uint8_t*, size_t, struct pjw_hash*);
uint32_t pjw_final(struct pjw_hash*);
uint32_t pjw(uint8_t* in, size_t len);

int l_pjw(lua_State*);
int l_pjw_init(lua_State*);
int l_pjw_update(lua_State*);
int l_pjw_final(lua_State*);
