#include "lua.h"
#include "table.h"
#include "crypto.h"
#include "io.h"
#include "math.h"
#include "net.h"
#include "thread.h"
#include "test.h"
#include "config.h"

#define open_common(name)\
  int luaopen_lullaby_##name (lua_State* L){\
    luaL_register(L, #name, name##_function_list);\
    int tidx = lua_gettop(L);\
    int idx = i_config_metatable(L, name##_config);\
    lua_pushvalue(L, idx);\
    lua_getmetatable(L, -1);\
    int midx = lua_gettop(L);\
    luaI_tsetcf(L, midx, "__gc", clean_lullaby_##name);\
    lua_setmetatable(L, idx);\
    lua_settop(L, tidx);\
    return 1;\
  }

open_common(table);
open_common(crypto);
open_common(io);
open_common(math);
open_common(net);
open_common(thread);
open_common(test);

#define push(T, name)\
  lua_pushstring(L, #name);\
  luaopen_lullaby_##name(L);\
  lua_settable(L, T);


int luaopen_lullaby(lua_State* L) { 
  lua_newtable(L);
  int top = lua_gettop(L);

  push(top, table);
  push(top, crypto);
  push(top, io);
  push(top, math);
  push(top, net);
  push(top, thread);
  push(top, test);
  luaI_tsets(L, top, "version", GIT_COMMIT);

  lua_settop(L, top); 
  return 1;
}
