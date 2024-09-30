#ifndef __common_net_h
#define __common_net_h

#include "lua5.4/lauxlib.h"
#include "lua5.4/lua.h"

#include <sys/types.h>
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <string.h>
#include <unistd.h>
#include <sys/stat.h>
#include <errno.h>

#include "../net.h"
#include "../lua.h"
#include "../io.h"
#include "../table.h"
#include "../types/str.h"
#include "../types/parray.h"
#include "../types/map.h"

#define max_con 200
//2^42
#define MAX_HEADER_SIZE (1<<20)
#define BUFFER_SIZE 20000
#define HTTP_BUFFER_SIZE 4098
#define max_content_length 200000

static int ports[65535] = { 0 };
static parray_t* paths = NULL;

enum file_status {
  _ignore, BARRIER_READ, FILE_HEADER, FILE_BODY, NORMAL
};

struct file_parse {
  enum file_status status;
  str *current, *old, *boundary, *boundary_id;
  int dash_count, table_idx;
};

typedef struct {
  int fd;
  int port;
  lua_State* L;
  struct sockaddr_in cli;
} thread_arg_struct;

struct lchar {
  char* c;
  int len;
  char req[20];
};

struct sarray_t {
  struct lchar** cs;
  int len;
};

static pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
static pthread_mutex_t con_mutex = PTHREAD_MUTEX_INITIALIZER;
static pthread_mutex_t lua_mutex = PTHREAD_MUTEX_INITIALIZER;

extern map_t* mime_type;

#endif
