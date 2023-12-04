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

  free(out);
  return 1; 
};
