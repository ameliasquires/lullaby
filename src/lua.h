#include <lua5.4/lua.h>
#include <lua5.4/lualib.h>
#include <lua5.4/lauxlib.h>

#define lua_objlen(L,i) lua_rawlen(L,(i))
#define luaL_register(L, M, F) luaL_newlib(L, F);


