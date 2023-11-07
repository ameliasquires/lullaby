#include "../lua.h"
#include <stdint.h>

/**
 * calculates a fletcher hash of (len) bytes
 *
 * @param {uint8_t*} input bytes
 * @param {size_t} input length
 * @return {uint8_t} 8 bit hash
*/
uint8_t i_fletcher8(uint8_t*,size_t);

/**
 * calculates a fletcher hash of (len) bytes
 *
 * @param {uint8_t*} input bytes
 * @param {size_t} input length
 * @return {uint16_t} 16 bit checksum
*/
uint16_t i_fletcher16(uint8_t*,size_t);

/**
 * calculates a fletcher hash of (len) bytes
 *
 * @param {uint8_t*} input bytes
 * @param {size_t} input length
 * @return {uint32_t} 32 bit checksum
*/
uint32_t i_fletcher32(uint8_t*,size_t);

int l_fletcher32(lua_State*);
int l_fletcher16(lua_State*);
int l_fletcher8(lua_State*);
