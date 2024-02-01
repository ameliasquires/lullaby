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

#include <sys/types.h>
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <string.h>
#include <unistd.h>
#include "net.h"
#include "lua.h"

#include "io.h"
#include "table.h"
#include "i_str.h"
#include "parray.h"

#define max_con 200
#define BUFFER_SIZE 2048

static int ports[65535] = { 0 };
static parray_t* paths = NULL;

struct lchar {
  char* c;
  int len;
};

struct sarray_t {
  struct lchar** cs;
  int len;
};

pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t lua_mutex = PTHREAD_MUTEX_INITIALIZER;

size_t recv_full_buffer(int client_fd, char** _buffer, int* header_eof){
  char* buffer = malloc(BUFFER_SIZE * sizeof * buffer);
  memset(buffer, 0, BUFFER_SIZE);
  char* header;
  size_t len = 0;
  *header_eof = -1;
  int n;

  for(;;){
    n = recv(client_fd, buffer + len, BUFFER_SIZE, 0);
    if(*header_eof == -1 && (header = strstr(buffer, "\r\n\r\n")) != NULL){
      *header_eof = header - buffer;
    }
    if(n != BUFFER_SIZE) break;
    len += BUFFER_SIZE;
    buffer = realloc(buffer, len + BUFFER_SIZE);
    memset(buffer + len, 0, BUFFER_SIZE);
  }
  //buffer[len - 1] = 0;

  *_buffer = buffer;
  return len + BUFFER_SIZE;
}

int parse_header(char* buffer, int header_eof, str*** _table, int* _len){
  if(header_eof == -1) return -1;
  char add[] = {0,0};
  int lines = 3;
  for(int i = 0; i != header_eof; i++) lines += buffer[i] == '\n';
  str** table = malloc(sizeof ** table * lines * 2);
  table[0] = str_init("Request");// table[1] = str_init("Post|Get");
  table[2] = str_init("Path");// table[3] = str_init("/");
  table[4] = str_init("Version");// table[5] = str_init("HTTP/1.1");
  str* current = str_init("");
  int ins = 1;
  int oi = 0;
  for(; oi != header_eof; oi++){
    add[0] = buffer[oi];
    if(buffer[oi] == '\n') break;
    if(buffer[oi] == ' '){
      table[ins] = str_init(current->c);
      ins += 2;
      str_clear(current);
    } else str_push(current, add);
  }
  current->c[current->len - 1] = 0;
  table[ins] = str_init(current->c);
  str_clear(current);
  int tlen = 6;

  int key = 1;
  for(int i = oi + 1; i != header_eof; i++){
    if(key && buffer[i]==':' || !key && buffer[i]=='\n') {
      if(!key) current->c[current->len - 1] = 0;
      table[tlen] = str_init(current->c);
      str_clear(current);
      tlen++;
      i+=key;
      key = !key;
      continue;
    }
    add[0] = buffer[i];
    str_push(current, add);
  }
  table[tlen] = str_init(current->c);
  tlen++;
  str_free(current);
  *_len = tlen / 2;
  *_table = table; 
  return 0;
}

int stable_key(str** table, char* target, int flen){
  for(int i = 0; i != flen * 2; i+=2){
    if(strcmp(table[i]->c,target) == 0){
      return i + 1;
    }
  }
  return -1;
}

void http_build(str** _dest, int code, char* code_det, char* header_vs, char* content){
  /**dest = str_init(
    "HTTP/1.1 404 Not Found\r\n"
    "Content-Type: text/plain\r\n"
    "\r\n"
    "test"
  );*/
  char* dest = malloc(BUFFER_SIZE);
  memset(dest, 0, BUFFER_SIZE);
  sprintf(dest, 
    "HTTP/1.1 %i %s\r\n"
    "%s"
    "\r\n"
    , code, code_det, header_vs);

  *_dest = str_init(dest);
  str_push(*_dest, content);
  free(dest);
}

typedef struct {
  int fd;
  int port;
  lua_State* L;
} thread_arg_struct;

