#include "lua.h"
#include "table.h"
#include "crypto.h"
#include "config.h"
#include "io.h"
#include "math.h"
#include "net.h"
#include <signal.h>

static int lua_exit(lua_State* L){
  printf("hi\n");
  return 0;
}

void sigHandle(int s){
  signal(s, SIG_IGN);
  printf("meow\n");
  signal(s, sigHandle);
}

int luaopen_llib(lua_State* L) { 
    /*
    
    lua_newuserdata(L, sizeof(void*));
    luaL_newmetatable(L, "gc");
		lua_pushstring(L, "__gc");
		lua_pushcfunction(L, &lua_exit);
		lua_settable(L, -3);

    lua_setmetatable(L, -2);				
		lua_setfield(L, LUA_REGISTRYINDEX, "grr");
    signal(SIGTERM, sigHandle);
		signal(SIGINT, sigHandle);
*/
    //create <lib>.array functions
    lua_newtable(L);

    //lua_newtable(L);
    lua_pushstring(L, "array");
    luaL_register(L, NULL, array_function_list);
    lua_settable(L, -3);

    lua_pushstring(L, "crypto");
    luaL_register(L, NULL, crypto_function_list);
    lua_settable(L, -3);

    lua_pushstring(L, "io");
    luaL_register(L, NULL, io_function_list);
    lua_settable(L, -3);

    lua_pushstring(L, "math");
    luaL_register(L, NULL, math_function_list);
    lua_settable(L, -3);

    lua_pushstring(L, "config");
    luaL_register(L, NULL, config_function_list);
    lua_settable(L, -3);

    lua_pushstring(L, "net");
    luaL_register(L, NULL, net_function_list);
    lua_settable(L, -3);

    //make llib global
    lua_setglobal(L, "llib");
    return 1;
}
