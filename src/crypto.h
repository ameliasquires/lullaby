#include "lua.h"
#include "i_util.h"

int l_md5(lua_State*);

static const luaL_Reg crypto_function_list [] = {
      {"md5",l_md5},

      {NULL,NULL}
};

