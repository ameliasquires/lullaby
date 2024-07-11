#include <lua5.4/lua.h>
#include <lua5.4/lualib.h>
#include <lua5.4/lauxlib.h>
#include <stdlib.h>

#ifndef __lua_h
#define __lua_h
enum deep_copy_flags {
  SKIP_META = (1 << 0),
  SKIP_GC = (1 << 1),
  IS_META = (1 << 2)
};
#endif 

void* __malloc_(size_t);
void __free_(void*);

void luaI_deepcopy(lua_State* src, lua_State* dest, enum deep_copy_flags);
void luaI_deepcopy2(lua_State* src, lua_State* dest);

void lua_set_global_table(lua_State*);
//todo: char* _luaL_tolstring(lua_State*, int, size_t*);

//generic macro that takes other macros (see below)
#define _tset_b(L, Tidx, K, V, F)\
    lua_pushvalue(L, Tidx);\
    lua_pushstring(L, K);\
    F(L, V);\
    lua_settable(L, Tidx);

//some macros to make batch adding easier (may switch to arrays for this later)
#define luaI_tseti(L, Tidx, K, V)\
    _tset_b(L, Tidx, K, V, lua_pushinteger)
#define luaI_tsetb(L, Tidx, K, V)\
  _tset_b(L, Tidx, K, V, lua_pushboolean)
#define luaI_tsetsl(L, Tidx, K, V, len)\
    lua_pushvalue(L, Tidx);\
    lua_pushstring(L, K);\
    lua_pushlstring(L, V, len);\
    lua_settable(L, Tidx);
#define luaI_tsets(L, Tidx, K, V)\
    _tset_b(L, Tidx, K, V, lua_pushstring)
#define luaI_tsetv(L, Tidx, K, V)\
    _tset_b(L, Tidx, K, V, lua_pushvalue)
#define luaI_tsetcf(L, Tidx, K, V)\
    _tset_b(L, Tidx, K, V, lua_pushcfunction)
#define luaI_tsetlud(L, Tidx, K, V)\
    _tset_b(L, Tidx, K, V, lua_pushlightuserdata)

int writer(lua_State*, const void*, size_t, void*);

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
    //todo: #define luaL_tolstring(L, idx, n) _luaL_tolstring(L, idx, n)

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


