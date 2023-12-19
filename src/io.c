#include <unistd.h>
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

  fp = fopen(a, "r");
  char* out = malloc(chunk);
    
  for(;;){
    char ch = fgetc(fp);
    if(ch==EOF) break;
    
    if(count >= chunk){
      chunk += chunk_iter;
      out = realloc(out, chunk);
    }
    out[count] = ch;
    count++;
  }
  out[count] = '\0';
  lua_pushstring(L, out);
  
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

void i_pprint(lua_State* L, int indent){
  int last_idx = lua_gettop(L);
  const char* type = lua_typename(L,lua_type(L,-1));
  int t = lua_type(L,-1);
  switch(lua_type(L,-1)){
    case LUA_TTABLE:
      print_indentation(indent);
      if(indent >= _max_depth && _max_depth >= 0) {printf("{"color_gray"..."color_reset"}"); break;}
      int skip = i_len(L,last_idx) < _start_nl_at;
      printf("{");
      if(!skip) printf("\n");
      lua_pushnil(L);
      for(;lua_next(L,last_idx) != 0;){
        if(lua_type(L,-2) == LUA_TSTRING){
          print_indentation(indent);
          printf(" %s"color_gray":"color_reset, lua_tostring(L,-2)); 
        }
        i_pprint(L,indent+1);
        printf(",");
        if(!skip) printf("\n");

        lua_pop(L,1);
      }
      
      print_indentation(indent);
      printf("}");
      break;
    case LUA_TSTRING:
      print_indentation(indent);
      printf("\"%s\"", lua_tostring(L,-1));
      break;
    default:
      print_indentation(indent);
      printf(color_yellow"%s"color_reset, lua_tostring(L,-1));
  }

  if(_print_type){
    if(lua_istable(L,last_idx)) printf(color_gray" : <%s:%lu>"color_reset,type,i_len(L,last_idx));
    else printf(color_gray" : <%s>"color_reset,type);
  }

}
int l_pprint(lua_State* L){
  i_pprint(L,0); 
  printf("\n");
  return 0;
}
