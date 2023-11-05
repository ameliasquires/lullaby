#include "../crypto.h"
#include <stdio.h>
#include <stdint.h>


uint16_t i_bsdchecksum(uint8_t *aa, size_t len){
    uint16_t check = 0x0;
    
    for(int i = 0; i != len; i++){
        uint8_t a = aa[i];
        check = (check >> 1) + ((check & 1) << 15);
        check += a;
        check &= 0xffff;
    }
    return check;
}

int l_bsdchecksum(lua_State* L){
  
  size_t len = 0;
  uint8_t* a = (uint8_t*)luaL_checklstring(L, 1, &len);
  
  char digest[16];

  uint16_t u = i_bsdchecksum(a, len);
  sprintf(digest,"%i",u);
  lua_pushstring(L, digest);

  return 1;
}
