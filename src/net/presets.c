#include "presets.h"
#include "lua.h"
#include "../types/path.h"

int path_sanitize(str* path){
  return 0;
}

int serve_callback(lua_State* L){
  int req = 2;
  //not sure why i even wanted this upvalue
  //const char* base = lua_tostring(L, lua_upvalueindex(1));
  lua_getfield(L, req, "path");
  const char* request_path = lua_tostring(L, -1);

  lua_settop(L, 1);
  lua_pushstring(L, request_path + 1);
  l_sendfile(L);
  return 0;
}

int l_preset_serve(lua_State* L){
  const char* rpath = luaL_checkstring(L, 1);

  path_t* p = path_init();
  path_add(p, rpath);
  str* base = path_tostr(p);

  path_add(p, "*");
  path_calc(p);
  str* c = path_tostr(p);

  lua_pushstring(L, "GET");
  lua_pushstring(L, c->c);

  lua_pushstring(L, base->c);
  lua_pushcclosure(L, serve_callback, 1);
 
  str_free(base);
  str_free(c);
  path_free(p);
  return 3;
}
