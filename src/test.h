#include "lua.h"
#include "config.h"

int ld_match(lua_State*);
int l_stack_dump(lua_State*);
int l_upvalue_key_table(lua_State* L);

static const luaL_Reg test_function_list [] = {
  {"_match", ld_match},
  {"stack_dump", l_stack_dump}, 
  {"upvalue_key_table", l_upvalue_key_table},
  {NULL,NULL}
};

static struct config test_config[] = {
  {.type = c_none}
};
