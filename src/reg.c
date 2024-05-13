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

  lib_thread_clean();
  //sigHandle(0);
  return 0;
}

int luaopen_llib(lua_State* L) { 
    lua_newuserdata(L, 1);
    int ud = lua_gettop(L);
    lua_newtable(L);
    int meta = lua_gettop(L);
    luaI_tsetcf(L, meta, "__gc", lua_exit);
    lua_pushvalue(L, meta);
    lua_setmetatable(L, ud);
    
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
