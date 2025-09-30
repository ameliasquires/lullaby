#include "../lua.h"
#include <stdint.h>

struct crc32_hash {
  uint32_t crc;
};

struct crc16_hash {
  uint16_t crc;
};

struct crc8_hash {
  uint8_t crc;
};

uint8_t crc8(uint8_t*, size_t len);
struct crc8_hash crc8_init();
void crc8_update(uint8_t*, size_t, struct crc8_hash*);
uint8_t crc8_final(struct crc8_hash*);

uint16_t crc16(uint8_t*, size_t len);
struct crc16_hash crc16_init();
void crc16_update(uint8_t*, size_t, struct crc16_hash*);
uint16_t crc16_final(struct crc16_hash*);

uint32_t crc32(uint8_t*, size_t len);
struct crc32_hash crc32_init();
void crc32_update(uint8_t*, size_t, struct crc32_hash*);
uint32_t crc32_final(struct crc32_hash*);

int l_crc8(lua_State*);
int l_crc8_init(lua_State*);
int l_crc8_update(lua_State*);
int l_crc8_final(lua_State*);

int l_crc16(lua_State*);
int l_crc16_init(lua_State*);
int l_crc16_update(lua_State*);
int l_crc16_final(lua_State*);

int l_crc32(lua_State*);
int l_crc32_init(lua_State*);
int l_crc32_update(lua_State*);
int l_crc32_final(lua_State*);
