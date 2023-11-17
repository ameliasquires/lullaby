#include "../i_util.h"
#include "../crypto.h"
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

static uint8_t T[256] = {0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,
  15,16,17,18,19,20,21,22,23,24,25,26,27,28,29,30,31,32,33,34,35,36,37,38,
  39,40,41,42,43,44,45,46,47,48,49,50,51,52,53,54,55,56,57,58,59,60,61,62,
  63,64,65,66,67,68,69,70,71,72,73,74,75,76,77,78,79,80,81,82,83,84,85,86,
  87,88,89,90,91,92,93,94,95,96,97,98,99,100,101,102,103,104,105,106,107,
  108,109,110,111,112,113,114,115,116,117,118,119,120,121,122,123,124,125,
  126,127,128,129,130,131,132,133,134,135,136,137,138,139,140,141,142,143,
  144,145,146,147,148,149,150,151,152,153,154,155,156,157,158,159,160,161,
  162,163,164,165,166,167,168,169,170,171,172,173,174,175,176,177,178,179,
  180,181,182,183,184,185,186,187,188,189,190,191,192,193,194,195,196,197,
  198,199,200,201,202,203,204,205,206,207,208,209,210,211,212,213,214,215,
  216,217,218,219,220,221,222,223,224,225,226,227,228,229,230,231,232,233,
  234,235,236,237,238,239,240,241,242,243,244,245,246,247,248,249,250,251,
  252,253,254,255};

uint8_t i_lr8(uint8_t y, uint8_t offset){
    return ( y << offset ) | ( y >> (8 - offset));
}

uint16_t i_lr16(uint16_t y, uint16_t offset){
    return ( y << offset ) | ( y >> (16 - offset));
}

uint8_t i_buzhash8(uint8_t* in, size_t len){
    uint8_t hash = 0;

    for(int i = 0; i != len; i++){
        hash ^= i_lr8(T[(uint8_t)in[i]],len - (i + 1));
    }

    return hash;
}
uint16_t i_buzhash16(uint8_t* in, size_t len){
    uint16_t hash = 0;

    for(int i = 0; i != len; i++){
        hash ^= i_lr16(T[(uint8_t)in[i]],len - (i + 1));
    }

    return hash;
}

int l_setbuzhash(lua_State* L){
  luaL_checktype(L, 1, LUA_TTABLE);
  size_t len = lua_objlen(L,1);
  
  if(len != 256) {
    p_error("new table must have a length of 256");
    exit(0); 
  }

  double s = 0;
  for(int i = 0; i <= len-1; i++){

    lua_pushinteger(L,i+1);
    lua_gettable(L,1);
       
    T[i] = luaL_checknumber(L, -1);
    lua_pop(L,1);
  }
  return 0;
}

int l_buzhash8(lua_State* L){ 
  size_t len = 0;
  uint8_t* a = (uint8_t*)luaL_checklstring(L, 1, &len);

  char digest[3];
  uint8_t u = i_buzhash8(a, len);
  
  sprintf(digest,"%x",u);

  lua_pushstring(L, digest);

  return 1;
}

int l_buzhash16(lua_State* L){ 
  size_t len = 0;
  uint8_t* a = (uint8_t*)luaL_checklstring(L, 1, &len);

  char digest[6];
  uint16_t u = i_buzhash16(a, len);
  
  sprintf(digest,"%04x",u);

  lua_pushstring(L, digest);

  return 1;
}
