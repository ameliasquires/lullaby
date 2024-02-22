#include "lua.h"
#include "util.h"

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
