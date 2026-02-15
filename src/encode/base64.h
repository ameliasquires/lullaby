#include "../lua.h"

int l_base64encode(lua_State*);
int l_base64decode(lua_State*);

int en_base64(char* in, uint64_t, char* out);
