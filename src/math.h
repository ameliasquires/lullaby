#include "lua.h"

int l_lcm(lua_State*);
int l_gcd(lua_State*);

static const luaL_Reg math_function_list [] = {
  {"lcm",l_lcm},
  //{"gcd",l_gcd},
  
  {NULL,NULL}
};
