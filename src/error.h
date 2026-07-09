#include "lua.h"
#include "config.h"

#define luaI_assert(L, eq){_helperluaI_assert(L, eq, __FILE__, __LINE__);}
#define _helperluaI_assert(L, eq, file, line){\
  if(!(eq)){\
    char err[1024] = {0};\
    sprintf(err, "(%s:%i) %s assertion failed", file, line, #eq);\
    return luaI_error(L, err);}}\

int luaI_error(lua_State* L, const char*);

int l_is(lua_State*);

#define clean_lullaby_error luaI_nothing

static const luaL_Reg error_function_list [] = {
  {"is", l_is},

  {NULL,NULL}
};

static struct config error_config[] = {
  {.type = c_none}
};
