#include "common.h"

int l_write(lua_State* L);
int l_send(lua_State* L);
int l_close(lua_State* L);
int l_roll(lua_State* L);
#define bsize 512
int l_sendfile(lua_State* L);