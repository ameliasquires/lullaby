#include "lua.h"
#include <stdio.h>
#include "io.h"
#include <stdlib.h>
#include <string.h>
#include "types/str.h"
#include <stdint.h>
#include "util.h"
#include "types/parray.h"

static int malloc_count = 0;

int luaI_nothing(lua_State* L){
  return 0;
}

void luaI_fromparray(lua_State* L, int table_idx, parray_t* table, int strval){
  for(int i = 0; i != table->len; i++){
    if(table->P[i].value == NULL) lua_pushboolean(L, 1);
    else if(strval) lua_pushlstring(L, ((str*)table->P[i].value)->c, ((str*)table->P[i].value)->len);
    else lua_pushstring(L, (char*)table->P[i].value);

    lua_setfield(L, table_idx, table->P[i].key->c);
  }
}

int luaI_lowercase_index(lua_State* L){
  if(lua_type(L, 2) == LUA_TSTRING){
    str* s = str_init(lua_tostring(L, 2));
    str_lowercase(s);
    lua_pushlstring(L, s->c, s->len);
    str_free(s);
  }
  lua_rawget(L, 1);

  return 1;
}

int luaI_lowercase_newindex(lua_State* L){
  if(lua_type(L, 2) == LUA_TSTRING){
    str* s = str_init(lua_tostring(L, 2));
    str_lowercase(s);
    lua_pushlstring(L, s->c, s->len);
    str_free(s);
    lua_pushvalue(L, 3);
  }

  lua_rawset(L, 1);
  return 0;
}


int luaI_iserror(lua_State* L){
  if(lua_gettop(L) < 3) return 0;
  return lua_type(L, -3) == LUA_TNIL &&
          lua_type(L, -2) == LUA_TSTRING &&
          lua_type(L, -1) == LUA_TNUMBER;
}

int _stream_read(lua_State* L){
  uint64_t len = 0;
  if(lua_gettop(L) > 1){
    len = lua_tointeger(L, 2);
  }

  lua_getfield(L, 1, "_read");
  stream_read_function rf = lua_touserdata(L, -1);

  lua_getfield(L, 1, "_state");
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
  str_free(cont);
  return 1;
}

int _stream_load(lua_State* L){
  _stream_read(L);
  if(luaI_iserror(L)) return 3;
  int idx = lua_gettop(L);
  luaI_tsetv(L, 1, "txt", idx);

  return 0;
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

  lua_getfield(L, 1, "_read");
  stream_read_function rf = lua_touserdata(L, -1);

  lua_getfield(L, 1, "_state");
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
      str_free(cont);
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
  str_free(cont);

  fclose(f); 
  return 0;
}

