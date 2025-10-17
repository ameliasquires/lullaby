#include "thread.h"
#include "stdint.h"
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>
#include <signal.h>
#include <sys/eventfd.h>
#include "types/str.h"
#include "util.h"

#include "types/larray.h"
#include "hash/fnv.h"
#include "table.h"

struct thread_info {
  str* function;
  lua_State* L;
  int return_count, done;
  pthread_t tid;
  pthread_mutex_t* lock, *ready_lock;
  pthread_cond_t* cond;
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
    larray_set(&thread_locks, idx, (void*)mp);
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

int _mutex_lock(lua_State* L){
  lua_pushstring(L, "_");
  lua_gettable(L, 1);
  pthread_mutex_t *lock = lua_touserdata(L, -1);

  pthread_mutex_lock(lock);

  return 0;
}

int _mutex_unlock(lua_State* L){
  lua_pushstring(L, "_");
  lua_gettable(L, 1);
  pthread_mutex_t *lock = lua_touserdata(L, -1);

  pthread_mutex_unlock(lock);

  return 0;
}

int _mutex_free(lua_State* L){
  lua_pushstring(L, "_");
  lua_gettable(L, 1);
  pthread_mutex_t *lock = lua_touserdata(L, -1);

  if(lock != NULL){
    pthread_mutex_destroy(&*lock);
    free(lock);

    luaI_tsetlud(L, 1, "_", NULL);
  }

  return 0;
}

int l_mutex(lua_State* L){
  pthread_mutex_t *lock = malloc(sizeof * lock);

  if(pthread_mutex_init(&*lock, NULL) != 0)
    luaI_error(L, -1, "mutex init failed");

  lua_newtable(L);
  int idx = lua_gettop(L);
  luaI_tsetlud(L, idx, "_", lock);
  luaI_tsetcf(L, idx, "lock", _mutex_lock);
  luaI_tsetcf(L, idx, "unlock", _mutex_unlock);
  luaI_tsetcf(L, idx, "free", _mutex_free);

  lua_newtable(L);
  int midx = lua_gettop(L);
  luaI_tsetcf(L, midx, "__gc", _mutex_free);

  lua_pushvalue(L, midx);
  lua_setmetatable(L, idx);
  lua_pushvalue(L, idx);

  return 1;
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

  pthread_mutex_unlock(&*info->lock);

  pthread_exit(NULL);
  p_error("thread did not exit");

  return 1;
}

void _thread_exit_signal(int i){
  pthread_exit(NULL);
}

void* handle_thread(void* _args){
  struct thread_info* args = (struct thread_info*)_args;
  lua_State* L = args->L;
  pthread_mutex_lock(&*args->lock);

#ifdef SUPPORTS_PTHREAD_CANCEL
  pthread_setcanceltype(PTHREAD_CANCEL_ASYNCHRONOUS, NULL);
#endif

  signal(SIGUSR1, _thread_exit_signal);

  pthread_detach(args->tid);
  //unlock main
  pthread_mutex_lock(&*args->ready_lock);
  pthread_cond_signal(&*args->cond);
  pthread_mutex_unlock(&*args->ready_lock);

  lua_newtable(L);
  int res_idx = lua_gettop(L);
  luaI_tsetlud(L, res_idx, "_", args);

  lua_newtable(L);
  int meta_idx = lua_gettop(L);
  luaI_tsetcf(L, meta_idx, "__call", l_res);
  lua_pushvalue(L, meta_idx);
  lua_setmetatable(L, res_idx);

  luaL_loadbuffer(L, args->function->c, args->function->len, "thread");
  int x = lua_gettop(L);
  str_free(args->function);
  args->function = NULL;

  lua_assign_upvalues(L, x);
  lua_pushvalue(L, res_idx);
  lua_call(L, 1, 0);
  args->done = 1;
  pthread_mutex_unlock(&*args->lock);

  return NULL;
}

int _thread_await(lua_State* L){
  lua_pushstring(L, "_");
  lua_gettable(L, 1);
  struct thread_info* info = lua_touserdata(L, -1);
  if(info->L == NULL) luaI_error(L, -1, "thread was already closed")
  if(info->tid == 0) luaI_error(L, -2, "thread was killed early")

  //maybe error here if tid is zero
  pthread_mutex_lock(&*info->lock);

  env_table(info->L, 0);
  luaI_deepcopy(info->L, L, SKIP_LOCALS);
  lua_pop(info->L, 1);
  env_table(L, 0);
  luaI_jointable(L);

  lua_setglobal(L, "_locals");

  for(int i = 0; i != info->return_count; i++){
    int ot = lua_gettop(info->L);

    lua_pushvalue(info->L, ot - info->return_count + i);

    luaI_deepcopy(info->L, L, 0);

    int type = lua_type(info->L, ot - info->return_count + i);
    if(type == LUA_TTABLE || type == LUA_TUSERDATA){
      lua_getmetatable(info->L, ot - info->return_count + i);
      int idx = lua_gettop(info->L);
      luaI_tsetnil(info->L, idx, "__gc");
    }

    lua_settop(info->L, ot);
  }

  lua_pushnil(L);
  lua_setglobal(L, "_locals");
  pthread_mutex_unlock(&*info->lock);

  return info->return_count;
}

int _thread_clean(lua_State* L){
  lua_pushstring(L, "_");
  lua_gettable(L, 1);
  struct thread_info* info = lua_touserdata(L, -1);
  if(info != NULL && info->L != NULL){
    luaI_tsetnil(L, 1, "_");

    if(info->tid != 0 && !info->done){
#ifdef SUPPORTS_PTHREAD_CANCEL
      pthread_cancel(info->tid);
#else
      pthread_kill(info->tid, SIGUSR1);
#endif
    }
 
    //lua_gc(info->L, LUA_GCRESTART);
    lua_gc(info->L, LUA_GCCOLLECT);

    lua_close(info->L);

    info->L = NULL;

    pthread_mutex_destroy(&*info->lock);
    free(info->lock);

    free(info);
  }
  return 0;
}


int _thread_close(lua_State* L){
#ifdef SUPPORTS_PTHREAD_CANCEL

  lua_pushstring(L, "_");
  lua_gettable(L, 1);
  struct thread_info* info = lua_touserdata(L, -1);

  if(info->tid != 0) pthread_cancel(info->tid);
  info->tid = 0;

  return 0;
#else
  return _thread_kill(L);
#endif
}


int _thread_kill(lua_State* L){
  lua_pushstring(L, "_");
  lua_gettable(L, 1);
  struct thread_info* info = lua_touserdata(L, -1);

  if(info->tid != 0) pthread_kill(info->tid, SIGUSR1);
  info->tid = 0;

  return 0;
}

int l_async(lua_State* oL){
  lua_State* L = luaL_newstate(); 
  lua_gc(L, LUA_GCSTOP);

  luaL_openlibs(L);
  luaI_copyvars(oL, L);

  struct thread_info* args = calloc(1, sizeof * args);
  args->L = L;
  args->lock = malloc(sizeof * args->lock);
  pthread_mutex_init(&*args->lock, NULL);
  args->ready_lock = malloc(sizeof * args->ready_lock);
  pthread_mutex_init(&*args->ready_lock, NULL);
  args->return_count = 0;
  args->cond = malloc(sizeof * args->cond);
  pthread_cond_init(&*args->cond, NULL);

  args->function = str_init("");
  lua_pushvalue(oL, 1);
  lua_dump(oL, writer, (void*)args->function, 0);

  pthread_mutex_lock(&*args->ready_lock);

  pthread_create(&args->tid, NULL, handle_thread, (void*)args);

  pthread_cond_wait(&*args->cond, &*args->ready_lock);
  pthread_mutex_unlock(&*args->ready_lock);

  pthread_cond_destroy(&*args->cond);
  free(args->cond);
  pthread_mutex_destroy(&*args->ready_lock);
  free(args->ready_lock);

  lua_newtable(oL);
  int res_idx = lua_gettop(oL);
  luaI_tsetcf(oL, res_idx, "await", _thread_await);
  luaI_tsetcf(oL, res_idx, "clean", _thread_clean);
  luaI_tsetcf(oL, res_idx, "kill", _thread_kill);
  luaI_tsetcf(oL, res_idx, "close", _thread_close);
  luaI_tsetlud(oL, res_idx, "_", args);

  lua_newtable(oL);
  int meta_idx = lua_gettop(oL);
  luaI_tsetcf(oL, meta_idx, "__gc", _thread_clean);

  lua_pushvalue(oL, meta_idx);
  lua_setmetatable(oL, res_idx);
  lua_pushvalue(oL, res_idx);
  return 1;
}

struct thread_buffer {
  lua_State* L;
  pthread_mutex_t* lock;
};

int _buffer_get(lua_State* L){
  struct thread_buffer *buffer = lua_touserdata(L, 1);
  pthread_mutex_lock(&*buffer->lock);
  luaI_deepcopy(buffer->L, L, SKIP_GC | SKIP_LOCALS);
  pthread_mutex_unlock(&*buffer->lock);
  return 1;
}

int _buffer_set(lua_State* L){
  struct thread_buffer *buffer = lua_touserdata(L, 1);
  pthread_mutex_lock(&*buffer->lock);
  lua_settop(buffer->L, 0);
  luaI_deepcopy(L, buffer->L, SKIP_LOCALS);
  pthread_mutex_unlock(&*buffer->lock);

  lua_getmetatable(L, 2);
  int idx = lua_gettop(L);
  luaI_tsetnil(L, idx, "__gc");

  return 1;
}

#include <assert.h>
_Atomic int used = 0;

int _buffer_mod(lua_State* L){
  struct thread_buffer *buffer = lua_touserdata(L, 1);
  pthread_mutex_lock(&*buffer->lock);
  //printf("%p\n", &*buffer->lock);
  assert(used == 0);
  used = 1;

  luaI_deepcopy(buffer->L, L, SKIP_GC | SKIP_LOCALS);
  int item = lua_gettop(L);
  lua_pushvalue(L, 2);
  lua_pushvalue(L, item);
  lua_call(L, 1, 1);

  if(lua_type(L, -1) != LUA_TNIL){
    int idx = lua_gettop(L);
    lua_settop(buffer->L, 0);
    luaI_deepcopy(L, buffer->L, SKIP_LOCALS);

    lua_getmetatable(L, idx);
    idx = lua_gettop(L);
    luaI_tsetnil(L, idx, "__gc");
  }

  used = 0;

  pthread_mutex_unlock(&*buffer->lock);
  return 1;
}


int l_buffer_index(lua_State* L){
  uint64_t len, hash;
  struct thread_buffer *buffer = lua_touserdata(L, 1);
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
      lua_pushstring(buffer->L, str);
      lua_gettable(buffer->L, 1);
      if(lua_isnil(buffer->L, -1)){
        lua_pushnil(L);
        return 1;
      }

      luaI_deepcopy(buffer->L, L, SKIP_GC | SKIP_LOCALS);
      lua_pop(buffer->L, 1);
      break;
  }
  return 1;
}

