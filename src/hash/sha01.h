#include "../lua.h"
#include <stdint.h>

/**
 * calculates a sha1 (or 0) of bytes
 *  
 * @param {uint8_t} version (1 or 0)
 * @param {char*} output stream
 * @param {const char*} input bytes
 * @return {void}
*/
void i_sha01(uint8_t, char*, const char*);

int l_sha1(lua_State*);
int l_sha0(lua_State*);
