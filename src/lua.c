#include "lua.h"
#include <stdio.h>
#include "io.h"
#include <stdlib.h>
#include <string.h>
#include "i_str.h"
#include "parray.h"

static int ii = 0;
void i_dcopy(lua_State* src, lua_State* dest, void* _seen){
    parray_t* seen = (parray_t*)_seen;
    if(seen == NULL) seen = parray_init();
    size_t len;
    int at, at2;
    int *sp = malloc(1);
    char* s;
    void* whar;
    int old_top = lua_gettop(src);
    //printf("%i\n",ii++);
    switch(lua_type(src, -1)){
        case LUA_TNUMBER:
            lua_pushnumber(dest, luaL_checknumber(src, -1));
            break;
        case LUA_TSTRING:
            s = (char*)luaL_checklstring(src, -1, &len);
            lua_pushlstring(dest, s, len);
            break;
        case LUA_TTABLE:
            lua_newtable(dest);
            at = lua_gettop(dest);
            at2 = lua_gettop(src);

            *sp = at2;
            whar = parray_get(seen, (void*)lua_topointer(src, at2));
            if( whar != NULL){
                //printf("%s\n",lua_tostring(src, at2));
                //printf("WHAR\n");
                    
                lua_pushvalue(dest, *(int*)whar);
                return;
            } else parray_set(seen, (void*)lua_topointer(src, at2), sp);

            lua_pushnil(src);
            for(;lua_next(src, at2) != 0;){
                lua_pushvalue(src, -2);
                
                i_dcopy(src, dest, seen);

                lua_pushvalue(src, -2);
                i_dcopy(src, dest, seen);
                
                lua_settable(dest, at);
                lua_pop(src, 3);
            }
            lua_settop(dest, at);
            break;
        case LUA_TFUNCTION:
            //lua_pushnil(dest);
            //break;
            if(lua_iscfunction(src, old_top)){
                //kinda silly
                lua_pushcfunction(dest, lua_tocfunction(src, -1));
                break;
            }
            
            lua_getglobal(src, "string");
            lua_pushstring(src, "dump");
            lua_gettable(src, -2);
            lua_pushvalue(src, old_top);
            lua_call(src, 1, 1);

            s = (char*)luaL_checklstring(src, -1, &len);
            lua_pushlstring(dest, s, len);
            //for(int i = 0; i != len; i++) printf("%c",s[i]);
            printf("%i\n",luaL_loadbuffer(dest, s, len, "test"));
            //lua_pushvalue(dest, -1);
            break;
        case LUA_TUSERDATA:
            lua_pushlightuserdata(dest, lua_touserdata(src, -1));
            break;
        default:
            printf("%i\n",lua_type(src, -1));
            lua_pushnil(dest);
            break;
    }
    //lua_settop(src, old_top);
}