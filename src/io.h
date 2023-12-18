#include "lua.h"

#define color_black "\e[30m"
#define color_red "\e[31m"
#define color_green "\e[32m"
#define color_yellow "\e[33m"
#define color_blue "\e[34m"
#define color_magenta "\e[35m"
#define color_cyan "\e[36m"
#define color_lgray "\e[37m"
#define color_gray "\e[90m"
#define color_lred "\e[91m"
#define color_lgreen "\e[92m"
#define color_lyellow "\e[93m"
#define color_lblue "\e[94m"
#define color_lmagenta "\e[95m"
#define color_lcyan "\e[96m"
#define color_white "\e[97m"
#define color_reset "\e[0m"

int l_readfile(lua_State*);
int l_debug(lua_State*);
int l_log(lua_State*);
int l_warn(lua_State*);
int l_error(lua_State*);
int l_pprint(lua_State*);

static const luaL_Reg io_function_list [] = {
  {"readfile",l_readfile},
  {"debug",l_debug},
  {"log",l_log},
  {"warn",l_warn},
  {"error",l_error},
  {"pprint",l_pprint},
  {NULL,NULL}
};
