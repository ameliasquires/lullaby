#ifdef _WIN32 //add -lws2_32
  #include <winsock2.h>
  //#define socklen_t __socklen_t
  //#define close closesocket
  typedef int socklen_t;
#else
  #include <sys/socket.h>
  #include <arpa/inet.h>
#define closesocket close
#endif

#include "lua.h"
#include "config.h"
#include "types/str.h"
#include "types/parray.h"
#include <stdint.h>

int l_listen(lua_State*);

int l_request(lua_State*);
int l_srequest(lua_State*);
int l_wss(lua_State*);


int64_t recv_full_buffer(int client_fd, char** _buffer, int* header_eof, int* state);

int parse_header(char* buffer, int header_eof, parray_t** _table);

void i_write_header(lua_State* L, int header_top, str** _resp, char* content, size_t len);

void client_fd_errors(int client_fd);

int content_disposition(str* src, parray_t** _dest);

//int rolling_file_parse(lua_State* L, int* files_idx, int* body_idx, char* buffer, str* content_type, size_t blen, struct file_parse* _content);

void* handle_client(void *_arg);

int start_serv(lua_State* L, int port);

//
static char* http_codes[600] = {0};

int clean_lullaby_net(lua_State* L);

static const luaL_Reg net_function_list [] = {
  {"listen",l_listen},
  {"request",l_request},
  {"srequest",l_srequest},
  {"wss",l_wss},
  
  {NULL,NULL}
};

extern char* _mimetypes;
extern uint64_t _mimetypes_len;

static struct config net_config[] = {
  {.name = "mimetypes", .type = c_string, .value = {.c_string = &_mimetypes, .len = &_mimetypes_len}},
  {.type = c_none}
};
