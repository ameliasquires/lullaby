#include "../lua.h"
#include <stdint.h>

struct sysvchecksum_hash {
    uint32_t check;
};

struct sysvchecksum_hash sysvchecksum_init();
void sysvchecksum_update(uint8_t*, size_t, struct sysvchecksum_hash*);
uint16_t sysvchecksum_final(struct sysvchecksum_hash*);
uint16_t sysvchecksum(uint8_t*, size_t);

int l_sysvchecksum(lua_State*);
int l_sysvchecksum_init(lua_State*);
int l_sysvchecksum_update(lua_State*);
int l_sysvchecksum_final(lua_State*);
