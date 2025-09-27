#ifndef __common_net_h
#define __common_net_h

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

enum file_status {
  _ignore, BARRIER_READ, FILE_HEADER, FILE_BODY, NORMAL
};

struct file_parse {
  enum file_status status;
  str *current, *old, *boundary, *boundary_id;
  int dash_count, table_idx;
};

typedef struct {
  int fd, ser;
  int port;
  lua_State* L;
  struct sockaddr_in cli;
  parray_t* paths;
} thread_arg_struct;

struct lchar {
  char* c;
  int len;
  char req[20];
};

struct net_server_state {
  int event_fd;
};

struct sarray_t {
  struct lchar** cs;
  int len;
};

extern map_t* mime_type;

int start_serv(lua_State* L, int port, parray_t* paths, struct net_server_state*);

enum {
  NETEV_NULL = 0,
  NETEV_DEFAULT = 1,
  NETEV_CLOSE_EVENT = 2,
};

#endif
