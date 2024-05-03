#include "lua.h"

int l_async(lua_State*);

static const luaL_Reg thread_function_list [] = {
  {"async",l_async},
  
  {NULL,NULL}
};
