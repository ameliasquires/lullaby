#include "../crypto.h"
#include <stdio.h>
#include <stdint.h>
#include <math.h>

uint32_t i_sysvchecksum(uint8_t *aa, size_t len){
    uint32_t check = 0x0;
    
    for(int i = 0; i != len; i++){
        check += aa[i];
    }

    uint32_t r = check % (int)pow(2,16) + (check % (int)pow(2,32)) / (int)pow(2,16);
    return (r % (int)pow(2,16)) + r / (int)pow(2,16);
}

int l_sysvchecksum(lua_State* L){
  
  size_t len = 0;
  uint8_t* a = (uint8_t*)luaL_checklstring(L, 1, &len);
  
  char digest[32];

  uint32_t u = i_sysvchecksum(a, len);
  sprintf(digest,"%x",u);
  lua_pushstring(L, digest);

  return 1;
}
