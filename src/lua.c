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
void luaI_deepcopy(lua_State* src, lua_State* dest, enum deep_copy_flags flags){
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
            n = lua_tonumber(src, -1);
            if(n == (int)n) lua_pushinteger(dest, (int)n);
            else lua_pushnumber(dest, n);
            break;
        case LUA_TSTRING:;
            size_t slen;
            const char* ss = lua_tolstring(src, -1, &slen);
            lua_pushlstring(dest, ss, slen);
            break;
        case LUA_TTABLE:
            modi = 1;
            lua_newtable(dest);
            at = lua_gettop(dest);
            at2 = lua_gettop(src);
            //printf("before\n"); 
            char* aauwu = calloc(sizeof * aauwu, 50);
            sprintf(aauwu, "%p", lua_topointer(src, at2));
            //lua_pushstring(dest, aauwu);
            //lua_gettable(dest, LUA_REGISTRYINDEX);
            lua_getfield(dest, LUA_REGISTRYINDEX, aauwu);
            if(lua_type(dest, -1) == LUA_TNIL){
              //printf("new %p\n", lua_topointer(src, at2));
              lua_pushstring(dest, aauwu);
              lua_pushvalue(dest, at);
              lua_settable(dest, LUA_REGISTRYINDEX);
              lua_pop(dest, 1);
            } else {
              //printf("use %p\n", lua_topointer(src, at2));
              //lua_pop(dest, 1);
              lua_remove(dest, -2);
              return;
            }
            free(aauwu);
            //printf("after %i:%i\n", at, lua_gettop(dest));

            lua_pushnil(src);
            for(;lua_next(src, at2) != 0;){
                int first, second = first = lua_gettop(src);
                first -= 1;
                if((flags & SKIP_GC) && lua_type(src, first) == LUA_TSTRING 
                  && strcmp("__gc", lua_tostring(src, first)) == 0){
                  //printf("found %s\n", lua_tostring(src, first));
                  lua_pop(src, 1);
                  continue;
                }
                lua_pushvalue(src, first);
                //l_pprint(src);
                //lua_pop(src, 1);
                luaI_deepcopy(src, dest, flags);
         
                lua_pushvalue(src, second);
                luaI_deepcopy(src, dest, flags);
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

          luaL_loadbuffer(dest, awa->c, awa->len, awa->c);
          //lua_remove(dest, -2);
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
          lua_pushnumber(dest, 5);
          break;
    }
    int tidx = lua_gettop(dest);
    int aa = lua_gettop(src);

    if(modi && !(flags & SKIP_META) && lua_getmetatable(src, -1)){
      luaI_deepcopy(src, dest, flags | IS_META | SKIP_META);
      lua_setmetatable(dest, tidx);

      lua_settop(dest, tidx);
    }
    lua_settop(src, old_top);
}

void luaI_deepcopy2(lua_State* src, lua_State* dest){
  switch(lua_type(src, -1)){
    case LUA_TNUMBER:
      lua_pushinteger(dest, lua_tointeger(src, -1));
      break;

    case LUA_TSTRING:;
      size_t size = 0;
      const char* str = lua_tolstring(src, -1, &size);
      lua_pushlstring(dest, str, size);
      break;
    
    case LUA_TTABLE:;
      const void* p = lua_topointer(src, -1);
      char* p_string = calloc(80, sizeof * p_string);
      sprintf(p_string, "%p", p);
      
      //lua_getfield(dest, LUA_REGISTRYINDEX, p_string);
      lua_pushstring(dest, p_string);
      lua_gettable(dest, LUA_REGISTRYINDEX);
      if(!lua_isnil(dest, -1)){
        break;
      }

      lua_pop(dest, 1);
      lua_pushstring(dest, p_string);
      lua_newtable(dest);
      //lua_setfield(dest, LUA_REGISTRYINDEX, p_string);
      //lua_getfield(dest, LUA_REGISTRYINDEX, p_string);
      lua_settable(dest, LUA_REGISTRYINDEX);

      lua_pushstring(dest, p_string);
      lua_gettable(dest, LUA_REGISTRYINDEX);

      free(p_string);

      int src_top = lua_gettop(src);
      int dest_top = lua_gettop(dest);

      lua_pushnil(src);
      for(;lua_next(src, src_top) != 0;){
        luaI_deepcopy2(src, dest); 
        lua_pop(src, 1);
        luaI_deepcopy2(src, dest);

        lua_settable(dest, dest_top);
      }
      break;

    default:
      lua_pushinteger(dest, 4);
      break;
  }
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

