#include "lua.h"
#include "table.h"
#include "crypto.h"
#include "config.h"
#include "io.h"
#include "math.h"

int luaopen_llib(lua_State* L) { 
    lua_newtable(L);
    
    //create <lib>.array functions
    lua_newtable(L);
    luaL_register(L, NULL, array_function_list);
    lua_setfield(L, 2, "array");
   
    lua_newtable(L);
    luaL_register(L, NULL, crypto_function_list);
    lua_setfield(L, 2, "crypto");
    
    lua_newtable(L);
    luaL_register(L, NULL, io_function_list);
    lua_setfield(L, 2, "io");

    lua_newtable(L);
    luaL_register(L, NULL, math_function_list);
    lua_setfield(L, 2, "math");

    lua_newtable(L);
    luaL_register(L, NULL, config_function_list);
    lua_setfield(L, 2, "config");
    //make llib global
    lua_setglobal(L, "llib");
    return 1;
}
