#include "lua.h"
#include "table.h"
#include "crypto.h"

int luaopen_llib(lua_State* L) { 
    lua_newtable(L);
    
    //create <lib>.array functions
    lua_newtable(L);
    luaL_register(L, NULL, array_function_list);
    lua_setfield(L, 2, "array");
   
    lua_newtable(L);
    luaL_register(L, NULL, crypto_function_list);
    lua_setfield(L, 2, "crypto");
    //make llib global
    lua_setglobal(L, "llib");
    return 1;
}
