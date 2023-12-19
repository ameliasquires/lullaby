#include "config.h"
#include <string.h>

int _print_type = 0;
int _max_depth = 2;
int _start_nl_at = 3;
int _file_malloc_chunk = 512;

int get_config_map(const char* key){
  for(int i = 0; config_map[i].key != NULL; i++){
    if(strcmp(config_map[i].key,key) == 0) return i;
  }
  return -1;
}

int l_set(lua_State* L) {
    luaL_checktype(L, 1, LUA_TTABLE);
    get_config_map("print_type");
    lua_pushnil(L);
      for(;lua_next(L,1) != 0;){
        int ind = get_config_map(lua_tostring(L,-2)); 

        if(ind != -1) {
          *config_map[ind].value = lua_tonumber(L,-1);
        }
        lua_pop(L,1);
      }
    return 0;
}