void http_code(int code, char* code_det){
  //this was done with a script btw
  switch(code){
    case 100: sprintf(code_det,"Continue"); break;
    case 101: sprintf(code_det,"Switching Protocols"); break;
    case 102: sprintf(code_det,"Processing"); break;
    case 103: sprintf(code_det,"Early Hints"); break;
    case 200: sprintf(code_det,"OK"); break;
    case 201: sprintf(code_det,"Created"); break;
    case 202: sprintf(code_det,"Accepted"); break;
    case 203: sprintf(code_det,"Non-Authoritative Information"); break;
    case 204: sprintf(code_det,"No Content"); break;
    case 205: sprintf(code_det,"Reset Content"); break;
    case 206: sprintf(code_det,"Partial Content"); break;
    case 207: sprintf(code_det,"Multi-Status"); break;
    case 208: sprintf(code_det,"Already Reported"); break;
    case 226: sprintf(code_det,"IM Used"); break;
    case 300: sprintf(code_det,"Multiple Choices"); break;
    case 301: sprintf(code_det,"Moved Permanently"); break;
    case 302: sprintf(code_det,"Found"); break;
    case 303: sprintf(code_det,"See Other"); break;
    case 304: sprintf(code_det,"Not Modified"); break;
    case 307: sprintf(code_det,"Temporary Redirect"); break;
    case 308: sprintf(code_det,"Permanent Redirect"); break;
    case 400: sprintf(code_det,"Bad Request"); break;
    case 401: sprintf(code_det,"Unauthorized"); break;
    case 402: sprintf(code_det,"Payment Required"); break;
    case 403: sprintf(code_det,"Forbidden"); break;
    case 404: sprintf(code_det,"Not Found"); break;
    case 405: sprintf(code_det,"Method Not Allowed"); break;
    case 406: sprintf(code_det,"Not Acceptable"); break;
    case 407: sprintf(code_det,"Proxy Authentication Required"); break;
    case 408: sprintf(code_det,"Request Timeout"); break;
    case 409: sprintf(code_det,"Conflict"); break;
    case 410: sprintf(code_det,"Gone"); break;
    case 411: sprintf(code_det,"Length Required"); break;
    case 412: sprintf(code_det,"Precondition Failed"); break;
    case 413: sprintf(code_det,"Content Too Large"); break;
    case 414: sprintf(code_det,"URI Too Long"); break;
    case 415: sprintf(code_det,"Unsupported Media Type"); break;
    case 416: sprintf(code_det,"Range Not Satisfiable"); break;
    case 417: sprintf(code_det,"Expectation Failed"); break;
    case 418: sprintf(code_det,"I'm a teapot"); break;
    case 421: sprintf(code_det,"Misdirected Request"); break;
    case 422: sprintf(code_det,"Unprocessable Content"); break;
    case 423: sprintf(code_det,"Locked"); break;
    case 424: sprintf(code_det,"Failed Dependency"); break;
    case 425: sprintf(code_det,"Too Early"); break;
    case 426: sprintf(code_det,"Upgrade Required"); break;
    case 428: sprintf(code_det,"Precondition Required"); break;
    case 429: sprintf(code_det,"Too Many Requests"); break;
    case 431: sprintf(code_det,"Request Header Fields Too Large"); break;
    case 451: sprintf(code_det,"Unavailable For Legal Reasons"); break;
    case 500: sprintf(code_det,"Internal Server Error"); break;
    case 501: sprintf(code_det,"Not Implemented"); break;
    case 502: sprintf(code_det,"Bad Gateway"); break;
    case 503: sprintf(code_det,"Service Unavailable"); break;
    case 504: sprintf(code_det,"Gateway Timeout"); break;
    case 505: sprintf(code_det,"HTTP Version Not Supported"); break;
    case 506: sprintf(code_det,"Variant Also Negotiates"); break;
    case 507: sprintf(code_det,"Insufficient Storage"); break;
    case 508: sprintf(code_det,"Loop Detected"); break;
    case 510: sprintf(code_det,"Not Extended"); break;
    case 511: sprintf(code_det,"Network Authentication Required"); break;
    default: sprintf(code_det,"unknown");
  }
}

int l_send(lua_State* L){
  int res_idx = 1;
  
  lua_pushvalue(L, res_idx);
  lua_pushstring(L, "client_fd");
  lua_gettable(L, res_idx);
  int client_fd = luaL_checkinteger(L, -1);
 
  /*
  lua_pushvalue(L, res_idx);
  lua_pushstring(L, "Content");
  lua_gettable(L, res_idx);*/
  char* content = (char*)luaL_checkstring(L, 2);
  
  lua_pushvalue(L, res_idx);
  lua_pushstring(L, "header");
  lua_gettable(L, -2);
  int header = lua_gettop(L);

  str* header_vs = str_init("");
  lua_pushnil(L);
  for(;lua_next(L, header) != 0;){
      char* key = (char*)luaL_checklstring(L, -2, NULL);
      if(strcmp(key, "Code") != 0){
        str_push(header_vs, key);
        str_push(header_vs, ": ");
        str_push(header_vs, (char*)luaL_checklstring(L, -2, NULL));
        str_push(header_vs, "\r\n");
      }
      lua_pop(L, 1);
  }

  lua_pushvalue(L, header);
  lua_pushstring(L, "Code");
  lua_gettable(L, header);
  int code = luaL_checkinteger(L, -1);
  str* resp;
  
  char code_det[50] = {0};
  http_code(code, code_det);
  http_build(&resp, code,  code_det, header_vs->c, content);
  send(client_fd, resp->c, resp->len, 0);
  str_free(resp);
  str_free(header_vs);
  return 0;
}

