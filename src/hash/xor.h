#include "../lua.h"
#include <stdint.h>
/**
 * calculates a xor hash of (len) bytes
 *
 * @param {uint8_t*} input bytes
 * @param {size_t} input length
 * @return {uint8_t} 8 bit checksum
*/
uint8_t xor8(uint8_t*, size_t);

int l_xor8(lua_State*);
