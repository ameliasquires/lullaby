#include "lua.h"
#include <stdio.h>
#include "io.h"
#include <stdlib.h>
#include <string.h>
#include "types/str.h"
#include <stdint.h>
#include "types/parray.h"

static int malloc_count = 0;

int luaI_nothing(lua_State* L){
  return 0;
}

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

int _stream_read(lua_State* L){
  uint64_t len = 0;
  if(lua_gettop(L) > 1){
    len = lua_tointeger(L, 2);
  }

  lua_pushstring(L, "_read");
  lua_gettable(L, 1);
  stream_read_function rf = lua_touserdata(L, -1);

  lua_pushstring(L, "_state");
  lua_gettable(L, 1);
  void* state = lua_touserdata(L, -1);

  str* cont = str_init("");
  int ret = rf(len, &cont, &state);

  if(ret < 0){
    luaI_error(L, ret, "read error");
  }

  if(ret == 0){
    luaI_tsetb(L, 1, "more", 0);
  }
  
  lua_pushlstring(L, cont->c, cont->len);
  free(cont);
  return 1;
}

int _stream_file(lua_State* L){
  const int CHUNK_SIZE = 4096;
  uint64_t maxlen = 0;
  uint64_t totallen = 0;
  const char* mode = "w";
  if(lua_gettop(L) > 2){
    maxlen = lua_tointeger(L, 3);
  }

  if(lua_gettop(L) > 3){
    mode = lua_tostring(L, 4);
  }

  lua_pushstring(L, "_read");
  lua_gettable(L, 1);
  stream_read_function rf = lua_touserdata(L, -1);

  lua_pushstring(L, "_state");
  lua_gettable(L, 1);
  void* state = lua_touserdata(L, -1);

  const char* filename = lua_tostring(L, 2);
  FILE *f;
  f = fopen(filename, mode);
  if(f == NULL){
    luaI_error(L, -1, "unable to open file");
  }

  str* cont = str_init("");
  for(;;){
    int ret = rf(CHUNK_SIZE, &cont, &state);
    //printf("%s\n", cont->c);

    if(ret < 0){
      fclose(f);
      luaI_error(L, ret, "read error"); 
    }

    fwrite(cont->c, sizeof * cont->c, cont->len, f);
    totallen += cont->len;
    str_clear(cont);

    if(ret == 0 || totallen >= maxlen){
      if(ret == 0) {luaI_tsetb(L, 1, "more", 0);}
      break;
    }
  }

  fclose(f); 
  return 0;
}

int _stream_free(lua_State* L){
  lua_pushstring(L, "_free");
  lua_gettable(L, 1);
  void* rf = lua_touserdata(L, -1);

  lua_pushstring(L, "_state");
  lua_gettable(L, 1);
  void* state = lua_touserdata(L, -1);

  if(rf != NULL){
    ((stream_free_function)rf)(&state);
  }
  return 0;
}

