#include <lua.h>
#include <lualib.h>
#include <lauxlib.h>


#if LUA_VERSION_NUM == 504
    #define lreg(N, FN)\
        lua_pushstring(L, N);\
        luaL_register(L, NULL, FN);\
        lua_settable(L, -3);

    #define lua_objlen(L,i) lua_rawlen(L,(i))
    #define luaL_register(L, M, F) luaL_newlib(L, F);
#else
    #define lreg(N, FN)\
        lua_newtable(L);\
        luaL_register(L, NULL, FN);\
        lua_setfield(L, 2, N);
#endif


