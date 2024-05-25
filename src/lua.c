#include "lua.h"
#include <stdio.h>
#include "io.h"
#include <stdlib.h>
#include <string.h>
#include "lua5.4/lua.h"
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

/**
 * @brief copy top element from src to dest
 *
 * @param {lua_State*} source
 * @param {lua_State*} dest
 * @param {void*} items already visited, use NULL
 * @param {int} whether or not to skip meta data
*/
void luaI_deepcopy(lua_State* src, lua_State* dest, void* _seen, int skip_meta){
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
    double n;
    int old_top = lua_gettop(src);
    int modi = 0;

    switch(lua_type(src, -1)){
        case LUA_TNUMBER:
            n = luaL_checknumber(src, -1);
            if(n == (int)n) lua_pushinteger(dest, (int)n);
            else lua_pushnumber(dest, n);
            break;
        case LUA_TSTRING:
            s = (char*)luaL_checklstring(src, -1, &len);
            lua_pushlstring(dest, s, len);
            break;
        case LUA_TTABLE:
            modi = 1;
            lua_newtable(dest);
            at = lua_gettop(dest);
            at2 = lua_gettop(src);
            char aauwu[50] = {0};
            sprintf(aauwu, "%p", lua_topointer(src, at2));

            whar = parray_get(seen, aauwu);
            if(whar != NULL){
                lua_pop(dest, 1);
                lua_rawgeti(dest, LUA_REGISTRYINDEX, *(int*)whar);
                return;
            }
            int *sp = malloc(sizeof * sp);

            int r = luaL_ref(dest, LUA_REGISTRYINDEX);
            lua_rawgeti(dest, LUA_REGISTRYINDEX, r);
            *sp = r;
            parray_set(seen, aauwu, sp);

            lua_pushnil(src);
            for(;lua_next(src, at2) != 0;){
                int first, second = first = lua_gettop(src);
                first -= 1;
                lua_pushvalue(src, first);
                luaI_deepcopy(src, dest, seen, 0);
         
                lua_pushvalue(src, second);
                luaI_deepcopy(src, dest, seen, 0);
                lua_settable(dest, at);

                lua_pop(src, 3);
            }
            break;
        case LUA_TFUNCTION:
          if(lua_iscfunction(src, old_top)){
            //kinda silly
            lua_pushcfunction(dest, lua_tocfunction(src, -1));
            break;
          }
            
          str* awa = str_init("");
          lua_dump(src, writer, (void*)awa, 0);
          lua_pushlstring(dest, awa->c, awa->len);

          luaL_loadbuffer(dest, awa->c, awa->len, awa->c);
          lua_remove(dest, -2);
          str_free(awa);
          break;
        case LUA_TUSERDATA:
          modi = 1;
          size_t raw_len = lua_rawlen(src, -1);
          void* ud = lua_newuserdata(dest, raw_len);
          memcpy(ud, lua_touserdata(src, -1), raw_len);
          break;
        case LUA_TLIGHTUSERDATA:
          modi = 1;
          lua_pushlightuserdata(dest, lua_touserdata(src, -1)); 
          break;
        default:
          printf("unknown type %i\n",lua_type(src, -1));
          lua_pushnil(dest);
          break;
    }
    int tidx = lua_gettop(dest);
    int aa = lua_gettop(src);

    if(modi&&!skip_meta&&lua_getmetatable(src, -1)){
      luaI_deepcopy(src, dest, seen, 1);
      lua_setmetatable(dest, tidx);

      lua_settop(dest, tidx);
    }
    lua_settop(src, aa);
    
    if(wnull) parray_clear(seen, FREE);
    _seen = seen;
}

/**
 * @brief extracts a table to be global
 *
 * @param {lua_State*} source
*/
void lua_set_global_table(lua_State* L){
  lua_pushnil(L);
  for(;lua_next(L, -2) != 0;){
    lua_setglobal(L, lua_tostring(L, -2));
  }
}

