#include "../lua.h"
#include <stdint.h>

/**
 * calculates a bsdchecksum of (len) bytes
 *
 * @param {uint8_t*} input bytes
 * @param {size_t} input length
 * @return {uint16_t} 16 bit checksum
*/
uint16_t i_bsdchecksum(uint8_t*, size_t);

int l_bsdchecksum(lua_State*);
