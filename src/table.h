#include "lua.h"
#include "i_util.h"
#include "i_common.h"

int l_len(lua_State*);          //[double+int] -> i
int l_reverse(lua_State*);      //[double+int] -> arr[N]
int l_greatest(lua_State*);     //[double+int] -> i
int l_least(lua_State*);        //[double+int] -> i
int l_shuffle(lua_State*);      //[double+int] -> arr[N]
int l_sum(lua_State*);      //[double+int] -> i

int l_indexof(lua_State*); //[double+int], item -> i
int l_sindexof(lua_State*);//[double+int] (greatest -> least), item -> i

//comparison sorts
int l_quicksort(lua_State*);    //[double+int] -> arr[N] (greatest -> least)
int l_mergesort(lua_State*);    //[double+int] -> arr[N] (greatest -> least) 
int l_shellsort(lua_State*);    //[double+int] -> arr[N] (greatest -> least)
int l_bubblesort(lua_State*);   //[double+int] -> arr[N] (greatest -> least)
int l_heapsort(lua_State*);     //[double+int] -> arr[N] (greatest -> least)

//non-comparison sorts
  //good for large arrays filled with small values
int l_countingsort(lua_State*); //[int] (arr[N] >= 0) -> arr[N] (least -> greatest)

//esoteric sorts
int l_miraclesort(lua_State*);  //[double+int] -> arr[-∞<=N<=∞] (greatest -> least)
int l_stalinsort(lua_State*);   //[double+int] -> arr[?<=N] (greatest -> least)
int l_slowsort(lua_State*);     //[double+int] -> arr[N] (greatest -> least)
int l_bogosort(lua_State*);     //[double+int] -> arr[N] (greatest -> least)


static const luaL_Reg array_function_list [] = {
      {"len",l_len},
      {"reverse",l_reverse},
      {"greatest",l_greatest},
      {"least",l_least},
      {"shuffle",l_shuffle},
      {"sum",l_sum},

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

      {NULL,NULL}
};
