#include <lua5.4/lua.h>
#include <lua5.4/lualib.h>
#include <lua5.4/lauxlib.h>
#include <stdlib.h>

#ifndef __lua_h
#define __lua_h
enum deep_copy_flags {
  SKIP_META = (1 << 0),
  SKIP_GC = (1 << 1),
  IS_META = (1 << 2),
  SKIP__G = (1 << 3),
};
#endif 

#ifndef GIT_COMMIT
#define GIT_COMMIT "unknown"
#endif

void* __malloc_(size_t);
void __free_(void*);

void luaI_deepcopy(lua_State* src, lua_State* dest, enum deep_copy_flags);
void luaI_deepcopy2(lua_State* src, lua_State* dest);

void lua_set_global_table(lua_State*);
//todo: char* _luaL_tolstring(lua_State*, int, size_t*);

void luaI_copyvars(lua_State* src, lua_State* dest);

void lua_upvalue_key_table(lua_State* L, int fidx);
int lua_assign_upvalues(lua_State* L, int fidx);

//generic macro that takes other macros (see below)
#define _tset_b(L, Tidx, K, V, F)\
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

#define luaI_treplk(L, Tidx, K, nK){\
  lua_pushstring(L, K);\
  lua_gettable(L, Tidx);\
  int _v = lua_gettop(L);\
  luaI_tsetv(L, Tidx, nK, _v);\
  lua_pushstring(L, K);\
  lua_pushnil(L);\
  lua_settable(L, Tidx);\
  lua_pop(L, 1);}

#define luaI_error(L, en, str){\
  lua_pushnil(L);\
  lua_pushstring(L, str);\
  lua_pushinteger(L, en);\
  return 3;}


int writer(lua_State*, const void*, size_t, void*);

#if LUA_VERSION_NUM == 504
    #define lua_objlen(L,i) lua_rawlen(L,(i))

    #define luaL_register(L, M, F) luaL_newlib(L, F);
#elif LUA_VERSION_NUM == 501
    #define luaL_tolstring lua_tolstring
    
    #define lua_dump(A, B, C, D) lua_dump(A, B, C)
    
    #define lua_rawlen lua_objlen

    #define lua_gc(A, B) lua_gc(A, B, 0)

    #define lua_pushglobaltable(L) {lua_getglobal(L, "_G");}
#endif