volatile size_t threads = 0;
void* handle_client(void *_arg){
  //pthread_mutex_lock(&mutex);
  thread_arg_struct* args = (thread_arg_struct*)_arg;
  int client_fd = args->fd;
  char* buffer;
  char dummy[2] = {0, 0};
  int header_eof;

  //create state for this thread
  lua_State* L = luaL_newstate();
  luaL_openlibs(L);

  //read full request
  size_t bytes_received = recv_full_buffer(client_fd, &buffer, &header_eof);

  //if the buffer, yknow exists
  if(bytes_received > 0){
    str** table;
    int len = 0;
    //checks for a valid header
    if(parse_header(buffer, header_eof, &table, &len) != -1){
      int k = stable_key(table, "Path", len);

      char portc[10] = {0};
      sprintf(portc, "%i", args->port);

      str* aa = str_init(portc);
      str_push(aa, table[k]->c);

      void* v = parray_get(paths, aa->c);

      if(v == NULL){
        str* resp;
        http_build(&resp, 404, "Not Found","text/html", "<h1>404</h1>");
        send(client_fd, resp->c, resp->len, 0);
        str_free(resp);
      } else {
        lua_newtable(L);
        lua_newtable(L);
        for(int i = 0; i != len * 2; i+=2){
          //printf("'%s' :: '%s'\n",table[i]->c, table[i+1]->c);
          lua_pushstring(L, table[i]->c);
          lua_pushstring(L, table[i+1]->c);
          lua_settable(L, -3);
        }
        
        int req_idx = lua_gettop(L);
        lua_newtable(L);
        int res_idx = lua_gettop(L);
        //functions
        lua_pushstring(L, "send");
        lua_pushcfunction(L, l_send);
        lua_settable(L, -3);

        //values
        lua_pushstring(L, "client_fd");
        lua_pushinteger(L, client_fd);
        lua_settable(L, -3);
  
        //header table
        lua_newtable(L);
        lua_pushstring(L, "Code");
        lua_pushinteger(L, 200);
        lua_settable(L, -3);

        lua_pushstring(L, "Content-Type");
        lua_pushstring(L, "text/html");
        lua_settable(L, -3);
        
        lua_pushstring(L, "header");
        lua_pushvalue(L, -2);
        lua_settable(L, res_idx);

        //the function(s)
        struct sarray_t* awa = (struct sarray_t*)v;
        for(int i = 0; i != awa->len; i++){
          struct lchar* wowa = awa->cs[i];
          luaL_loadbuffer(L, wowa->c, wowa->len, wowa->c);

          int func = lua_gettop(L);

          lua_pushvalue(L, func); // push function call
          lua_pushvalue(L, res_idx); //push methods related to dealing with the request
          lua_pushvalue(L, req_idx); //push info about the request

          //call the function
          lua_call(L, 2, 0);
        }
      }
      
    }

    for(int i = 0; i != len; i++){
      str_free(table[i]);
    }
    free(table);
  }

  closesocket(client_fd);
  free(args);
  free(buffer);
  lua_close(L);

  pthread_mutex_lock(&mutex);
  threads--;
  pthread_mutex_unlock(&mutex);
  return NULL;
}

