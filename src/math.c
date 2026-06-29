/*local function gcd(a,b)
  if b == 0 then return a end
  return gcd(b, math.fmod(a, b))
end

local function lcm(a)
  local out = a[1]

  for i=2,#a do
    out = ((a[i] * out)) / (gcd(a[i], out))
  end
  return out
end
*/
#include "math.h"
#include "stdint.h"
#include <stdlib.h>
#include <threads.h>
#include <pthread.h>

uint64_t gcd(uint64_t a, uint64_t b){
  if(b == 0) return a;
  return gcd(b, a % b);
}

uint64_t lcm(uint64_t* a, size_t len){
  uint64_t out = a[0];
  
  for(size_t i = 1; i != len; i++){
    out = (a[i] * out) / gcd(a[i], out);
  }
  return out;
}

int l_lcm(lua_State* L){
  luaL_checktype(L, 1, LUA_TTABLE);
  size_t len = lua_objlen(L,1); 
  uint64_t* nums = malloc(sizeof * nums * len);
  for(size_t i = 0; i <= len-1; i++){

    lua_pushinteger(L,i+1);
    lua_gettable(L,1);
       
    nums[i] = luaL_checknumber(L, -1);
    lua_pop(L,1);
  }
  lua_pushnumber(L, lcm(nums, len));
  free(nums);
  return 1;
}

thread_local uint16_t seedv[3] = {0, 0, 0};

int l_random(lua_State* L){
  if(seedv[0] + seedv[1] + seedv[2] == 0){
    struct timespec ts;
    clock_gettime(CLOCK_REALTIME, &ts);
    uint64_t seed = (ts.tv_sec << 32) | ts.tv_nsec;

    seedv[0] = seed;
    seedv[1] = seed >> 16;
    seedv[2] = seed >> 32;
  }

  size_t min = 1;
  switch(lua_gettop(L)){
    case 2:
      min = lua_tointeger(L, 1);
      lua_remove(L, 1);
    case 1:
      lua_pushinteger(L, (erand48(seedv) * (lua_tointeger(L, 1) - min + 1)) + min);
      break;
    case 0:
    default:
      lua_pushnumber(L, erand48(seedv));
  }

  return 1;
}
