#include "lua.h"
#include "util.h"

#define i_swap(A,B) double temp = A; A = B; B = temp;

int l_len(lua_State*);          //[double+int] -> i
int l_reverse(lua_State*);      //[double+int] -> arr[N]
int l_greatest(lua_State*);     //[double+int] -> i
int l_least(lua_State*);        //[double+int] -> i
int l_shuffle(lua_State*);      //[double+int] -> arr[N]

//comparison sorts
int l_quicksort(lua_State*);    //[double+int] -> arr[N] (greatest -> least)
int l_mergesort(lua_State*);    //[double+int] -> arr[N] (greatest -> least) 
int l_shellsort(lua_State*);    //[double+int] -> arr[N] (greatest -> least)
int l_bubblesort(lua_State*);   //[double+int] -> arr[N] (greatest -> least)
int l_heapsort(lua_State*);     //[double+int] -> arr[N] (greatest -> least)

//non-comparison sorts
int l_countingsort(lua_State*); //[int], arr[N] >= 0 -> arr[N] (greatest -> least)

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