void luaI_newstream(lua_State* L, stream_read_function readf, stream_free_function freef, void* state){
  lua_newtable(L);
  int tidx = lua_gettop(L);

  luaI_tsetlud(L, tidx, "_read", readf);
  luaI_tsetlud(L, tidx, "_free", freef);
  luaI_tsetlud(L, tidx, "_state", state);
  luaI_tsetcf(L, tidx, "read", _stream_read); 
  luaI_tsetcf(L, tidx, "close", _stream_free); 
  luaI_tsetb(L, tidx, "more", 1);
  luaI_tsetcf(L, tidx, "file", _stream_file);
  
  lua_newtable(L);
  int midx = lua_gettop(L);

  luaI_tsetcf(L, midx, "__gc", _stream_free);

  lua_pushvalue(L, midx);
  lua_setmetatable(L, tidx);

  lua_pushvalue(L, tidx);
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
    //printf("%i\n",seen->len);
    int at, at2;
    //int *sp = malloc(1);
    //int *sp;
    double n;
    int old_top = lua_gettop(src);
    int modi = 0;
    
    int type;
    switch(type = lua_type(src, -1)){
        case LUA_TNUMBER:
            n = lua_tonumber(src, -1);
            if(n == (uint64_t)n) lua_pushinteger(dest, lua_tonumber(src, -1));
            else lua_pushnumber(dest, n);
            break;
        case LUA_TBOOLEAN:
            lua_pushboolean(dest, lua_toboolean(src, -1));
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
              free(aauwu);
              return;
            }
            free(aauwu);
            //printf("after %i:%i\n", at, lua_gettop(dest));

            lua_pushnil(src);
            for(;lua_next(src, at2) != 0;){
                int first, second = first = lua_gettop(src);
                first -= 1;
                //this is a mess, skip if key is __gc (when SKIP_GC)
                //and skip _G (when SKIP__G)
                if(((flags & SKIP__G) && lua_type(src, first) == LUA_TSTRING
                  && strcmp("_G", lua_tostring(src, first)) == 0)
                  || ((flags & SKIP_GC) && lua_type(src, first) == LUA_TSTRING 
                  && strcmp("__gc", lua_tostring(src, first)) == 0)){
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

          luaL_loadbuffer(dest, awa->c, awa->len, "fun");
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
        case LUA_TTHREAD:
          lua_pushnil(dest);
          break; 
        default:
          printf("unknown type %i vs (old)%i\n",lua_type(src, -1), type);
          //abort();
          lua_pushnil(dest);
          break;
    }
    int tidx = lua_gettop(dest);

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

int env_table(lua_State* L, int provide_table){
  if(provide_table == 0){
    lua_newtable(L);
  }
  int tidx = lua_gettop(L);
 
  lua_Debug debug;
  for(int i = 0;; i++){
    if(lua_getstack(L, i, &debug) == 0) break;
    for(int idx = 1;; idx++){
      const char* name = lua_getlocal(L, &debug, idx);
      int val = lua_gettop(L);
      if(name == NULL) break;

      lua_pushstring(L, name);
      lua_gettable(L, tidx);
      //all temporary (non-local variables) should start with '('
      if(!lua_isnil(L, -1) || name[0] == '('){
        lua_pop(L, 2);
        continue;
      }

      luaI_tsetv(L, tidx, name, val);
      lua_pop(L, 2);
    }
  }

  //luaI_tseti(L, tidx, "hii", 234);

  return 1;
}

//main is the default values, merge is the new and overridden ones
void luaI_jointable(lua_State* L, int main, int merge){
  int idx = lua_gettop(L);

  lua_pushvalue(L, merge);

  lua_pushnil(L);
  for(;lua_next(L, -2) != 0;){
    lua_pushvalue(L, -2);
    lua_pushvalue(L, -2);
    lua_settable(L, main);
    lua_pop(L, 1);
  }

  lua_settop(L, idx);
}

//copys all variables from state A to B, including locals (stored in _locals)
//populates _ENV the same as _G
void luaI_copyvars(lua_State* from, lua_State* to){
  lua_getglobal(from, "_G");
  luaI_deepcopy(from, to, SKIP_GC | SKIP__G);
  lua_set_global_table(to);

  env_table(from, 0);
  luaI_deepcopy(from, to, SKIP_GC);
  int idx = lua_gettop(to);
  lua_pushglobaltable(to);
  int tidx = lua_gettop(to);

  luaI_tsetv(to, idx, "_ENV", tidx);

  luaI_tsetv(to, tidx, "_locals", idx);
}

/**
 * @brief extracts a table to be global
 *
 * @param {lua_State*} source
*/
void lua_set_global_table(lua_State* L){
  lua_pushnil(L);
  for(;lua_next(L, -2) != 0;){
    if(lua_type(L, -2) != LUA_TSTRING){
      lua_pop(L, 1);
      continue;
    }

    //lua_pushstring(L, lua_tostring(L, -2));
    lua_setglobal(L, lua_tostring(L, -2));
  }
}

//returns a table where index is the name at that index
void lua_upvalue_key_table(lua_State* L, int fidx){
  lua_newtable(L);
  int tidx = lua_gettop(L);
  char* name;

  for(int i = 1; (name = (char*)lua_getupvalue(L, fidx, i)) != NULL; i++){
    lua_pushinteger(L, lua_rawlen(L, tidx) + 1);
    lua_pushstring(L, name);
    lua_settable(L, tidx);
  }

  lua_pushvalue(L, tidx);
}

//sets each upvalue where the name exists in _locals table.
//if function was dumped it wont work if debug values are stripped
int lua_assign_upvalues(lua_State* L, int fidx){
  lua_getglobal(L, "_locals");
  int lidx = lua_gettop(L);

  lua_upvalue_key_table(L, fidx);

  lua_pushnil(L);
  for(;lua_next(L, -2) != 0;){
    lua_gettable(L, lidx);
    if(lua_isnil(L, -1)){
      lua_pop(L, 1);
    }
    lua_setupvalue(L, fidx, lua_tointeger(L, -2));
  }

  lua_pushvalue(L, fidx);

  return 0;
}
