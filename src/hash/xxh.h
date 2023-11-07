#include "../lua.h"
#include <stdint.h>

/**
 * calculates a xxhash32 of (len) bytes
 *
 * @param {uint8_t*} input bytes
 * @param {uint32_t} hash seed
 * @param {size_t} input length
 * @return {uint32_t} 32 bit hash
*/
uint32_t i_xxhash32(uint8_t*, uint32_t, size_t);

/**
 * calculates a xxhash64 of (len) bytes
 *
 * @param {uint8_t*} input bytes
 * @param {uint64_t} hash seed
 * @param {size_t} input length
 * @return {uint64_t} 64 bit hash
*/
uint64_t i_xxhash64(uint8_t*, uint64_t, size_t);

int l_xxh32(lua_State*);
int l_xxh64(lua_State*);
