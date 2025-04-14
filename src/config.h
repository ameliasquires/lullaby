#include "lua.h"

#ifndef _config_h
#define _config_h

enum config_type {
  c_none,
  c_string,
  c_int,
  c_number,
  //c_function,
  //c_table
};

struct config {
  const char* name;
  enum config_type type;

  //a single value will be valid, all other are undefined (except len which will be valid for c_string and c_function)
  struct value {
    char** c_string;
    int* c_int;
    float* c_number;
    char** c_function;
    //location of table in lua registery
    int* c_table_idx;
    //length used for c_string or c_function
    size_t* len;
  } value;
};

int i_config_metatable(lua_State*, struct config[]);

#endif
