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
  int return_count, done, request_close;
  pthread_t tid;
  pthread_mutex_t* lock, *ready_lock, *close_lock;
  pthread_cond_t* cond;
};

#include "io.h"

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

  lua_newtable(L);
  int idx = lua_gettop(L);

  for(int i = info->return_count - 1; i != -1; i--){
    lua_pushinteger(L, lua_rawlen(L, idx) + 1);
    lua_pushvalue(L, i + 2);
    lua_settable(L, idx);
  }

  env_table(L, 0);
  lua_setglobal(L, "_res_locals");

  lua_pushvalue(L, idx);
  lua_setglobal(L, "_return_table");

  info->done = 1;
  pthread_mutex_unlock(&*info->lock);

  pthread_exit(NULL);
  p_error("thread did not exit");

  return 1;
}

int _res_testclose(lua_State* L){
  lua_pushstring(L, "_");
  lua_gettable(L, 1);
  struct thread_info* info = lua_touserdata(L, -1);

  pthread_mutex_lock(&*info->close_lock);
  pthread_cond_signal(&*info->cond);

  if(info->request_close){
    info->done = 1;

    pthread_mutex_unlock(&*info->lock);
    pthread_mutex_unlock(&*info->close_lock);

    pthread_exit(NULL);
  }

  pthread_mutex_unlock(&*info->close_lock);

  return 0;
}

void _res_testclose_debug(lua_State* L, lua_Debug* d){
  _res_testclose(L);
}

int _res_autoclose(lua_State* L){
  lua_sethook(L, _res_testclose_debug, LUA_HOOKLINE, 1);
  return 0;
}

void _thread_exit_signal(int i){
  pthread_exit(NULL);
}

void* handle_thread(void* _args){
  struct thread_info* args = (struct thread_info*)_args;
  lua_State* L = args->L;
  pthread_mutex_lock(&*args->lock);

  signal(SIGUSR1, _thread_exit_signal);

  pthread_detach(args->tid);
  //unlock main
  pthread_mutex_lock(&*args->ready_lock);
  pthread_cond_signal(&*args->cond);
  pthread_mutex_unlock(&*args->ready_lock);

  lua_newtable(L);
  int res_idx = lua_gettop(L);
  luaI_tsetcf(L, res_idx, "testclose", _res_testclose);
  luaI_tsetcf(L, res_idx, "autoclose", _res_autoclose);
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

  pthread_mutex_lock(&*args->close_lock);
  pthread_cond_signal(&*args->cond);
  pthread_mutex_unlock(&*args->close_lock);

  return NULL;
}

int _thread_await(lua_State* L){
  lua_pushstring(L, "_");
  lua_gettable(L, 1);
  struct thread_info* info = lua_touserdata(L, -1);
  if(info->L == NULL) luaI_error(L, -1, "thread was already closed")
  if(info->tid == 0) luaI_error(L, -2, "thread was killed early")

  pthread_mutex_lock(&*info->lock);

  if(info->return_count == 0) return 0;
  lua_getglobal(info->L, "_res_locals");
  luaI_deepcopy(info->L, L, SKIP_LOCALS | STRIP_GC);
  env_table(L, 0);

  luaI_jointable(L);

  lua_setglobal(L, "_locals");

  lua_getglobal(info->L, "_return_table");
  int idx = lua_gettop(info->L);

  for(int i = info->return_count; i != 0; i--){
    lua_pushinteger(info->L, i);
    lua_gettable(info->L, idx);
    luaI_deepcopy(info->L, L, STRIP_GC);
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
      pthread_kill(info->tid, SIGUSR1);
    }
 
    //lua_gc(info->L, LUA_GCRESTART);
    lua_gc(info->L, LUA_GCCOLLECT);

    lua_close(info->L);

    info->L = NULL;

    pthread_mutex_destroy(&*info->lock);
    free(info->lock);
    pthread_mutex_destroy(&*info->close_lock);
    free(info->close_lock);

    pthread_cond_destroy(&*info->cond);
    free(info->cond);

    free(info);
  }
  return 0;
}

