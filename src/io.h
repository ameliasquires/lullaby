#include "lua.h"

int l_readfile(lua_State*);

static const luaL_Reg io_function_list [] = {
  {"readfile",l_readfile},
  {NULL,NULL}
};
