#include <unistd.h>
#include "types/str.h"
#include "io.h"
#include "stdlib.h"
#include "stdio.h"
#include "config.h"
#include "stdint.h"
#include "table.h"

int l_readfile(lua_State* L){
  size_t len;
  char* a = (char*)luaL_checklstring(L, 1, &len);

  FILE *fp;
  const uint64_t chunk_iter = _file_malloc_chunk;
  uint64_t chunk = chunk_iter;
  uint64_t count = 0;

  if(access(a, F_OK)) {
    p_fatal("file not found");
  }
  if(access(a, R_OK)){
    p_fatal("missing permissions");
  }

  fp = fopen(a, "rb");
  fseek(fp, 0L, SEEK_END);
  size_t sz = ftell(fp);
  fseek(fp, 0L, SEEK_SET);
  char* out = calloc(sz + 1, sizeof * out);

  fread(out, sizeof * out, sz, fp);

  lua_pushlstring(L, out, sz);
  
  fclose(fp);
  free(out);
  return 1; 
};

lua_Debug i_get_debug(lua_State* L){
  lua_Debug ar;
  lua_getstack(L, 1, &ar);
  lua_getinfo(L, "nSl", &ar);
  return ar;
}

int l_debug(lua_State* L){
  size_t input_len = 0;
  char* input = (char*)luaL_checklstring(L, 1, &input_len);
  lua_Debug debug = i_get_debug(L);
  printf(color_gray"[ debug ] (%s:%i) %s\n"color_reset, debug.source, debug.currentline, input);
  return 0;
}

int l_log(lua_State* L){
  size_t input_len = 0;
  char* input = (char*)luaL_checklstring(L, 1, &input_len);
  lua_Debug debug = i_get_debug(L);
  printf(color_green"[ log ] (%s:%i)"color_gray" %s\n"color_reset, debug.source, debug.currentline, input);
  return 0;
}

int l_warn(lua_State* L){
  size_t input_len = 0;
  char* input = (char*)luaL_checklstring(L, 1, &input_len);
  lua_Debug debug = i_get_debug(L);
  printf(color_yellow"[ warn ] (%s:%i) %s\n"color_reset, debug.source, debug.currentline, input);
  return 0;
}

int l_error(lua_State* L){
  size_t input_len = 0;
  char* input = (char*)luaL_checklstring(L, 1, &input_len);
  lua_Debug debug = i_get_debug(L);
  printf(color_red"[ error ] (%s:%i) %s\n"color_reset, debug.source, debug.currentline, input);
  return 0;
}

void print_indentation(int i){
  for(int z = 0; z < i; z++) printf(" ");
}

void i_pprint(lua_State* L, int indent, int skip_indent){
  int last_idx = lua_gettop(L);
  const char* type = lua_typename(L,lua_type(L,-1));
  int t = lua_type(L,-1);
  switch(lua_type(L,-1)){
    case LUA_TTABLE:
      printf(" ");
      if(indent >= _max_depth && _max_depth >= 0) {
        printf("{"color_gray);
        if(_collapse_to_memory) printf(" %p ",lua_topointer(L, -1));
        else printf(" ... ");
        printf(color_reset"}");
        break;
      }

      int skip = i_len(L,last_idx) < _start_nl_at || _collapse_all;
      printf("{");
      if(!skip) printf("\n");
      lua_pushnil(L);
      for(;lua_next(L,last_idx) != 0;){
        if(lua_type(L,-2) == LUA_TSTRING){
          if(!skip) print_indentation(indent);
          printf(" '%s'"color_gray": "color_reset, lua_tostring(L,-2)); 
        } else {
          if(!skip) print_indentation(indent + 1);
          //printf(" '%i'"color_gray": "color_reset, luaL_checknumber(L,-2)); 
        }
        int tuwu = lua_gettop(L);
        i_pprint(L,indent+1,1);
        printf(",");
        if(!skip) printf("\n");
        
        lua_settop(L, tuwu);
        lua_pop(L,1);
      }
      print_indentation(indent);
      printf("}");
      break;
    case LUA_TSTRING:
      if(!skip_indent) print_indentation(indent);
      size_t len;
      char* wowa = (char*)luaL_tolstring(L, -1, &len);
      printf("\"");
      for(int i = 0; i != len; i++) {
        printf("%c",wowa[i]);
        if(wowa[i] == '\n') print_indentation(indent);
      }
      printf("\"");
      break;
    case LUA_TBOOLEAN:
      if(!skip_indent) print_indentation(indent);
      printf(color_blue"%s"color_reset, lua_toboolean(L,-1)?"true":"false");
      break;
    case LUA_TFUNCTION:
      if(!skip_indent) print_indentation(indent);
      printf(color_yellow"(%p)"color_reset, lua_topointer(L, -1));
      break;
    default:
      if(!skip_indent) print_indentation(indent);
      printf(color_yellow"%s"color_reset, lua_tostring(L,-1));
  }

  if(_print_type){
    if(lua_istable(L,last_idx)) printf(color_gray" : <%s:%lu>"color_reset,type,i_len(L,last_idx));
    else printf(color_gray" : <%s>"color_reset,type);
  }

}

