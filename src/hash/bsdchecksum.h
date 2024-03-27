#include "../lua.h"
#include <stdint.h>

struct bsdchecksum_hash {
  uint16_t check;
};

/**
 * calculates a bsdchecksum of (len) bytes
 *
 * @param {uint8_t*} input bytes
 * @param {size_t} input length
 * @return {uint16_t} 16 bit checksum
*/
uint16_t bsdchecksum(uint8_t*, size_t);
struct bsdchecksum_hash bsdchecksum_init();
void bsdchecksum_update(uint8_t*, size_t, struct bsdchecksum_hash*);
uint16_t bsdchecksum_final(struct bsdchecksum_hash*);

int l_bsdchecksum(lua_State*);
int l_bsdchecksum_init(lua_State*);
int l_bsdchecksum_update(lua_State*);
int l_bsdchecksum_final(lua_State*);
