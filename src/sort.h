#include "lua.h"
#include "util.h"

//comparison sorts
int l_quicksort(lua_State*);    //[double+int] -> arr[N] (greatest -> least)
int l_mergesort(lua_State*);    //[double+int] -> arr[N] (greatest -> least) 
int l_shellsort(lua_State*);    //[double+int] -> arr[N] (greatest -> least)
int l_bubblesort(lua_State*);   //[double+int] -> arr[N] (greatest -> least)
int l_heapsort(lua_State*);     //[double+int] -> arr[N] (greatest -> least)
