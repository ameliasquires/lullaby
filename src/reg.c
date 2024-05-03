#include "lua.h"
#include "table.h"
#include "crypto.h"
#include "config.h"
#include "io.h"
#include "math.h"
#include "net.h"
#include "thread.h"
#include <signal.h>
#include <stdlib.h>

void sigHandle(int s){
  //signal(s, SIG_IGN);

  //signal(s, sigHandle);
  exit(s);
}

static int lua_exit(lua_State* L){

  sigHandle(0);
  return 0;
}

int luaopen_llib(lua_State* L) { 
    lua_newuserdata(L, sizeof(void*));
    luaL_newmetatable(L, "gc");
		lua_pushstring(L, "__gc");
		lua_pushcfunction(L, &lua_exit);
		lua_settable(L, -3);

    lua_setmetatable(L, -2);				
		lua_setfield(L, LUA_REGISTRYINDEX, "grr");
    signal(SIGTERM, sigHandle);
		signal(SIGINT, sigHandle);

    //create <lib>.array functions
    lua_newtable(L);

    //lua_newtable(L);

    lreg("array", array_function_list);
    lreg("crypto", crypto_function_list);
    lreg("io", io_function_list);
    lreg("math", math_function_list);
    lreg("config", config_function_list);
    lreg("net", net_function_list);
    lreg("thread", thread_function_list);

    //make llib global
    lua_setglobal(L, "llib");
    return 1;
}