int start_serv(lua_State* L, int port){
  //need these on windows for sockets (stupid)
#ifdef _WIN32
  WSADATA Data;
  WSAStartup(MAKEWORD(2, 2), &Data);
#endif

  int server_fd;
  struct sockaddr_in server_addr;

  //open the socket
  if((server_fd = socket(AF_INET, SOCK_STREAM, 0)) < 0){
    printf("error opening socket\n");
    abort();
  }

  server_addr.sin_family = AF_INET;
  server_addr.sin_addr.s_addr = INADDR_ANY;
  server_addr.sin_port = htons(port);

  //bind to port
  if(bind(server_fd, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0){
    printf("failed to bind to port\n");
    abort();
  }

  if(listen(server_fd, max_con) < 0){
    printf("failed to listen\n");
    abort();
  }
  /*
  lua_rawgeti(L, LUA_REGISTRYINDEX, ports[port]);
  lua_pushstring(L, "/");
  lua_gettable(L, -2);
  lua_pushstring(L, "fn");
  lua_gettable(L, -2);
  int aa = lua_gettop(L);*/
  if (pthread_mutex_init(&mutex, NULL) != 0)
    printf("mutex init failed\n");

  for(;;){
    struct sockaddr_in client_addr;
    socklen_t client_addr_len = sizeof(client_addr);
    int* client_fd = malloc(sizeof(int));

    if((*client_fd = accept(server_fd, (struct sockaddr*)&client_addr, &client_addr_len)) < 0){
      printf("failed to accept\n");
      abort();
    }
    //printf("%i\n",threads);
    //open a state to call shit, should be somewhat thread safe

    thread_arg_struct* args = malloc(sizeof * args);
    args->fd = *client_fd;
    args->port = port;
    //args->L = oL;

    pthread_mutex_lock(&mutex);
    threads++;
    pthread_mutex_unlock(&mutex);

    //send request to handle_client()
    pthread_t thread_id;
    pthread_create(&thread_id, NULL, handle_client, (void*)args);
    pthread_detach(thread_id);
    
  }

}

int l_GET(lua_State* L){
  lua_pushstring(L, "port");
  lua_gettable(L, 1);
  int port = luaL_checkinteger(L, -1);

  char portc[10] = {0};
  sprintf(portc, "%i%s", port, lua_tostring(L, 2));

  
  lua_getglobal(L, "string");
  lua_pushstring(L, "dump");
  lua_gettable(L, -2);
  lua_pushvalue(L, 3);
  lua_call(L, 1, 1);

  size_t len;
  char* a = (char*)luaL_checklstring(L, -1, &len);
  struct lchar* awa = malloc(len + 1);
  awa->c = a;
  awa->len = len;

  if(paths == NULL)
    paths = parray_init();
    
  //please free this
  void* v_old_paths = parray_get(paths, portc);
  struct sarray_t* old_paths;
  if(v_old_paths == NULL){
    old_paths = malloc(sizeof * old_paths);
    old_paths->len = 0;
    old_paths->cs = malloc(sizeof * old_paths->cs);
  } else old_paths = (struct sarray_t*)v_old_paths;

  old_paths->len++;
  old_paths->cs = realloc(old_paths->cs, sizeof * old_paths->cs * old_paths->len);
  old_paths->cs[old_paths->len - 1] = awa;

  parray_set(paths, portc, (void*)old_paths);
  return 1;
}

int l_lock(lua_State* L){
  pthread_mutex_lock(&lua_mutex);
  return 0;
}

int l_unlock(lua_State* L){
  pthread_mutex_unlock(&lua_mutex);
  return 0;
}

int l_listen(lua_State* L){
  lua_State* src = luaL_newstate();
  lua_State* dest = luaL_newstate();


  if(lua_gettop(L) != 2) {
    printf("not enough args");
    abort();
  }
  if(lua_type(L, 1) != LUA_TFUNCTION) {
    printf("expected a function at arg 1");
    abort();
  }
  int port = luaL_checkinteger(L, 2);
  
  lua_newtable(L);
  lua_pushstring(L, "GET");
  lua_pushcfunction(L, l_GET);
  lua_settable(L, -3);

  lua_pushstring(L, "lock");
  lua_pushcfunction(L, l_lock);
  lua_settable(L, -3);
  
  lua_pushstring(L, "unlock");
  lua_pushcfunction(L, l_unlock);
  lua_settable(L, -3);
  
  lua_pushstring(L, "port");
  lua_pushvalue(L, 2);
  lua_settable(L, -3);

  lua_pushvalue(L, 1); //the function
  lua_pushvalue(L, -2); //the server table

  lua_pcall(L, 1, 0, 0);

  start_serv(L, port);
  return 0;
}

void* hh(void* _L){

  lua_State* L = (lua_State*)_L;
  lua_call(L, 0, 0);

  return NULL;
}

int l_spawn(lua_State* L){
  lua_getglobal(L, "string");
  lua_pushstring(L, "dump");
  lua_gettable(L, -2);
  lua_pushvalue(L, 1);
  lua_call(L, 1, 1);

  size_t len;
  char* a = (char*)luaL_checklstring(L, -1, &len);
  //luaL_loadbuffer(L, a, len, a);
  //lua_call(L,0,0);

  lua_State* sL = luaL_newstate();
  luaL_openlibs(sL);
  requiref(sL, "_G", luaopen_base, 0);
  requiref(sL, "package", luaopen_package, 1);

  lua_pushlstring(sL, a, len);
  char* b = (char*)luaL_checklstring(sL, -1, &len);
  luaL_loadbuffer(sL, b, len, b);
  
  //l_pprint(L);
  pthread_t thread_id;
  pthread_create(&thread_id, NULL, hh, (void*)sL);
  pthread_detach(thread_id);
  

  return 0;
}
