#include "lua.h"
#include "util.h"
#include "sort.h"
#include "config.h"
#include <stdint.h>

void i_shuffle(double*, size_t);
uint64_t i_len(lua_State*,int);

int l_len(lua_State*);          //[double+int] -> i
int l_reverse(lua_State*);      //[double+int] -> arr[N]
int l_max(lua_State*);     //[double+int] -> i
int l_min(lua_State*);        //[double+int] -> i
int l_shuffle(lua_State*);      //[double+int] -> arr[N]
int l_sum(lua_State*);      //[double+int] -> i

int l_indexof(lua_State*); //[double+int], item -> i
int l_sindexof(lua_State*);//[double+int] (greatest -> least), item -> i
int l_split(lua_State*);
int l_to_char_array(lua_State*);

int l_unpack(lua_State*);
int l_split(lua_State*);
int l_equal(lua_State*);
int l_dup(lua_State*);

#define clean_lullaby_table luaI_nothing

#warning "docs needed here"
static const luaL_Reg table_function_list [] = {
  {"len",l_len},
  {"reverse",l_reverse}, //no docs
  {"max",l_max}, //no docs
  {"min",l_min}, //no docs
  {"shuffle",l_shuffle}, //no docs
  {"sum",l_sum}, //no docs
  {"split",l_split},
  {"to_char_array", l_to_char_array}, //no docs

  {"index",l_indexof}, //no docs
  {"sindex",l_sindexof}, //no docs

  //updates these and the functions
  {"quicksort",l_quicksort},
  {"mergesort",l_mergesort},
  {"shellsort",l_shellsort},
  {"bubblesort",l_bubblesort},
  {"heapsort",l_heapsort},

  {"unpack", l_unpack}, //no docs
  {"equal", l_equal},
  {"dup", l_dup},

  {NULL,NULL}
};

static struct config table_config[] = {
  {.type = c_none}
};
