#include "lua.h"
#include "config.h"

int l_async(lua_State*);
int l_buffer(lua_State*);
int l_testcopy(lua_State*);
int l_mutex(lua_State*);
int l_usleep(lua_State*);
int l_sleep(lua_State*);

void lib_thread_clean();

#define clean_lullaby_thread luaI_nothing

static const luaL_Reg thread_function_list [] = {
  {"async",l_async},
  {"buffer",l_buffer},
  {"testcopy",l_testcopy},
  {"mutex", l_mutex},
  {"usleep", l_usleep},
  {"sleep", l_sleep},
  {NULL,NULL}
};

static struct config thread_config[] = {
  {.type = c_none}
};