int _thread_kill(lua_State* L){
  lua_pushstring(L, "_");
  lua_gettable(L, 1);
  struct thread_info* info = lua_touserdata(L, -1);

  if(info->tid != 0){
    pthread_kill(info->tid, SIGUSR1);
    pthread_mutex_lock(&*info->close_lock);
    pthread_cond_signal(&*info->cond);
    pthread_mutex_unlock(&*info->close_lock);
  }
  info->tid = 0;

  return 0;
}

int _thread_close(lua_State* L){
  lua_pushstring(L, "_");
  lua_gettable(L, 1);
  struct thread_info* info = lua_touserdata(L, -1);

  if(info->tid == 0) return 0;
  
  pthread_mutex_lock(&*info->close_lock);

  info->request_close = 1;

  pthread_cond_wait(&*info->cond, &*info->close_lock);
  pthread_mutex_unlock(&*info->close_lock);

  info->tid = 0;

  return 0;
}

int l_async(lua_State* oL){
  lua_State* L = luaL_newstate(); 
  lua_gc(L, LUA_GCSTOP);

  luaL_openlibs(L);
  luaI_copyvars(oL, L);
  luaL_openlibs(L);

  struct thread_info* args = calloc(1, sizeof * args);
  args->L = L;
  args->lock = malloc(sizeof * args->lock);
  pthread_mutex_init(&*args->lock, NULL);
  args->ready_lock = malloc(sizeof * args->ready_lock);
  pthread_mutex_init(&*args->ready_lock, NULL);
  args->close_lock = malloc(sizeof * args->close_lock);
  pthread_mutex_init(&*args->close_lock, NULL);

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
  luaI_deepcopy(L, buffer->L, SKIP_LOCALS | STRIP_GC);
  pthread_mutex_unlock(&*buffer->lock);

  return 1;
}

int _buffer_mod(lua_State* L){
  struct thread_buffer *buffer = lua_touserdata(L, 1);
  pthread_mutex_lock(&*buffer->lock);

  luaI_deepcopy(buffer->L, L, SKIP_GC | SKIP_LOCALS);
  int item = lua_gettop(L);
  lua_pushvalue(L, 2);
  lua_pushvalue(L, item);
  lua_call(L, 1, 1);

  if(lua_type(L, -1) != LUA_TNIL){
    int idx = lua_gettop(L);
    lua_settop(buffer->L, 0);
    luaI_deepcopy(L, buffer->L, STRIP_GC | SKIP_LOCALS);

    lua_getmetatable(L, idx);
    idx = lua_gettop(L);
    luaI_tsetnil(L, idx, "__gc");
  }

  pthread_mutex_unlock(&*buffer->lock);
  return 1;
}


int l_buffer_index(lua_State* L){
  uint64_t len, hash;
  struct thread_buffer *buffer = lua_touserdata(L, 1);
  const char* str = luaL_tolstring(L, 2, &len);

  hash = fnv_1((uint8_t*)str, len, v_1);

  //maybe strcmp after the hash has been verified?
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

int meta_proxy(lua_State* L){
  int argc = lua_gettop(L);
  struct thread_buffer *buffer = lua_touserdata(L, 1);
  pthread_mutex_lock(&*buffer->lock);

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
  luaI_deepcopy(buffer->L, L, SKIP_LOCALS | STRIP_GC);

  lua_pushnil(buffer->L);
  lua_setmetatable(buffer->L, -2);

  lua_settop(buffer->L, 1);
  pthread_mutex_unlock(&*buffer->lock);
  //printf("%p\n", lua_topointer(buffer->L, -1));
  return 1;
}

#warning "make this reapply for new objects!"
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
  luaI_deepcopy(L, buffer->L, SKIP_LOCALS | STRIP_GC);

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

int l_usleep(lua_State* L){
  uint64_t n = lua_tonumber(L, 1);
  usleep(n);

  return 0;
}

int l_sleep(lua_State* L){
  double n = lua_tonumber(L, 1);
  usleep(n * 1000 * 1000);

  return 0;
}

int l_testcopy(lua_State* L){ 
  lua_State* temp = luaL_newstate();
  luaI_deepcopy(L, temp, SKIP_GC);
  lua_close(temp);
  return 1;
}
