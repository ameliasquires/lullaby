#include "../lua.h"
#include <stdint.h>

/**
 * calculates a sysv checksum of (len) bytes
 *
 * @param {uint8_t*} input bytes
 * @param {size_t} input length
 * @return {uint32_t} 32 bit checksum
*/
uint32_t i_sysvchecksum(uint8_t*, size_t);

int l_sysvchecksum(lua_State*);
