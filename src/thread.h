#include "lua.h"

int l_async(lua_State*);
int l_tlock(lua_State*);
int l_tunlock(lua_State*);

static const luaL_Reg thread_function_list [] = {
  {"async",l_async},
  {"lock",l_tlock},
  {"unlock",l_tunlock},
  {NULL,NULL}
};
