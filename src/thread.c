#include "thread.h"
#include "lua5.4/lauxlib.h"
#include "lua5.4/lua.h"
#include "stdint.h"
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>
#include "types/str.h"
#include "util.h"

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
    lua_pushstring(L, "_");
    lua_gettable(L, 1);
    struct thread_info* info = lua_touserdata(L, -1);
    info->return_count = return_count;

    for(int i = info->return_count - 1; i != -1; i--){
        int ot = lua_gettop(L);

        lua_pushvalue(L, 2 + i);
        i_dcopy(L, info->L, NULL);

        lua_settop(L, ot);
    }

    pthread_mutex_unlock(&info->lock);

    pthread_exit(NULL);
    p_error("thread did not exit");

    return 1;
}

void* handle_thread(void* _args){
  struct thread_info* args = (struct thread_info*)_args;
  lua_State* L = args->L;

  lua_newtable(L);
  int res_idx = lua_gettop(L);
  luaI_tsetcf(L, res_idx, "res", l_res);
  luaI_tsetlud(L, res_idx, "_", args);

  luaL_loadbuffer(L, args->function->c, args->function->len, "thread");
  str_free(args->function);

  lua_pushvalue(L, res_idx);
  lua_call(L, 1, 0);

  pthread_mutex_unlock(&args->lock);

  return NULL;
}

int l_await(lua_State* L){
    lua_pushstring(L, "_");
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
  lua_set_global_table(L);

  struct thread_info* args = calloc(1, sizeof * args);
  args->L = L;
  args->cond = (pthread_cond_t)PTHREAD_COND_INITIALIZER;
  args->lock = (pthread_mutex_t)PTHREAD_MUTEX_INITIALIZER;
  pthread_mutex_lock(&args->lock);
  args->return_count = 0;

  args->function = str_init("");
  lua_pushvalue(oL, 1);
  lua_dump(oL, writer, (void*)args->function, 0);
 
  pthread_create(&args->tid, NULL, handle_thread, (void*)args);
  pthread_detach(args->tid);

  lua_newtable(oL);
  int res_idx = lua_gettop(oL);
  luaI_tsetcf(oL, res_idx, "await", l_await);
  luaI_tsetlud(oL, res_idx, "_", args);
  lua_pushvalue(oL, res_idx);
  return 1;
}
