#include "lua.h"
#include "util.h"
#include "sort.h"
#include "config.h"
#include <stdint.h>

void i_shuffle(double*, size_t);
uint64_t i_len(lua_State*,int);

int l_len(lua_State*);          //[double+int] -> i
int l_reverse(lua_State*);      //[double+int] -> arr[N]
int l_greatest(lua_State*);     //[double+int] -> i
int l_least(lua_State*);        //[double+int] -> i
int l_shuffle(lua_State*);      //[double+int] -> arr[N]
int l_sum(lua_State*);      //[double+int] -> i

int l_indexof(lua_State*); //[double+int], item -> i
int l_sindexof(lua_State*);//[double+int] (greatest -> least), item -> i
int l_split(lua_State*);
int l_to_char_array(lua_State*);

int l_unpack(lua_State*);
int l_split(lua_State*);

static const luaL_Reg table_function_list [] = {
      {"len",l_len},
      {"reverse",l_reverse},
      {"greatest",l_greatest},
      {"least",l_least},
      {"shuffle",l_shuffle},
      {"sum",l_sum},
      {"split",l_split},
      {"to_char_array", l_to_char_array},

      {"index",l_indexof},
      {"sindex",l_sindexof},
      
      {"quicksort",l_quicksort},
      {"mergesort",l_mergesort},
      {"shellsort",l_shellsort},
      {"bubblesort",l_bubblesort},
      {"heapsort",l_heapsort},

      {"countingsort",l_countingsort},

      {"miraclesort",l_miraclesort},
      {"stalinsort",l_stalinsort},
      {"slowsort",l_slowsort},
      {"bogosort",l_bogosort},

      {"unpack", l_unpack},

      {NULL,NULL}
};

static struct config table_config[] = {
  {.type = c_none}
};
