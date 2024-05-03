#include "thread.h"
#include "lua5.4/lauxlib.h"
#include "lua5.4/lua.h"
#include "stdint.h"
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>
#include "types/str.h"

struct thread_info {
    str* function;
    lua_State* L;
    int return_count;
    pthread_t tid;
    pthread_cond_t cond;
    pthread_mutex_t lock;
};

#include "io.h"

int l_res(lua_State* L){
    int return_count = lua_gettop(L) - 1;
    lua_pushstring(L, "t");
    lua_gettable(L, 1);
    struct thread_info* info = lua_touserdata(L, -1);
    info->return_count = return_count;

    for(int i = info->return_count - 1; i != -1; i--){
        int ot = lua_gettop(L);

        lua_pushvalue(L, 2 + i);
        l_pprint(L);
        i_dcopy(L, info->L, NULL);

        lua_settop(L, ot);
    }

    pthread_mutex_unlock(&info->lock);

    pthread_cond_t c = PTHREAD_COND_INITIALIZER;
    pthread_mutex_t l = PTHREAD_MUTEX_INITIALIZER;
    pthread_mutex_lock(&l);
    for (;;) pthread_cond_wait(&c, &l);
    pthread_mutex_unlock(&l);

    return 1;
}

void* handle_thread(void* _args){
  struct thread_info* args = (struct thread_info*)_args;
  lua_State* L = args->L;

  pthread_mutex_lock(&args->lock);

  lua_newtable(L);
  int res_idx = lua_gettop(L);
  luaI_tsetcf(L, res_idx, "res", l_res);
  luaI_tsetlud(L, res_idx, "t", args);

  luaL_loadbuffer(L, args->function->c, args->function->len, "thread");
  str_free(args->function);

  lua_pushvalue(L, res_idx);
  lua_call(L, 1, 0);

  return NULL;
}

int l_await(lua_State* L){
    lua_pushstring(L, "t");
    lua_gettable(L, 1);
    struct thread_info* info = lua_touserdata(L, -1);

    pthread_mutex_lock(&info->lock);

    for(int i = 0; i != info->return_count; i++){
        int ot = lua_gettop(info->L);

        lua_pushvalue(info->L, ot - info->return_count + i);
        i_dcopy(info->L, L, NULL);

        lua_settop(info->L, ot);
    }

    return info->return_count;
}

int l_async(lua_State* oL){
   lua_State* L = luaL_newstate(); 

  luaL_openlibs(L);

  lua_getglobal(oL, "_G");
  i_dcopy(oL, L, NULL);
  lua_setglobal(L, "_G");

  struct thread_info* args = calloc(1, sizeof * args);
  args->L = L;
  args->cond = PTHREAD_COND_INITIALIZER;
  args->lock = PTHREAD_MUTEX_INITIALIZER;
  args->return_count = 0;

  args->function = str_init("");
  lua_pushvalue(oL, 1);
  lua_dump(oL, writer, (void*)args->function, 0);
 
  pthread_create(&args->tid, NULL, handle_thread, (void*)args);
  pthread_detach(args->tid);

  lua_newtable(oL);
  int res_idx = lua_gettop(oL);
  luaI_tsetcf(oL, res_idx, "res", l_await);
  luaI_tsetlud(oL, res_idx, "t", args);
  lua_pushvalue(oL, res_idx);
  return 1;
}