int l_pprint(lua_State* L){
  i_pprint(L,0,0); 
  printf("\n");
  return 0;
}

enum json_state {
  normal, reading
};

#define push() \
  if(is_array){\
    lua_pushinteger(L, iter_count);\
  } else {\
    char kbuf[key->len];\
    memset(kbuf, 0, key->len);\
    strncpy(kbuf, key->c + 1, key->len - 2);\
    lua_pushstring(L, kbuf);\
  }\
  char buf[value->len];\
  memset(buf, 0, value->len);\
  switch(value->c[0]){\
    case '{': case '[':\
      json_parse(L, value);\
      break;\
    case '"':\
      strncpy(buf, value->c + 1, value->len - 2);\
      lua_pushstring(L, buf);\
      break;\
    default:\
      lua_pushnumber(L, atof(value->c));\
      break;\
    }\
    lua_settable(L, last_idx);

void json_parse(lua_State* L, str* raw){
  enum json_state state = normal;
  char topush[2] = {0,0};
  int count = 0;
  char last = '\0';
  int token_depth = 0;

  str* key = str_init("");
  str* value = str_init("");

  lua_newtable(L);
  int last_idx = lua_gettop(L);
  int is_array = raw->c[0] == '[';
  int i = 1;
  int iter_count = 1;
  int escape_next = 0;
  for(i = 1; i != raw->len - 1; i++){  
    topush[0] = *(raw->c + i);
    if(state == normal && (topush[0] == ' ' || topush[0] == '\n' || topush[0] <= 10)) continue;
    if(escape_next > 0) escape_next--;

    switch(topush[0]){
      case '\\':
        escape_next = 2;
        break;
      case '"':
        if(escape_next == 1) break;
      case '{': case '}':
      case '[': case ']':

        if(last == '{' && topush[0] == '}' || last == '[' && topush[0] == ']') token_depth--;

        if((last == '\0' || last == '"' && topush[0] == '"'
            || last == '{' && topush[0] == '}' || last == '[' && topush[0] == ']')){
          if(token_depth == 0){
            if(last == '\0'){
              last = topush[0];
            } else {
              last = '\0';
            }

            if(state==reading) state = normal;
            else state = reading;
          }
        }
        break;
      case ',':
        if(state == normal){
          push()
          str_clear(key);
          str_clear(value);
          count = 0;
          iter_count++;
          continue;
        }
        break;
      case ':':
        if(state == normal){
          count++;
          continue;
        }
        break;
    }
    //if(escape_next == 2) continue;
    if(last == '{' && topush[0] == '{' || last == '[' && topush[0] == '[') token_depth++;

    if(count == 0 && !is_array) str_push(key, topush);
    else str_push(value, topush);
  }
  push()
  str_free(key);
  str_free(value);
  //printf("key: %s, value : %s\n",key.c,value.c);
}

int l_json_parse(lua_State* L){
  str* raw_json = str_init((char*)luaL_checklstring(L, 1, NULL));
  json_parse(L, raw_json);
  str_free(raw_json);
  return 1;
}

int l_arg_handle(lua_State* L){
  luaL_checktype(L, 1, LUA_TTABLE);
  luaL_checktype(L, 2, LUA_TTABLE);
   
  size_t len = lua_objlen(L,1);
  for(size_t i = 0; i <= len - 1; i++){
    lua_pushnumber(L,i + 1);
    lua_gettable(L,1);
    
    lua_pushnumber(L,1);
    lua_gettable(L,-2);
    size_t inner_len = lua_objlen(L,-1);
    size_t inner_idx = lua_gettop(L);

    lua_pushnumber(L, 2);
    lua_gettable(L, -3);

    size_t function_idx = lua_gettop(L);

    for(int ii = 1; ii <= inner_len; ii++){
      lua_pushnumber(L, ii);
      lua_gettable(L, inner_idx);
      
      const char* key = lua_tostring(L, -1);

      size_t input_len = lua_objlen(L, 2);

      for(int iii = 1; iii <= input_len; iii++){
        lua_pushnumber(L, iii);
        lua_gettable(L, 2);
        if(strcmp(lua_tostring(L, -1), key) == 0){
          lua_pushvalue(L, function_idx);
          lua_pcall(L, 0, 0, 0);
          ii = inner_len + 1;
          break;
        }
      }
    
    }
  }
  return 0;
}
