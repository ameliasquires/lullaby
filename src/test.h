#include "lua.h"

int ld_match(lua_State*);

static const luaL_Reg test_function_list [] = {
  {"_match", ld_match},
  
  {NULL,NULL}
};