int _stream_free(lua_State* L){
  lua_getfield(L, 1, "_free");
  void* rf = lua_touserdata(L, -1);

  lua_getfield(L, 1, "_state");
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
  luaI_tsetcf(L, tidx, "load", _stream_load); 
  luaI_tsetcf(L, tidx, "close", _stream_free); 
  luaI_tsetb(L, tidx, "more", 1);
  luaI_tsetcf(L, tidx, "file", _stream_file);
  luaI_tsets(L, tidx, "txt", "")

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

enum table_cache table_cache(lua_State* L, char* key, int index){
  lua_getfield(L, LUA_REGISTRYINDEX, key);
  if(lua_type(L, -1) == LUA_TNIL){
    lua_pushstring(L, key);
    lua_pushvalue(L, index);
    lua_settable(L, LUA_REGISTRYINDEX);
    lua_pop(L, 1);
    return CACHE_MISS;
  } else {
    //lua_pop(dest, 1);
    lua_remove(L, -2);
    return CACHE_HIT;
   }
}

/**
 * @brief copy top element from src to dest
 *
 * @param {lua_State*} source
 * @param {lua_State*} dest
 * @param {void*} items already visited, use NULL
 * @param {int} whether or not to skip meta data
 */
int luaI_deepcopy(lua_State* src, lua_State* dest, enum deep_copy_flags flags){
#warning "ensure enough space, cheap fix, should rewrite this function"
  luaI_assert2(src, lua_checkstack(src, 10));
  luaI_assert2(src, lua_checkstack(dest, 10));

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
      if(n == (uint64_t)n) lua_pushinteger(dest, (uint64_t)lua_tonumber(src, -1));
      else lua_pushnumber(dest, n);
      break;
    case LUA_TBOOLEAN:
      lua_pushboolean(dest, lua_toboolean(src, -1));
      break;
    case LUA_TNIL:
      lua_pushnil(dest);
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
                     sprintf(aauwu, "tab-%p", lua_topointer(src, at2));

                     if(table_cache(dest, aauwu, at) == CACHE_HIT){
                       free(aauwu);
                       return 0;
                     }
                     
                     free(aauwu);

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
                     int f = lua_gettop(dest);
                     //if(!(flags & SKIP_LOCALS)) lua_assign_upvalues(dest, lua_gettop(dest));
                     char* poi = calloc(sizeof * poi, 50);
                     sprintf(poi, "fun-%p", lua_topointer(src, old_top));

                     if(table_cache(dest, poi, f) == CACHE_HIT){
                       free(poi);
                       str_free(awa);
                       return 0;
                     }
                     free(poi);

                     
                     for(int i = 1; lua_getupvalue(src, -1, i) != NULL; i++){
                       luaI_deepcopy(src, dest, flags | IS_UPVALUE);
                       
                       lua_setupvalue(dest, f, i);
                       lua_pop(src, 1);
                     }

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
                     fprintf(stderr, "unable to copy LUA_TTHREAD, pushing nil\n");
                     lua_pushnil(dest);
                     break; 
    default:
                     fprintf(stderr, "unknown type %i vs (old)%i\n",lua_type(src, -1), type);
                     //abort();
                     lua_pushnil(dest);
                     break;
  }
  int tidx = lua_gettop(dest);

  if(modi && !(flags & SKIP_META) && lua_getmetatable(src, -1)){
    luaI_deepcopy(src, dest, flags | IS_META);
    lua_setmetatable(dest, tidx);

    lua_settop(dest, tidx);

    if(flags & STRIP_GC){
      int sidx = lua_gettop(src);
      lua_getmetatable(src, sidx);
      luaI_tsetnil(src, sidx, "__gc");
    }
  }
  lua_settop(src, old_top);
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

//top table is prioritized
void luaI_jointable(lua_State* L){
  int idx = lua_gettop(L) - 1;

  lua_pushnil(L);
  for(;lua_next(L, -2) != 0;){
    lua_pushvalue(L, -2);
    lua_pushvalue(L, -2);
    lua_settable(L, idx);
    lua_pop(L, 1);
  }

  lua_pushvalue(L, idx);
}

//copys all variables from state A to B, including locals (stored in _locals)
//populates _ENV the same as _G
void luaI_copyvars(lua_State* from, lua_State* to){
  lua_getglobal(from, "_locals");
  int x = lua_gettop(from);

  if(lua_isnil(from, x)){
    lua_pop(from, 1);
    x = 0;
  }

  env_table(from, x != 0);
  luaI_deepcopy(from, to, SKIP_GC | SKIP_LOCALS);
  lua_pop(from, 1);
  int idx = lua_gettop(to);
  lua_pushglobaltable(to);
  int tidx = lua_gettop(to);

  luaI_tsetv(to, idx, "_ENV", tidx);
  luaI_tsetv(to, tidx, "_locals", idx);

  lua_getglobal(from, "_G");
  luaI_deepcopy(from, to, SKIP_GC | SKIP__G);
  lua_set_global_table(to);

  lua_pushvalue(to, idx);
  lua_setglobal(to, "_locals");
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

  lua_settop(L, fidx);

  return 0;
}

int luaI_errtraceback(lua_State* L){
  luaL_traceback(L, L, lua_tostring(L, -1), 1);
  return 1;
}

