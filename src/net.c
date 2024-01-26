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

pthread_mutex_t mutex;

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

void http_build(str** _dest, int code, char* code_det, char* type, char* content){
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
    "Content-Type: %s\r\n"
    "\r\n"
    , code, code_det, type);

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
  lua_pushstring(L, "Content-Type");
  lua_gettable(L, res_idx);
  char* content_type = (char*)luaL_checkstring(L, -1);

  lua_pushvalue(L, res_idx);
  lua_pushstring(L, "Code");
  lua_gettable(L, res_idx);
  int code = luaL_checkinteger(L, -1);
  str* resp;
  
  char code_det[50] = {0};
  http_code(code, code_det);
  http_build(&resp, code,  code_det,content_type, content);
  send(client_fd, resp->c, resp->len, 0);
  str_free(resp);
  return 0;
}

void* handle_client(void *_arg){
  //pthread_mutex_lock(&mutex);
  printf("start\n");
  //int client_fd = *((int*)_arg);
  thread_arg_struct* args = (thread_arg_struct*)_arg;
  int client_fd = args->fd;
  //printf("%i\n",args->port);
  lua_State* L = args->L;
  char* buffer;
  char dummy[2] = {0, 0};
  int header_eof;
  //read full request
  size_t bytes_received = recv_full_buffer(client_fd, &buffer, &header_eof);
  printf("buffer made\n");
  //printf("%i\n",recv(client_fd, dummy, 1, 0 ));
  
  //if the buffer, yknow exists
  if(bytes_received > 0){
    str** table;
    int len = 0;
    //checks for a valid header
    if(parse_header(buffer, header_eof, &table, &len) != -1){
      printf("parsed\n");
      //printf("%s\n",buffer);
      
      //str* resp;
      //http_build(&resp, 200, "OK","text/html", "<h1>hello world!</h1>");
      
      //lua_pushvalue(L, 1);
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
        printf("starting\n");
        lua_rawgeti(L, LUA_REGISTRYINDEX, *(int*)v);
        printf("read table\n");

        int func = lua_gettop(L); 
        
        lua_newtable(L);
        lua_newtable(L);
        printf("after tables\n");
        //printf("%s\n",buffer);
        for(int i = 0; i != len * 2; i+=2){
          //printf("'%s' :: '%s'\n",table[i]->c, table[i+1]->c);
          lua_pushstring(L, table[i]->c);
          lua_pushstring(L, table[i+1]->c);
          lua_settable(L, -3);
        }
        
        int req_idx = lua_gettop(L);
        lua_newtable(L);
        //functions
        lua_pushstring(L, "send");
        lua_pushcfunction(L, l_send);
        lua_settable(L, -3);

        //values
        lua_pushstring(L, "client_fd");
        lua_pushinteger(L, client_fd);
        lua_settable(L, -3);

        lua_pushstring(L, "Code");
        lua_pushinteger(L, 200);
        lua_settable(L, -3);

        lua_pushstring(L, "Content-Type");
        lua_pushstring(L, "text/html");
        lua_settable(L, -3);

        lua_pushstring(L, "Content");
        lua_pushstring(L, "");
        lua_settable(L, -3);
        int res_idx = lua_gettop(L);

        lua_pushvalue(L, func);
        lua_pushvalue(L, res_idx);
        lua_pushvalue(L, req_idx);
        printf("calling\n");
        pthread_mutex_lock(&mutex);
        lua_call(L, 2, 0);
        pthread_mutex_unlock(&mutex);
        //*/

      }
      //send(client_fd, resp->c, resp->len, 0);

      //str_free(resp);
      
    }

    for(int i = 0; i != len; i++){
      str_free(table[i]);
    }
    free(table);
  }
  printf("close\n");
  closesocket(client_fd);
  free(args);
  free(buffer);
  printf("end\n");
  //pthread_mutex_unlock(&mutex);
  return NULL;
}

