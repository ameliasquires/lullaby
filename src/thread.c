#include "thread.h"
#include "lua5.4/lauxlib.h"
#include "lua5.4/lua.h"
#include "stdint.h"
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>
#include "types/str.h"
#include "util.h"

#include "types/larray.h"

struct thread_info {
    str* function;
    lua_State* L;
    int return_count, done;
    pthread_t tid;
    pthread_mutex_t lock;
};

#include "io.h"

//give the current thread priority to locking thread_lock_lock (fixes race conds)
pthread_mutex_t thread_priority_lock = (pthread_mutex_t)PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t thread_lock_lock = (pthread_mutex_t)PTHREAD_MUTEX_INITIALIZER;
larray_t* thread_locks = NULL;

int l_tlock(lua_State* L){
    int idx = luaL_checkinteger(L, 1);

    pthread_mutex_lock(&thread_lock_lock);
    pthread_mutex_lock(&thread_priority_lock);
    pthread_mutex_unlock(&thread_priority_lock);
    pthread_mutex_t mutex;
    if(thread_locks == NULL) thread_locks = larray_init();
    int i = 0;
    if((i = larray_geti(thread_locks, idx)) == -1){
        pthread_mutex_init(&mutex, NULL);
        pthread_mutex_lock(&mutex);
        larray_set(&thread_locks, idx, (void*)mutex);
    } else {
        pthread_mutex_t m = (pthread_mutex_t)thread_locks->arr[i].value;
        pthread_mutex_lock(&thread_priority_lock);
    
        pthread_mutex_unlock(&thread_lock_lock);
        pthread_mutex_lock(&m);
        pthread_mutex_lock(&thread_lock_lock);

        pthread_mutex_unlock(&thread_priority_lock);
        thread_locks->arr[i].value = (void*)m;

    }
    
    pthread_mutex_unlock(&thread_lock_lock);
    return 0;
}

int l_tunlock(lua_State* L){
    int idx = luaL_checkinteger(L, 1);

    pthread_mutex_lock(&thread_lock_lock);
    int i = 0;
    if(thread_locks != NULL && (i = larray_geti(thread_locks, idx)) != -1){
        pthread_mutex_t m = (pthread_mutex_t)thread_locks->arr[i].value;
        pthread_mutex_unlock(&m);
        thread_locks->arr[i].value = (void*)m;
    }

    pthread_mutex_unlock(&thread_lock_lock);
    return 0;
}

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
  lua_gc(L, LUA_GCSTOP);

  lua_newtable(L);
  int res_idx = lua_gettop(L);
  luaI_tsetlud(L, res_idx, "_", args);

  lua_newtable(L);
  int meta_idx = lua_gettop(L);
  luaI_tsetcf(L, meta_idx, "__call", l_res);
  lua_pushvalue(L, meta_idx);
  lua_setmetatable(L, res_idx);

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

    if(info->L == NULL) p_fatal("thread has already been cleaned");
    if(!info->done) pthread_mutex_lock(&info->lock);
    info->done = 1;

    for(int i = 0; i != info->return_count; i++){
        int ot = lua_gettop(info->L);

        lua_pushvalue(info->L, ot - info->return_count + i);
        i_dcopy(info->L, L, NULL);
        
        lua_settop(info->L, ot);
    }

    return info->return_count;
}

int l_clean(lua_State* L){
    lua_pushstring(L, "_");
    lua_gettable(L, 1);
    struct thread_info* info = lua_touserdata(L, -1);
    if(info->L != NULL){
        lua_gc(info->L, LUA_GCRESTART);
        lua_gc(info->L, LUA_GCCOLLECT);
        lua_close(info->L);
        info->L = NULL;
        pthread_mutex_destroy(&info->lock);
        free(info);
    }
    return 0;
}

int l_async(lua_State* oL){
   lua_State* L = luaL_newstate(); 

  luaL_openlibs(L);

  lua_getglobal(oL, "_G");
  i_dcopy(oL, L, NULL);
  lua_set_global_table(L);

  struct thread_info* args = calloc(1, sizeof * args);
  args->L = L;
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
  luaI_tsetcf(oL, res_idx, "clean", l_clean);
  luaI_tsetlud(oL, res_idx, "_", args);

  lua_newtable(oL);
  int meta_idx = lua_gettop(oL);
  luaI_tsetcf(oL, meta_idx, "__gc", l_clean);

  lua_pushvalue(oL, meta_idx);
  lua_setmetatable(oL, res_idx);
  lua_pushvalue(oL, res_idx);
  return 1;
}
