#include "lua.h"


extern int _print_type;
extern int _max_depth;
extern int _start_nl_at;
extern int _collapse_all;
extern int _collapse_to_memory;
extern int _print_meta;

extern int _file_malloc_chunk;

struct str_to_int {
  const char* key;
  int* value;
};

static struct str_to_int config_map[] = {
  {"file_chunksize", &_file_malloc_chunk},
  {"print_type", &_print_type},
  {"max_depth", &_max_depth},
  {"collapse_all", &_collapse_all},
  {"start_nl_at", &_start_nl_at},
  {"collapse_to_memory", &_collapse_to_memory},
  {"print_meta", &_print_meta},
  {NULL,NULL}
};

int l_set(lua_State*);

static const luaL_Reg config_function_list [] = {
  {"set",l_set},
  {NULL,NULL}
};
