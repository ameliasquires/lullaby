#include "io.h"
#include "stdlib.h"
#include "stdio.h"
#include "string.h"
#include "stdint.h"
#include "unistd.h"

int l_readfile(lua_State* L){
  size_t len;
  char* a = (char*)luaL_checklstring(L, 1, &len);

  FILE *fp;
  const uint64_t chunk_iter = 512;
  uint64_t chunk = 512;
  uint64_t count = 0;
  char* out = malloc(chunk);
  
  fp = fopen(a, "r");
  
  for(;;){
    char ch = fgetc(fp);   
    if(ch==EOF) break;
    
    if(count > chunk){
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

