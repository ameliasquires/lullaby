#include "test.h"
#include "net/util.h"
#include "types/parray.h"

int ld_match(lua_State* L){
  parray_t* a = parray_init();
  int o = match_param((char*)lua_tostring(L, 1), (char*)lua_tostring(L, 2), a);

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

int l_stack_dump(lua_State* L){
  /*StkId a = L->top.p-2;
    printf("%i %i\n", ttype(s2v(a)), LUA_TSTRING); 
    printf("is string? %i\n", ttisstring(&a->val));
    printf("%s\n", tsvalue(&a->val)->contents);*/
  //int level = 0;
  //lua_lock(L);
  //for(CallInfo* ci = L->ci; ci != &L->base_ci; ci = ci->previous) level++;
  //lua_unlock(L);
  //level -= 2;

  //printf("%i\n", level);
  lua_Debug info;
  for(int i = 0;; i++){
    if(lua_getstack(L, i, &info) == 0) break;
    for(int idx = 1;; idx++){
      const char* name = lua_getlocal(L, &info, idx);
      if(name == NULL) break;
      const char* type = lua_typename(L, lua_type(L, -1));
      printf("l:%i | %s = %s (%s)\n", i, name, lua_tostring(L, -1), type);
      lua_pop(L, 1);
    }
  }
  //lua_getstack(L, level, &info);
  //const char* name = lua_getlocal(L, &info, 2);
  return 0;
}

int l_upvalue_key_table(lua_State* L){
  lua_upvalue_key_table(L, 1);
  return 1;
}

int rea(uint64_t len, str** _output, void** v){
  str* output = *_output;
  str_push(output, "awa!!!! test\n");  
  *_output = output;
  return 1;
}

int l_stream_test(lua_State* L){
  luaI_newstream(L, rea, NULL, 0);
  return 1;
}
