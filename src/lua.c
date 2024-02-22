#include "lua.h"
#include <stdio.h>
#include "io.h"
#include <stdlib.h>
#include <string.h>
#include "types/str.h"
#include "types/parray.h"

static int ii = 0;
static int malloc_count = 0;

void* __malloc_(size_t N){
    printf("hi");
    malloc_count++;
    return (malloc)(N);
}

void __free_(void* p){
    malloc_count--;
    printf("%i\n",malloc_count);
    return (free)(p);
}

int writer(lua_State *L, const void* p, size_t sz, void* ud){
    char o[2] = {0};
    for (int i =0; i<sz; i++){
        //printf("%c\n",((char*)p)[i]);
        o[0] = ((char*)p)[i];
        str_pushl((str*)ud, o, 1);
        //printf("%s\n",((str*)ud)->c);
    }
    
    return 0;
}
void i_dcopy(lua_State* src, lua_State* dest, void* _seen){
    parray_t* seen = (parray_t*)_seen;
    int wnull = seen == NULL;
    if(wnull) seen = parray_init();
    size_t len;
    //printf("%i\n",seen->len);
    int at, at2;
    //int *sp = malloc(1);
    //int *sp;
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
            char aauwu[50] = {0};
            sprintf(aauwu, "%p", lua_topointer(src, at2));

            int* sp = malloc(1);
            whar = parray_get(seen, aauwu);
            if( whar != NULL){
                //printf("%s\n",lua_tostring(src, at2 - 1));
                //printf("WHAR\n");
                lua_pop(dest, 1);
                lua_rawgeti(dest, LUA_REGISTRYINDEX, *(int*)whar);

                
                return;
            }
            //lua_pushinteger(dest, 55);
            int r = luaL_ref(dest, LUA_REGISTRYINDEX);
            lua_rawgeti(dest, LUA_REGISTRYINDEX, r);
            *sp = r;
            parray_set(seen, aauwu, sp);
            //printf("saved %i\n", *sp);

            //for(int i = 0; i != seen->len; i++){
            //    printf("%i ", *(int*)seen->P[i].value);
            //}
            lua_pushnil(src);
            for(;lua_next(src, at2) != 0;){
                lua_pushvalue(src, -2);
                int a = lua_gettop(src);
                //l_pprint(src);
                lua_settop(src, a);
                i_dcopy(src, dest, seen);

                lua_pushvalue(src, -2);
                i_dcopy(src, dest, seen);
                
                lua_settable(dest, at);
                lua_pop(src, 3);
            }
            //lua_settop(dest, at);
            break;
        case LUA_TFUNCTION:
            if(lua_iscfunction(src, old_top)){
                //kinda silly
                lua_pushcfunction(dest, lua_tocfunction(src, -1));
                break;
            }
            
            str* awa = str_init("");
            lua_dump(src, writer, (void*)awa, 0);
            //l_pprint(src);
            //lua_pcall(src, 1, 1, 0);
            //l_pprint(src);
            //lua_settop(src, oo);
            //lua_pop(src, 6);

            //s = (char*)luaL_checklstring(src, -1, &len);
            lua_pushlstring(dest, awa->c, awa->len);
            //for(int i = 0; i != awa->len; i++) printf("%i : %c\n",i, awa->c[i]);
            luaL_loadbuffer(dest, awa->c, awa->len, awa->c);
            lua_remove(dest, -2);
            str_free(awa);
            //lua_pushvalue(dest, -1);
            break;
        case LUA_TUSERDATA:
            //printf("aww\n");
            //lua_pushnumber(dest, 8);
            lua_pushlightuserdata(dest, lua_touserdata(src, -1));
            break;
        default:
            printf("%i\n",lua_type(src, -1));
            lua_pushnil(dest);
            break;
    }
    if(wnull) parray_clear(seen, 1);
    //lua_settop(src, old_top);
    _seen = seen;
}