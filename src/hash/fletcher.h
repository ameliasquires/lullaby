#include "../lua.h"
#include <stdint.h>

struct fletcher8_hash {
    uint8_t s1, s2;
};

struct fletcher16_hash {
    uint16_t s1, s2;
};

struct fletcher32_hash {
    uint32_t s1, s2;
};

uint8_t fletcher8(uint8_t*,size_t);
struct fletcher8_hash fletcher8_init();
void fletcher8_update(uint8_t*, size_t, struct fletcher8_hash*);
uint8_t fletcher8_final(struct fletcher8_hash*);

uint16_t fletcher16(uint8_t*,size_t);
struct fletcher16_hash fletcher16_init();
void fletcher16_update(uint8_t*, size_t, struct fletcher16_hash*);
uint16_t fletcher16_final(struct fletcher16_hash*);

uint32_t fletcher32(uint8_t*,size_t);
struct fletcher32_hash fletcher32_init();
void fletcher32_update(uint8_t*, size_t, struct fletcher32_hash*);
uint32_t fletcher32_final(struct fletcher32_hash*);

int l_fletcher32(lua_State*);
int l_fletcher32_init(lua_State*);
int l_fletcher32_update(lua_State*);
int l_fletcher32_final(lua_State*);

int l_fletcher16(lua_State*);
int l_fletcher16_init(lua_State*);
int l_fletcher16_update(lua_State*);
int l_fletcher16_final(lua_State*);

int l_fletcher8(lua_State*);
int l_fletcher8_init(lua_State*);
int l_fletcher8_update(lua_State*);
int l_fletcher8_final(lua_State*);