int hi(lua_State* L){
  printf("hi\n");
  return 0;
}

//not thread safe yet
int meta_proxy(lua_State* L){
  int argc = lua_gettop(L);
  struct thread_buffer *buffer = lua_touserdata(L, 1);

  lua_getmetatable(buffer->L, 1);
  lua_pushstring(buffer->L, lua_tostring(L, 2));
  lua_gettable(buffer->L, 2);

  lua_pushvalue(buffer->L, 1);

  int count = 0;
  for(int i = 4; i <= argc; i++){
    count++;
    lua_pushvalue(L, i);
    luaI_deepcopy(L, buffer->L, SKIP_GC | SKIP_LOCALS);
  }

  //printf("%i\n",count);
  lua_call(buffer->L, count + 1, 1);
  luaI_deepcopy(buffer->L, L, SKIP_LOCALS);

  lua_pushnil(buffer->L);
  lua_setmetatable(buffer->L, -2);

  lua_settop(buffer->L, 1);
  //printf("%p\n", lua_topointer(buffer->L, -1));
  return 1;
}

void meta_proxy_gen(lua_State* L, struct thread_buffer *buffer, int meta_idx, int new_meta_idx){  

  lua_pushcfunction(L, meta_proxy); 
  lua_setglobal(L, "__proxy_call");

  lua_pushlightuserdata(L, buffer);
  lua_setglobal(L, "__this_obj");

  lua_pushcfunction(L, l_unpack);
  lua_setglobal(L, "__unpack");

  lua_pushnil(L);
  for(; lua_next(L, meta_idx) != 0;){
    int k = lua_gettop(L);
    k = lua_gettop(L) - 1;

    char* fn = calloc(128, sizeof * fn); 
    const char* key = lua_tostring(L, k);
    sprintf(fn, "return function(...)\
        return __proxy_call(__this_obj,'%s',...);end", key);
    luaL_dostring(L, fn);

    free(fn);
    int nf = lua_gettop(L);

    luaI_tsetv(L, new_meta_idx, lua_tostring(L, k), nf);

    lua_pop(L, 2);
  }
}

