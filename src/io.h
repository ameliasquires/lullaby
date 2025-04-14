#include "lua.h"
#include "config.h"

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
int l_arg_handle(lua_State*);

int l_json_parse(lua_State*);

extern int _file_malloc_chunk;
extern int _print_type;
extern int _max_depth;
extern int _start_nl_at;
extern int _collapse_all;
extern int _collapse_to_memory;
extern int _print_meta;

static struct config io_config[] = {
  {.name = "filechunksize", .type = c_int, .value = {.c_int = &_file_malloc_chunk}},
  {.name = "print_type", .type = c_int, .value = {.c_int = &_print_type}},
  {.name = "max_depth", .type = c_int, .value = {.c_int = &_max_depth}},
  {.name = "start_nl_at", .type = c_int, .value = {.c_int = &_start_nl_at}},
  {.name = "collapse_all", .type = c_int, .value = {.c_int = &_collapse_all}},
  {.name = "collapse_to_memory", .type = c_int, .value = {.c_int = &_collapse_to_memory}},
  {.name = "print_meta", .type = c_int, .value = {.c_int = &_print_meta}},
  {.type = c_none}
};

static const luaL_Reg io_function_list [] = {
  {"readfile",l_readfile},
  {"debug",l_debug},
  {"log",l_log},
  {"warn",l_warn},
  {"error",l_error},
  {"pprint",l_pprint},
  {"json_parse",l_json_parse},
  {"arg_handle",l_arg_handle},
  {NULL,NULL}
};
