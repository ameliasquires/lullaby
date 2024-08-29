#include "test.h"
#include "net/util.h"
#include "types/parray.h"

int ld_match(lua_State* L){
  parray_t* a = parray_init();
  int o = match_param(lua_tostring(L, 1), lua_tostring(L, 2), a);
  
  if(o == 0){
    lua_pushinteger(L, o);
    return 1;
  }

  lua_newtable(L);
  int tidx = lua_gettop(L);
  for(int i = 0; i != a->len; i++){
    //printf("%s:%s\n",a->P[i].key->c, (char*)a->P[i].value);
    luaI_tsets(L, tidx, a->P[i].key->c, (char*)a->P[i].value);
  }

  lua_pushinteger(L, o);
  lua_pushvalue(L, tidx);
  return 2;
}

