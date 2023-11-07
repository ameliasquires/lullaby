#include "../lua.h"

/**
 * calculates a md5 of bytes
 *
 * @param {char*} input bytes
 * @param {char[64]} output stream
 * @return {void}
*/
void i_md5(char*,char*);

int l_md5(lua_State*);
