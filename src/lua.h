#include <lua5.1/lua.h>
#include <lua5.1/lualib.h>
#include <lua5.1/lauxlib.h>


#if LUA_VERSION_NUM == 504
    #define lreg(N, FN)\
        lua_pushstring(L, N);\
        luaL_register(L, NULL, FN);\
        lua_settable(L, -3);

    
    #define requiref( L, modname, f, glob ) \
        { luaL_requiref( L, modname, f, glob ); lua_pop( L, 1 ); }

    #define lua_objlen(L,i) lua_rawlen(L,(i))
    #define luaL_register(L, M, F) luaL_newlib(L, F);
#else
    #define lreg(N, FN)\
        lua_newtable(L);\
        luaL_register(L, NULL, FN);\
        lua_setfield(L, 2, N);
    
    //taken straight from luaproc
    #define requiref(L, modname, f, glob){\
      lua_pushcfunction(L, f);\
      lua_pushstring(L, modname); \
      lua_call(L, 1, 1);\
      lua_getfield(L, LUA_GLOBALSINDEX, LUA_LOADLIBNAME);\
      if(lua_type(L, -1) == LUA_TTABLE){\
        lua_getfield(L, -1, "loaded");\
        if(lua_type(L, -1) == LUA_TTABLE){\
          lua_getfield(L, -1, modname);\
          if(lua_type(L, -1) == LUA_TNIL) {\
            lua_pushvalue(L, 1);\
            lua_setfield(L, -3, modname);\
          }\
        lua_pop(L, 1);\
        }\
      lua_pop(L, 1);\
      }\
      lua_pop(L, 1);\
      if(glob){\
        lua_setglobal(L, modname);\
      }else{\
        lua_pop(L, 1);\
      }\
    }
#endif


