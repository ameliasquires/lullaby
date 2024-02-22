#include "lua.h"

int l_listen(lua_State*);

static char* http_codes[600] = {0};


static const luaL_Reg net_function_list [] = {
  {"listen",l_listen},
  
  {NULL,NULL}
};
