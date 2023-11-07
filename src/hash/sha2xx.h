#include "../lua.h"

enum version {
    sha256, sha224
};

/**
 * calculates a sha2 hash of bytes
 *
 * @param {enum version} version to use
 * @param {char*} output stream 
 * @param {char*} input bytes
 * @return {void}
*/
void i_sha2xx(enum version, char* out_stream, char* input);

int l_sha256(lua_State*);
int l_sha224(lua_State*);
