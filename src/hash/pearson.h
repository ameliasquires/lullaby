#include "../lua.h"
#include <stdint.h>
/**
 * calculates a pearson hash of (len) bytes
 *
 * @param {uint8_t*} input bytes
 * @param {size_t} input length
 * @return {uint8_t} 8 bit hash
*/
uint8_t i_pearson(uint8_t*,size_t);

int l_setpearson(lua_State* L);
int l_pearson(lua_State* L);
