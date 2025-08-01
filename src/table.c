#include "table.h"
#include <stdlib.h>
#define _GNU_SOURCE
#include <string.h>
#include <stdint.h>

int l_split(lua_State* L){
  size_t str_len = 0;
  size_t search_len = 0;

  const char* str = luaL_checklstring(L, 1, &str_len);
  const char* search = luaL_checklstring(L, 2, &search_len);
  int skip = 1;
  if(lua_gettop(L) >= 3) skip = lua_toboolean(L, 3); 

  lua_newtable(L);
  int idx = lua_gettop(L);

  const char* last = str;

  for(;;){
    char* end = (char*)memmem(last, str_len - (last - str), search, search_len);
    if(end == NULL) break;

    if(!(skip && (end - last) == 0)){
      lua_pushinteger(L, lua_objlen(L, idx) + 1);
      lua_pushlstring(L, last, end - last);
      lua_settable(L, idx);
    }

    last = end + 1;
  }

  lua_pushinteger(L, lua_objlen(L, idx) + 1);
  lua_pushlstring(L, last, str_len - (last - str));
  lua_settable(L, idx);

  lua_pushvalue(L, idx);
  return 1;
}

int l_unpack(lua_State* L){
  int top = lua_gettop(L);
  lua_pushnil(L);
  
  for(;lua_next(L, top);){
    lua_pushvalue(L, -2);
    lua_remove(L, -3);
  }

  return lua_gettop(L) - top;
}

uint64_t i_len(lua_State* L, int pos){
  uint64_t i = 0;
  lua_pushnil(L);
  for(;lua_next(L,pos) != 0;){
    i += 1;
    lua_pop(L,1);
  }
  return i;
}
int l_len(lua_State* L) {
    luaL_checktype(L, 1, LUA_TTABLE);    
    lua_pushnumber(L,i_len(L,1));
    return 1;
}

int l_reverse(lua_State* L) {
    luaL_checktype(L, 1, LUA_TTABLE);
    
    size_t len = lua_objlen(L,1);
    lua_newtable(L);
    for(size_t i = 0; i <= len - 1; i++){
      lua_pushnumber(L,len - i - 1);
      lua_gettable(L,1);

      lua_pushnumber(L, i+1);
      lua_pushvalue(L, -2);

      lua_settable(L,2); 
    }

    lua_pushvalue(L, 2);
    return 1;
}

int l_greatest(lua_State* L) {
    luaL_checktype(L, 1, LUA_TTABLE);
    size_t len = lua_objlen(L,1);
    int touched = 0;
    double cur = 0;

    for(size_t i = 0; i <= len-1; i++){
      lua_pushinteger(L,i+1);
      lua_gettable(L,1);
       
      double t = luaL_checknumber(L, -1);
      if(!touched || t > cur) cur = t;
      touched = 1; 
      lua_pop(L,1);
    }
   
    lua_pushnumber(L, cur);
    return 1;
}

int l_least(lua_State* L) {
    luaL_checktype(L, 1, LUA_TTABLE);
    size_t len = lua_objlen(L,1);
    int touched = 0;
    double cur = 0;

    for(size_t i = 0; i <= len-1; i++){
      lua_pushinteger(L,i+1);
      lua_gettable(L,1);
       
      double t = luaL_checknumber(L, -1);
      if(!touched || t < cur) cur = t;
      touched = 1;
      lua_pop(L,1);
    }
   
    lua_pushnumber(L, cur);
    return 1;
}

void i_shuffle(double* arr, size_t len){
    for(size_t i = 0; i < len; i++){
      int i2 = rand()%len;
      int d = rand()%len;
      i_swap(arr[i2], arr[d]);
    }
}

int l_shuffle(lua_State* L) {
    luaL_checktype(L, 1, LUA_TTABLE);
    size_t len = lua_objlen(L,1);
    double* nums = malloc(sizeof * nums * len);
    for(int i = 0; i <= len-1; i++){

      lua_pushinteger(L,i+1);
      lua_gettable(L,1);
       
      nums[i] = luaL_checknumber(L, -1);
      lua_pop(L,1);
    }
  
    i_shuffle(nums, len);

    lua_newtable(L);
    for(size_t i = 0; i != len; i++){
      lua_pushnumber(L,i+1);
      lua_pushnumber(L,nums[i]);
      lua_settable(L, -3);
    }

    free(nums);
    return 1;
}

int l_sum(lua_State* L) {
    luaL_checktype(L, 1, LUA_TTABLE);
    size_t len = lua_objlen(L,1);
    double s = 0;
    for(int i = 0; i <= len-1; i++){

      lua_pushinteger(L,i+1);
      lua_gettable(L,1);
       
      s += luaL_checknumber(L, -1);
      lua_pop(L,1);
    }
  
    lua_pushnumber(L, s);
    return 1;
}

int l_indexof(lua_State* L) {
    int argc = lua_gettop(L);
    luaL_checktype(L, 1, LUA_TTABLE);
    size_t len = lua_objlen(L,1);
   
    //get optional 3rd argument, if its >0 set it to 0
    size_t start = argc == 3 ? luaL_checknumber(L,3) : 0;
    start = start > 0 ? start : start;

    for(size_t i = 0; i <= len-1; i++){
      lua_pushinteger(L,i+1);
      lua_gettable(L,1);
      
      if(lua_rawequal(L, -1, 2)){
          lua_pushnumber(L, i);
          return 1;
      }
      lua_pop(L,1);
    }
   
    lua_pushnumber(L, -1);
    return 1;
}

int l_sindexof(lua_State* L) {
    double target = luaL_checknumber(L, 2);
    luaL_checktype(L, 1, LUA_TTABLE);
    size_t len = lua_objlen(L,1);
    int l = 0;
    int r = len - 1;

    for(; l<=r;){
      int m = l + (r - l) /2;
      lua_pushinteger(L,m+1);
      lua_gettable(L,1);
       
      double t = luaL_checknumber(L, -1);
      if(t==target){
          lua_pushnumber(L, m);
          return 1;
      }
      if(t > target) l = m + 1;
      else r = m - 1;
      
      lua_pop(L,1);
    }
   
    lua_pushnumber(L, -1);
    return 1;
}

/*int l_split(lua_State* L){
  size_t input_len = 0;
  size_t split_len = 0;
  char* input = (char*)luaL_checklstring(L, 1, &input_len);
  char* split = (char*)luaL_checklstring(L, 2, &split_len);
  size_t table_len = 1;
  lua_newtable(L);

  size_t current_len = 0;
  char current[input_len]; 
  memset(current, 0, input_len);

  for(size_t i = 0; i <= (input_len - 1) - (split_len - 1); i++){
    int match = 1;
    for(size_t z = 0; z <= split_len - 1 && match; z++){
      if(split[z] != input[z + i]) match = 0;
    }
    if(match){
      lua_pushnumber(L, table_len++);
      lua_pushstring(L, current);
      lua_settable(L, -3);

      memset(current, 0, input_len);
      current_len = 0;
    } else {
      current[current_len] = input[i];
      current_len++;
    }
  }
  lua_pushnumber(L, table_len++);
  lua_pushstring(L, current);
  lua_settable(L, -3);

  return 1;
}*/

int l_to_char_array(lua_State* L){
  size_t input_len = 0;
  char* input = (char*)luaL_checklstring(L, 1, &input_len);
  lua_newtable(L);
  
  for(size_t i = 0; i <= input_len - 1; i++){
    lua_pushnumber(L, i + 1); 
    char uwu = input[i];
    lua_pushstring(L, &uwu);
    lua_settable(L, -3);
  }
  return 1;
}