void dcopy_lua(lua_State* dest, lua_State* src, int port){
  lua_newtable(dest); int ot = lua_gettop(dest);
  int tab_idx = luaL_ref(dest, LUA_REGISTRYINDEX);
  lua_rawgeti(dest,LUA_REGISTRYINDEX,tab_idx);
  
  lua_rawgeti(src, LUA_REGISTRYINDEX, ports[port]);
  int t = lua_gettop(src);
  lua_pushnil(src);
  for(;lua_next(src,t) != 0;){
    char* key = (char*)luaL_checkstring(src, -2);
    //lua_pushstring(dest, key);
    //printf("%s\n",key);
    lua_newtable(dest); int bt = lua_gettop(dest);

    int tt = lua_gettop(src);
    lua_pushnil(src);
    for(;lua_next(src,tt) != 0;){
      char* key2 = (char*)luaL_checkstring(src, -2);

      //copy function
      lua_pushstring(dest, key2);
      lua_xmove(src, dest, 1);
      lua_settable(dest, bt);
      //lua_pop(src,1); //not required as xmove pops
    }

    lua_pushstring(dest, key);
    lua_pushvalue(dest, bt);
    lua_settable(dest, ot);

    lua_pushvalue(dest,ot);
     l_pprint(dest);
    lua_pop(src, 1);
  }
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

    //open a state to call shit, should be somewhat thread safe
    lua_State* oL = lua_newthread(L);
    
    //lua_rawseti(L,LUA_REGISTRYINDEX,tab_idx);
    //lua_rawgeti(oL, LUA_REGISTRYINDEX, ports[port]);

    //printf("%p %p\n",lua_topointer(L, -1),lua_topointer(oL, -1));
    //l_pprint(oL);

    //lua_pushvalue(L, aa);
    //l_pprint(L);
    //printf("%i\n",lua_gettop(L));
    //lua_pop(oL, 1);
    //printf("%i %i\n",ports[port], port);
    lua_remove(L, -1);
    thread_arg_struct* args = malloc(sizeof * args);
    args->fd = *client_fd;
    args->port = port;
    args->L = oL;
    //send request to handle_client()
    pthread_t thread_id;
    pthread_create(&thread_id, NULL, handle_client, (void*)args);
    pthread_detach(thread_id);
    //pthread_join(thread_id, NULL);
    
    
    //handle_client((void*)args);
    
  }

}

int l_GET(lua_State* L){
  lua_pushstring(L, "port");
  lua_gettable(L, 1);
  int port = luaL_checkinteger(L, -1);

  char portc[10] = {0};
  sprintf(portc, "%i%s", port, lua_tostring(L, 2));
  lua_pushvalue(L, 3);
  int* idx = malloc(sizeof * idx);
  *idx = luaL_ref(L, LUA_REGISTRYINDEX);

  if(paths == NULL)
    paths = parray_init();
  parray_set(paths, portc, (void*)idx);
  /*
  int tab_idx = ports[port];
  int ot;
  if(tab_idx == 0){
    lua_newtable(L);
    ports[port] = tab_idx = luaL_ref(L, LUA_REGISTRYINDEX);
    lua_rawgeti(L,LUA_REGISTRYINDEX,tab_idx);
    
  } else {
    lua_rawgeti(L,LUA_REGISTRYINDEX,tab_idx);
  }
  int o = lua_gettop(L);

  lua_newtable(L);
  lua_pushstring(L, "fn");
  lua_pushvalue(L, 3);
  lua_settable(L, -3);

  lua_pushvalue(L, o);
  lua_pushvalue(L, 2);
  lua_pushvalue(L, -3);
  lua_settable(L, -3);
  */
  //printf("%i%s",port, lua_tostring(L, 2));
  
  return 1;
}

int l_listen(lua_State* L){
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

  lua_pushstring(L, "port");
  lua_pushvalue(L, 2);
  lua_settable(L, -3);

  lua_pushvalue(L, 1); //the function
  lua_pushvalue(L, -2); //the server table

  lua_pcall(L, 1, 0, 0);

  start_serv(L, port);
  return 0;
}

void* hh(void* args){
  pthread_mutex_lock(&mutex);
  lua_State* L = ((thread_arg_struct*)args)->L;
  lua_pcall(L, 0, 0, 0);
  pthread_mutex_unlock(&mutex);
  return NULL;
}

int l_spawn(lua_State* L){
  if(pthread_mutex_init(&mutex, NULL) != 0)abort();

  lua_State* sL = luaL_newstate();
  luaL_openlibs(sL);
  lua_xmove(L, sL, 1);
  thread_arg_struct* args = malloc(sizeof * args);
  args->L = sL;
    //send request to handle_client()
  pthread_t thread_id;
  pthread_create(&thread_id, NULL, hh, (void*)args);
  pthread_detach(thread_id);
  

  return 0;
}
