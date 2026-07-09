#include "error.h"
#include "lua.h"

#define LU_ERROR_STRING "lullaby.error"

struct error_data {
  char* name, *traceback;
};

int _error_gc(lua_State* L){
  struct error_data *e = lua_touserdata(L, 1);

  free(e->name);
  free(e->traceback);

  return 1;
}

int luaI_error(lua_State* L, const char* error){
  struct error_data* e = lua_newuserdata(L, sizeof * e);
  int idx = lua_gettop(L);
  e->name = calloc(sizeof * e->name, strlen(error));
  memcpy(e->name, error, strlen(error));

  luaL_traceback(L, L, error, 0);
  size_t len;
  const char* traceback = lua_tolstring(L, -1, &len);
  e->traceback = calloc(sizeof * e->traceback, len + 1);
  memcpy(e->traceback, traceback, len);

 
  lua_newtable(L);
  int meta = lua_gettop(L);
  lua_pushvalue(L, -1);
  lua_setmetatable(L, idx);

  luaI_tsets(L, meta, "__name", LU_ERROR_STRING);
  luaI_tsetcf(L, meta, "__gc", _error_gc);

  lua_settop(L, idx);
  return 1;
}

int _error_print(lua_State* L){
  lua_getfield(L, 1, "error");
  if(!lua_toboolean(L, -1)){
    lua_settop(L, 1);
    return 1;
  }

  lua_getfield(L, 1, "_");
  struct error_data *e = lua_touserdata(L, -1);

  printf("%s\n", e->name);
  lua_settop(L, 1);
  return 1;
}

int _error_raise(lua_State* L){
  lua_getfield(L, 1, "error");
  if(!lua_toboolean(L, -1)){
    lua_settop(L, 1);
    return 1;
  }

  lua_getfield(L, 1, "_");
  struct error_data *e = lua_touserdata(L, -1);

  luaL_error(L, e->traceback);
  lua_settop(L, 1);
  return 1;
}

int l_is(lua_State* L){
  int error = 1;
  lua_newtable(L);
  int idx = lua_gettop(L);
  luaI_tsetv(L, idx, "_", 1)

  luaI_tsetcf(L, idx, "print", _error_print);
  luaI_tsetcf(L, idx, "raise", _error_raise);

  if(lua_type(L, 1) != LUA_TUSERDATA) error = 0;
  if(!lua_getmetatable(L, 1)) error = 0;
  lua_getfield(L, -1, "__name");

  if(lua_type(L, -1) != LUA_TSTRING || strcmp(lua_tostring(L, -1), LU_ERROR_STRING) != 0) error = 0;

  luaI_tsetb(L, idx, "error", error);
  lua_settop(L, idx);
  
  return 1;
}
