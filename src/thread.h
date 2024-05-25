#include "lua.h"

int l_async(lua_State*);
int l_tlock(lua_State*);
int l_tunlock(lua_State*);
int l_buffer(lua_State*);
int l_testcopy(lua_State*);

void lib_thread_clean();

static const luaL_Reg thread_function_list [] = {
  {"async",l_async},
  {"lock",l_tlock},
  {"unlock",l_tunlock},
  {"buffer",l_buffer},
  {"testcopy",l_testcopy},
  {NULL,NULL}
};
