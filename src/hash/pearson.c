#include "../util.h"
#include "../crypto.h"
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>

static uint8_t pearson_table[256] = {0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,
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

struct pearson_hash pearson_init(){
  return (struct pearson_hash){.ret = 0};
}

void pearson_update(uint8_t* aa, size_t len, struct pearson_hash* hash){
  for(int i = 0; i != len; i++)
    hash->ret = pearson_table[(uint8_t)(hash->ret^aa[i])];
}

uint8_t pearson_final(struct pearson_hash* hash){
  return hash->ret;
}

uint8_t pearson(uint8_t* aa, size_t len){
  struct pearson_hash a = pearson_init();
  pearson_update(aa, len, &a);
  return pearson_final(&a);
}

int l_setpearson(lua_State* L){
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
       
    pearson_table[i] = luaL_checknumber(L, -1);
    lua_pop(L,1);
  }
  return 0;
}

common_hash_init_update(pearson);

int l_pearson_final(lua_State* L){
  lua_pushstring(L, "ud");
  lua_gettable(L, 1);

  struct pearson_hash* a = (struct pearson_hash*)lua_touserdata(L, -1);
  uint8_t u = pearson_final(a);
  char digest[8];
  sprintf(digest,"%x",u);
  lua_pushstring(L, digest);
  return 1;
}

int l_pearson(lua_State* L){
  if(lua_gettop(L) == 0) return l_pearson_init(L);
  size_t len = 0;
  uint8_t* a = (uint8_t*)luaL_checklstring(L, 1, &len);

  char digest[8];
  uint8_t u = pearson(a, len);
  
  sprintf(digest,"%x",u);

  lua_pushstring(L, digest);

  return 1;
}
