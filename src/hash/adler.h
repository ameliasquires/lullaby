#include "../lua.h"
#include <stdint.h>

/**
 * calculates a adler hash of (len) bytes
 *
 * @param {uint8_t*} input bytes
 * @param {size_t} input length
 * @return {uint32_t} 32 bit hash
*/
uint32_t i_adler32(uint8_t*, size_t);

int l_adler32(lua_State*);
