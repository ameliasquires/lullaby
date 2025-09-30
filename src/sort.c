#include "table.h"
#include <stdlib.h>

int i_hoarepartition(double* arr, int low, int high){
  double pivot = arr[((int)((high - low) / 2)) + low];
  int i = low - 1;
  int j = high + 1;

  for(;;){
    i++; j--;

    while(arr[i] > pivot) i++;
    while(arr[j] < pivot) j--;
    if(i >= j) return j;

    i_swap(arr[i],arr[j]);
  }
}

void i_quicksort(double* arr, int low, int high){
  if(low >= 0 && high >= 0 && low < high){
    int p = i_hoarepartition(arr, low, high);
    i_quicksort(arr, low, p);
    i_quicksort(arr, p + 1, high);
  }
}

int l_quicksort(lua_State* L) {
  luaL_checktype(L, 1, LUA_TTABLE);
  size_t len = lua_objlen(L,1); 
  double* nums = malloc(sizeof * nums * len);
  for(size_t i = 0; i <= len-1; i++){

    lua_pushinteger(L,i+1);
    lua_gettable(L,1);

    nums[i] = luaL_checknumber(L, -1);
    lua_pop(L,1);
  }

  i_quicksort(nums, 0, len - 1);

  lua_newtable(L);
  for(size_t i = 0; i != len; i++){
    lua_pushnumber(L,i+1);
    lua_pushnumber(L,nums[i]);
    lua_settable(L, -3);
  }

  free(nums);
  return 1;
}

void i_merge(double* arr, int b, int m, int e){
  int n1 = m - b + 1;
  int n2 = e - m;

  double* left = malloc(sizeof * left * n1);
  double* right = malloc(sizeof * right * n2);

  for(int i = 0; i < n1; i++) left[i] = arr[b + i];
  for(int i = 0; i < n2; i++) right[i] = arr[m + 1 + i];

  int l_ind = 0;
  int r_ind = 0;
  int k = b;

  for(; l_ind < n1 && r_ind < n2; k++){
    if(left[l_ind] >= right[r_ind]){
      arr[k] = left[l_ind];
      l_ind++;
    } else {
      arr[k] = right[r_ind];
      r_ind++;
    }
  }

  for(; l_ind < n1; k++){
    arr[k] = left[l_ind];
    l_ind++;
  }
  for(; r_ind < n2; k++){
    arr[k] = right[r_ind];
    r_ind++;
  }

  free(left);
  free(right);
}

void i_mergesort(double* arr, int b, int e){
  if(b < e){
    int mid = (b + e) /2;
    i_mergesort(arr, b, mid);
    i_mergesort(arr, mid + 1, e);
    i_merge(arr, b, mid, e);
  } 
}

int l_mergesort(lua_State* L) {
  luaL_checktype(L, 1, LUA_TTABLE);
  size_t len = lua_objlen(L,1); 
  double* nums = malloc(sizeof * nums * len);
  for(size_t i = 0; i <= len-1; i++){

    lua_pushinteger(L,i+1);
    lua_gettable(L,1);

    nums[i] = luaL_checknumber(L, -1);
    lua_pop(L,1);
  }

  i_mergesort(nums, 0, len - 1);

  lua_newtable(L);
  for(size_t i = 0; i != len; i++){
    lua_pushnumber(L,i+1);
    lua_pushnumber(L,nums[i]);
    lua_settable(L, -3);
  }

  free(nums);
  return 1;
}

void i_heapify(double* arr, int n, int i){
  int largest = i;
  int left = 2 * i + 1;
  int right = 2 * i + 2;

  if(left < n && arr[left] < arr[largest])
    largest = left;

  if(right < n && arr[right] < arr[largest])
    largest = right;

  if(largest != i){
    i_swap(arr[i],arr[largest]);
    i_heapify(arr,n,largest);
  }
}

