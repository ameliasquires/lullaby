#include "../lua.h"
#include <stdint.h>

/**
 * calculates a crc of (len) bytes
 *
 * @param {uint8_t*} input bytes
 * @param {size_t} input length
 * @return {uint8_t} 8 bit checksum
*/
uint8_t i_crc8(uint8_t*, size_t);

/**
 * calculates a crc of (len) bytes
 *
 * @param {uint8_t*} input bytes
 * @param {size_t} input length
 * @return {uint16_t} 16 bit checksum
*/
uint16_t i_crc16(uint8_t*, size_t);

/**
 * calculates a crc of (len) bytes
 *
 * @param {uint8_t*} input bytes
 * @param {size_t} input length
 * @return {uint32_t} 32 bit checksum
*/
uint32_t i_crc32(uint8_t*, size_t);

int l_crc8(lua_State*);
int l_crc16(lua_State*);
int l_crc32(lua_State*);
