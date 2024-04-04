#include "../lua.h"
#include <stdint.h>

struct sdbm_hash {
    uint64_t hash;
};

struct sdbm_hash sdbm_init();
void sdbm_update(uint8_t*, size_t len, struct sdbm_hash*);
uint64_t sdbm_final(struct sdbm_hash*);
uint64_t sdbm(uint8_t* in, size_t len);

int l_sdbm(lua_State*);
int l_sdbm_init(lua_State*);
int l_sdbm_update(lua_State*);
int l_sdbm_final(lua_State*);