int l_heapsort(lua_State* L) {
  luaL_checktype(L, 1, LUA_TTABLE);
  size_t len = lua_objlen(L,1); 
  double* nums = malloc(sizeof * nums * len);
  for(size_t i = 0; i <= len-1; i++){

    lua_pushinteger(L,i+1);
    lua_gettable(L,1);

    nums[i] = luaL_checknumber(L, -1);
    lua_pop(L,1);
  }
  for(int i = len / 2 - 1; i >= 0; i--) 
    i_heapify(nums,len,i);

  for(int i = len - 1; i >= 0; i--){
    i_swap(nums[i],nums[0]);
    i_heapify(nums, i, 0);
  }
  lua_newtable(L);
  for(size_t i = 0; i != len; i++){
    lua_pushnumber(L,i+1);
    lua_pushnumber(L,nums[i]);
    lua_settable(L, -3);
  }
  free(nums);
  return 1;
}

int l_shellsort(lua_State* L) {
  luaL_checktype(L, 1, LUA_TTABLE);
  size_t len = lua_objlen(L,1);
  double* nums = malloc(sizeof * nums * len);
  for(size_t i = 0; i <= len-1; i++){

    lua_pushinteger(L,i+1);
    lua_gettable(L,1);

    nums[i] = luaL_checknumber(L, -1);
    lua_pop(L,1);
  }

  for(int interval = len/2; interval > 0; interval /=2){
    for(int i = interval; i < len; i++){
      double temp = nums[i];
      int j;
      for(j = i; j >= interval && nums[j - interval] < temp; j -= interval){
        nums[j] = nums[j - interval];
      }
      nums[j] = temp;
    }
  }

  lua_newtable(L);
  for(size_t i = 0; i != len; i++){
    lua_pushnumber(L,i+1);
    lua_pushnumber(L,nums[i]);
    lua_settable(L, -3);
  }

  free(nums);
  return 1;
}

int l_bubblesort(lua_State* L) {
  luaL_checktype(L, 1, LUA_TTABLE);
  size_t len = lua_objlen(L,1);
  double* nums = malloc(sizeof * nums * len);
  for(size_t i = 0; i <= len-1; i++){

    lua_pushinteger(L,i+1);
    lua_gettable(L,1);

    nums[i] = luaL_checknumber(L, -1);
    lua_pop(L,1);
  }

  int n = len;
  for(;n > 0;){
    int new = 0;

    for(int i = 0; i != n-1; i++){
      if(nums[i+1]>nums[i]){
        double temp = nums[i];
        nums[i] = nums[i+1];
        nums[i+1] = temp;

        new = i+1;
      } 
    }

    n = new;
  }

  lua_newtable(L);
  for(size_t i = 0; i != len; i++){
    lua_pushnumber(L,i+1);
    lua_pushnumber(L,nums[i]);
    lua_settable(L, -3);
  }

  free(nums);
  return 1;
}

int l_countingsort(lua_State* L) {
  luaL_checktype(L, 1, LUA_TTABLE);
  size_t len = lua_objlen(L,1);
  int* nums = malloc(sizeof * nums * len);
  int* out = malloc(sizeof * nums * len);
  int max = 0;
  for(int i = 0; i <= len-1; i++){

    lua_pushinteger(L,i+1);
    lua_gettable(L,1);

    nums[i] = luaL_checknumber(L, -1);
    out[i] = 0;

    if(nums[i]<0) p_fatal("array.countingsort(<table>) requires all indices to be >= 0");
    max = max < nums[i] ? nums[i] : max;

    lua_pop(L,1);
  }

  int* count = calloc(max + 1, sizeof * count);

  for(size_t i = 0; i < len; i++){
    count[nums[i]]++;
  } 

  for(size_t i = 1; i <= max; i++){
    count[i] += count[i - 1];
  }

  for(int i = len - 1; i >= 0; i--){
    out[count[nums[i]] - 1] = nums[i];
    count[nums[i]]--;
  }

  lua_newtable(L);
  for(size_t i = 0; i != len; i++){
    lua_pushnumber(L,i+1);
    lua_pushnumber(L,out[i]);
    lua_settable(L, -3);
  }

  free(count);
  free(nums);
  free(out);
  return 1;
}

