/*
local function to_base10(inp,basein)
  local out = 0
  for i=1,#inp do
    out = out + (inp[i]*(basein^(#inp - i)))
  end
  return out
end

local function from_base10(inp,outbase)
  local out = {}
  while inp > 0 do
    local rem = inp % outbase
    inp = math.floor(inp / outbase)
    table.insert(out,1,rem)
  end
  return out
end
local function baseT_to_N(inp,basein,baseout)
  return from_base10(to_base10(inp,basein),baseout)
end
*/
#include <math.h>
#include <stdint.h>
#include "../lua.h"
#include "../table.h"

int l_baseconvert(lua_State* L){  
  luaL_checktype(L, 1, LUA_TTABLE);
  int64_t base_import = luaL_checkinteger(L, 2);  
  int64_t base_export = luaL_checkinteger(L, 3);  
  size_t len = lua_objlen(L,1);
  uint64_t out = 0;

  for(size_t i = 1; i <= len; i++){
    lua_pushnumber(L, i);
    lua_gettable(L, 1);

    out += luaL_checkinteger(L, -1) * pow(base_import, len - i); 
  }

  lua_newtable(L);
  for(int i = 1; out > 0; i++){
    uint64_t rem = out % base_export;
    out /= base_export;
        
    lua_pushnumber(L,i);
    lua_pushnumber(L, rem);
    lua_settable(L, -3);
  }

  return 1;
};
