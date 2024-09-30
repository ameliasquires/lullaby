#include "lua.h"

int ld_match(lua_State*);
int l_stack_dump(lua_State*);

static const luaL_Reg test_function_list [] = {
  {"_match", ld_match},
  {"stack_dump", l_stack_dump},  
  {NULL,NULL}
};