int i_sorted(double* arr, size_t len){
  for(size_t i = 0; i != len - 1; i++)
    if(arr[i] > arr[i+1]) return 0;
  return 1;
}

int l_miraclesort(lua_State* L) {
  luaL_checktype(L, 1, LUA_TTABLE);
  size_t len = lua_objlen(L,1);
  double* nums = malloc(sizeof * nums * len);
  for(size_t i = 0; i <= len-1; i++){

    lua_pushinteger(L,i+1);
    lua_gettable(L,1);

    nums[i] = luaL_checknumber(L, -1);
    lua_pop(L,1);
  }

  for(;!i_sorted(nums,len););

  lua_newtable(L);
  for(size_t i = 0; i != len; i++){
    lua_pushnumber(L,i+1);
    lua_pushnumber(L,nums[i]);
    lua_settable(L, -3);
  }

  free(nums);
  return 1;
}

int l_stalinsort(lua_State* L) {
  luaL_checktype(L, 1, LUA_TTABLE);
  size_t len = lua_objlen(L,1);
  size_t rlen = 0;
  double* nums = malloc(sizeof * nums * len);
  for(size_t i = 0; i <= len-1; i++){

    lua_pushinteger(L,i+1);
    lua_gettable(L,1);

    double n = luaL_checknumber(L, -1);
    if(rlen == 0 || nums[rlen - 1] <= n){
      nums[rlen] = n;
      rlen++;
    }

    lua_pop(L,1);
  }


  lua_newtable(L);
  for(size_t i = 0; i != rlen; i++){
    lua_pushnumber(L,i+1);
    lua_pushnumber(L,nums[i]);
    lua_settable(L, -3);
  }

  free(nums);
  return 1;
}

void i_slowsort(double* arr, int i, int j){
  if(i >= j) return;

  int m = (i + j) /2;

  i_slowsort(arr, i, m);
  i_slowsort(arr, m + 1, j);

  if(arr[j] < arr[m]){
    i_swap(arr[j], arr[m]);
  }

  i_slowsort(arr, i, j - 1);
}

int l_slowsort(lua_State* L) {
  luaL_checktype(L, 1, LUA_TTABLE);
  size_t len = lua_objlen(L,1);
  double* nums = malloc(sizeof * nums * len);
  for(int i = 0; i <= len-1; i++){

    lua_pushinteger(L,i+1);
    lua_gettable(L,1);

    nums[i] = luaL_checknumber(L, -1);
    lua_pop(L,1);
  }

  i_slowsort(nums, 0, len - 1);

  lua_newtable(L);
  for(size_t i = 0; i != len; i++){
    lua_pushnumber(L,i+1);
    lua_pushnumber(L,nums[i]);
    lua_settable(L, -3);
  }

  free(nums);
  return 1;
}

int l_bogosort(lua_State* L) {
  luaL_checktype(L, 1, LUA_TTABLE);
  size_t len = lua_objlen(L,1);
  double* nums = malloc(sizeof * nums * len);
  for(size_t i = 0; i <= len-1; i++){

    lua_pushinteger(L,i+1);
    lua_gettable(L,1);

    nums[i] = luaL_checknumber(L, -1);
    lua_pop(L,1);
  }

  for(;!i_sorted(nums, len);){
    i_shuffle(nums, len);
  }

  lua_newtable(L);
  for(size_t i = 0; i != len; i++){
    lua_pushnumber(L,i+1);
    lua_pushnumber(L,nums[i]);
    lua_settable(L, -3);
  }

  free(nums);
  return 1;
}
