#include "config.h"
#include "io.h"
#include <string.h>

int m_config_index(lua_State* L){
  lua_pushstring(L, "_config");
  lua_gettable(L, 1);
  struct config* conf = lua_touserdata(L, -1);
  int at = 0;
  struct config cc;

  for(;;at++){
    cc = conf[at];

    if(cc.type == c_none){
      lua_pushnil(L);
      return 1;
    }

    if(strcmp(cc.name, lua_tostring(L, 2)) == 0) {
      break;
    }
  }

  switch(cc.type){
    case c_int:
      lua_pushinteger(L, *cc.value.c_int);
      break;
    case c_string:
      lua_pushlstring(L, *cc.value.c_string, *cc.value.len);
      break;
    case c_number:
      lua_pushnumber(L, *cc.value.c_number);
      break;
    case c_none:;
  }

  return 1;
}

int m_config_newindex(lua_State* L){
  lua_pushstring(L, "_config");
  lua_gettable(L, 1);
  struct config* conf = lua_touserdata(L, -1);
  int at = 0;
  struct config cc;

  for(;;at++){
    cc = conf[at];

    if(cc.type == c_none){
      lua_pushnil(L);
      return 1;
    }

    if(strcmp(cc.name, lua_tostring(L, 2)) == 0) {
      break;
    }
  }

  switch(cc.type){
    case c_int:
      *cc.value.c_int = lua_tointeger(L, 3);
      break;
    case c_string:
      *cc.value.c_string = (char*)luaL_tolstring(L, 3, cc.value.len);
      break;
    case c_number:
      *cc.value.c_number = lua_tonumber(L, 3);
      break;
    case c_none:;
  }

  return 1;
};

int i_config_metatable(lua_State* L, struct config* conf){
  int idx = lua_gettop(L);
  luaI_tsetlud(L, idx, "_config", conf);
  
  lua_newtable(L);
  int meta_idx = lua_gettop(L);
  luaI_tsetcf(L, meta_idx, "__index", m_config_index);
  luaI_tsetcf(L, meta_idx, "__newindex", m_config_newindex);

  lua_pushvalue(L, meta_idx);
  lua_setmetatable(L, idx);

  return 1;
}