int l_buffer_gc(lua_State* L){
  struct thread_buffer *buffer = lua_touserdata(L, 1);
  pthread_mutex_lock(&*buffer->lock);
  pthread_mutex_unlock(&*buffer->lock);
  //race condition here, if something can manage to lock the thread between these two lines
  //add maybe a closing variable thats checked for
  pthread_mutex_destroy(&*buffer->lock);
  free(buffer->lock);

  lua_close(buffer->L);
  return 0;
}

int l_buffer(lua_State* L){ 
  int use = lua_getmetatable(L, 1);
  int old_meta_idx = lua_gettop(L);

  struct thread_buffer *buffer = lua_newuserdata(L, sizeof * buffer);
  int buffer_idx = lua_gettop(L);

  buffer->L = luaL_newstate();
  lua_gc(buffer->L, LUA_GCSTOP);
  buffer->lock = malloc(sizeof * buffer->lock);
  if(pthread_mutex_init(&*buffer->lock, NULL) != 0) p_fatal("pthread_mutex_init failed");
  lua_pushvalue(L, 1);
  luaI_deepcopy(L, buffer->L, SKIP_LOCALS);

  lua_newtable(L);
  int meta_idx = lua_gettop(L);
  if(use!=0) meta_proxy_gen(L, buffer, old_meta_idx, meta_idx);
  luaI_tsetcf(L, meta_idx, "__index", l_buffer_index);
  luaI_tsetcf(L, meta_idx, "__gc", l_buffer_gc);

  if(use != 0){
    lua_getmetatable(L, 1);
    int idx = lua_gettop(L);
    luaI_tsetnil(L, idx, "__gc");
  }

  lua_pushvalue(L, meta_idx);
  lua_setmetatable(L, buffer_idx);
  lua_pushvalue(L, buffer_idx);
  return 1;
}

void _lua_getfenv(lua_State* L){

}

int l_testcopy(lua_State* L){ 
  lua_State* temp = luaL_newstate();
  luaI_deepcopy(L, temp, SKIP_GC);
  lua_close(temp);
  return 1;
}
