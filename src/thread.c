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
#include "hash/fnv.h"

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

void lib_thread_clean(){
  if(thread_locks == NULL) return;

  for(int i = 0; i != thread_locks->size; i++){
    if(thread_locks->arr[i].used){
      //pthread_mutex_destroy(thread_locks->arr[i].value);
      free(thread_locks->arr[i].value);
    }
  }
      
  larray_clear(thread_locks);
}
int l_tlock(lua_State* L){
    int idx = luaL_checkinteger(L, 1);

    pthread_mutex_lock(&thread_lock_lock);
    //pthread_mutex_lock(&thread_priority_lock);
    //pthread_mutex_unlock(&thread_priority_lock);
    pthread_mutex_t mutex;
    if(thread_locks == NULL) thread_locks = larray_init();
    int i = 0;
    if((i = larray_geti(thread_locks, idx)) == -1){
        pthread_mutex_init(&mutex, NULL);
        pthread_mutex_lock(&mutex);
        pthread_mutex_t* mp = malloc(sizeof * mp);
        *mp = mutex;
        int id = larray_set(&thread_locks, idx, (void*)mp);
    } else {
        pthread_mutex_t *m = (pthread_mutex_t*)thread_locks->arr[i].value;
        pthread_mutex_lock(&thread_priority_lock);
    
        pthread_mutex_unlock(&thread_lock_lock);
        pthread_mutex_lock(m);
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
        pthread_mutex_t *m = (pthread_mutex_t*)thread_locks->arr[i].value;

        pthread_mutex_unlock(m);
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
        luaI_deepcopy(L, info->L, 0);

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
        luaI_deepcopy(info->L, L, 0);
        
        lua_settop(info->L, ot);
    }

    return info->return_count;
}

int l_clean(lua_State* L){
    lua_pushstring(L, "_");
    lua_gettable(L, 1);
    struct thread_info* info = lua_touserdata(L, -1);
    if(info != NULL && info->L != NULL){
      
      lua_gc(info->L, LUA_GCRESTART);
      lua_gc(info->L, LUA_GCCOLLECT);
      lua_close(info->L);
      info->L = NULL;
      pthread_mutex_destroy(&info->lock);
      free(info);

      luaI_tsetlud(L, 1, "_", NULL);
    }
    return 0;
}

int l_async(lua_State* oL){
  lua_State* L = luaL_newstate(); 

  lua_getglobal(oL, "_G");
  luaI_deepcopy(oL, L, SKIP_GC);
  //lua_set_global_table(L);

  return 0;

  struct thread_info* args = calloc(1, sizeof * args);
  args->L = L;
  //args->lock = (pthread_mutex_t)PTHREAD_MUTEX_INITIALIZER;
  pthread_mutex_init(&args->lock, NULL);
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

struct thread_buffer {
  lua_State* L;
  pthread_mutex_t lock;
};

int _buffer_get(lua_State* L){
  struct thread_buffer *buffer = lua_touserdata(L, 1);
  pthread_mutex_lock(&buffer->lock);
  luaI_deepcopy(buffer->L, L, 0);
  pthread_mutex_unlock(&buffer->lock);
  return 1;
}

int _buffer_set(lua_State* L){
  struct thread_buffer *buffer = lua_touserdata(L, 1);
  pthread_mutex_lock(&buffer->lock);
  lua_settop(buffer->L, 0);
  luaI_deepcopy(L, buffer->L, 0);
  pthread_mutex_unlock(&buffer->lock);
  return 1;
}

int _buffer_mod(lua_State* L){
  struct thread_buffer *buffer = lua_touserdata(L, 1);
  pthread_mutex_lock(&buffer->lock);
  luaI_deepcopy(buffer->L, L, 0);
  int item = lua_gettop(L);
  lua_pushvalue(L, 2);
  lua_pushvalue(L, item);
  lua_call(L, 1, 1);

  if(lua_type(L, -1) != LUA_TNIL){
    lua_settop(buffer->L, 0);
    luaI_deepcopy(L, buffer->L, 0);
  }
  pthread_mutex_unlock(&buffer->lock);
  return 1;
}


int l_buffer_index(lua_State* L){
  uint64_t len, hash;
  const char* str = luaL_tolstring(L, 2, &len);

  hash = fnv_1((uint8_t*)str, len, v_1);

  switch(hash){
    case 0xd8c8ad186b9ed323: //get
      lua_pushcfunction(L, _buffer_get);
      break;
    case 0xd89f9d186b7bb367: //set
      lua_pushcfunction(L, _buffer_set);
      break;
    case 0xd8b3c7186b8ca31f: //mod
      lua_pushcfunction(L, _buffer_mod);
      break;
    default:
      lua_pushnil(L);
      break;
  }
  return 1;
}

int hi(lua_State* L){
  printf("hi\n");
  return 0;
}

int meta_proxy(lua_State* L){
  printf("proxy");
  return 1;
}

void meta_proxy_gen(lua_State* L, int meta_idx, int new_meta_idx){  

  lua_pushnil(L);
  for(; lua_next(L, meta_idx) != 0;){
    int k, v = lua_gettop(L);
    k = lua_gettop(L) - 1;

    lua_pushcfunction(L, meta_proxy); 
    lua_setglobal(L, "__proxy_call");

    char* fn = calloc(128, sizeof * fn); //todo: find optimal value for these
    const char* key = lua_tostring(L, k);
    sprintf(fn, "return function(_a,_b,_c)\
return __proxy_call('%s',table.unpack({_a,_b,_c}));end", key);
    luaL_dostring(L, fn);
    //printf("%s\n",fn);
    free(fn);
    int nf = lua_gettop(L);

    luaI_tsetv(L, new_meta_idx, lua_tostring(L, k), nf);

    lua_pop(L, 3);
  }
}

int l_buffer_gc(lua_State* L){
  printf("gc\n");
  struct thread_buffer *buffer = lua_touserdata(L, 1);
  pthread_mutex_lock(&buffer->lock);
  lua_close(buffer->L);
  return 0;
}

int l_buffer(lua_State* L){ 
  int use = lua_getmetatable(L, 1);
  int old_meta_idx = lua_gettop(L);

  lua_newtable(L);
  int meta_idx = lua_gettop(L);
  if(use!=0) meta_proxy_gen(L, old_meta_idx, meta_idx);
  luaI_tsetcf(L, meta_idx, "__index", l_buffer_index);
  luaI_tsetcf(L, meta_idx, "__gc", l_buffer_gc);

  struct thread_buffer *buffer = lua_newuserdata(L, sizeof * buffer);
  int buffer_idx = lua_gettop(L);

  buffer->L = luaL_newstate();
  pthread_mutex_init(&buffer->lock, NULL);
  lua_pushvalue(L, 1);
  luaI_deepcopy(L, buffer->L, SKIP_GC);

  lua_pushvalue(L, meta_idx);
  lua_setmetatable(L, buffer_idx);
  lua_pushvalue(L, buffer_idx);
  return 1;
}

int l_testcopy(lua_State* L){
  lua_settop(L, 0);
  lua_State* temp = luaL_newstate();
  lua_getglobal(L, "_G");
  luaI_deepcopy(L, temp, SKIP_GC);
  //luaI_deepcopy(temp, L, NULL, SKIP_GC);
  lua_close(temp);
  return 1;
}
