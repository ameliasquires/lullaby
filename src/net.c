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
#include <sys/stat.h>

#include "net.h"
#include "lua.h"

#include "io.h"
#include "table.h"
#include "types/str.h"
#include "types/parray.h"

#define max_con 200
#define BUFFER_SIZE 2048

static int ports[65535] = { 0 };
static parray_t* paths = NULL;

struct lchar {
  char* c;
  int len;
  char req[20];
};

struct sarray_t {
  struct lchar** cs;
  int len;
};

pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t lua_mutex = PTHREAD_MUTEX_INITIALIZER;

int64_t recv_full_buffer(int client_fd, char** _buffer, int* header_eof){
  char* buffer = malloc(BUFFER_SIZE * sizeof * buffer);
  memset(buffer, 0, BUFFER_SIZE);
  char* header;
  size_t len = 0;
  *header_eof = -1;
  int n;
  int content_len = -1;
  //printf("before\n");
  //fcntl(client_fd, F_SETFL, fcntl(client_fd, F_GETFL) | O_NONBLOCK)
  for(;;){
    n = recv(client_fd, buffer + len, BUFFER_SIZE, 0);
    if(n < 0){
      *_buffer = buffer;
      if(*header_eof == -1) return -2; //dont even try w/ request, no header to read
      return -1; //well the header is fine atleast

    };
    if(*header_eof == -1 && (header = strstr(buffer, "\r\n\r\n")) != NULL){
      //printf("head\n");
      *header_eof = header - buffer;
      char* cont_len_raw = strstr(buffer, "Content-Length: ");
      if(cont_len_raw == NULL) {
        len += n;
        *_buffer = buffer;
        return len + BUFFER_SIZE;
      }
      str* cont_len_str = str_init("");
      if(cont_len_raw == NULL) abort();
      //i is length of 'Content-Length: '
      for(int i = 16; cont_len_raw[i] != '\r'; i++) str_pushl(cont_len_str, cont_len_raw + i, 1);
      content_len = strtol(cont_len_str->c, NULL, 10);
      str_free(cont_len_str);
      //printf("nut\n");*/

    }
    //check if the recv read the whole buffer length, sometimes it wont so i peek to see if there is more
    //if(n != BUFFER_SIZE && recv(client_fd, NULL, 1, MSG_PEEK) != 1) break;
    //printf("%i %i\n", n, content_len);
    len += n;
    //if(n != 0){
      //printf("buffer %i\n", n);
      buffer = realloc(buffer, len + BUFFER_SIZE);
      //printf("realloc\n");
      memset(buffer + len, 0, n);

      
      //printf("%i\n", len - *header_eof - 4);
    //}
    if(content_len != -1 && len - *header_eof - 4 >= content_len) break;
  }
  //printf("%i\n",len - *header_eof - 4);
  *_buffer = buffer;
  return len + BUFFER_SIZE;
}

int parse_header(char* buffer, int header_eof, parray_t** _table){
  if(header_eof == -1) return -1;
  char add[] = {0,0};
  int lines = 3;
  for(int i = 0; i != header_eof; i++) lines += buffer[i] == '\n';
  parray_t* table = parray_init();
  str* current = str_init("");

  int oi = 0;
  int item = 0;
  for(; oi != header_eof; oi++){
    if(buffer[oi] == ' ' || buffer[oi] == '\n'){
      if(buffer[oi] == '\n') current->c[current->len - 1] = 0;
      parray_set(table, item == 0 ? "Request" :
                item == 1 ? "Path" : "Version", (void*)str_init(current->c));
      str_clear(current);
      item++;
      if(buffer[oi] == '\n') break;
    } else str_pushl(current, buffer + oi, 1);
  }

  int key = 1;
  str* sw = NULL;
  for(int i = oi + 1; i != header_eof; i++){
    if(buffer[i] == ' ' && strcmp(current->c, "") == 0) continue;
    if(key && buffer[i] == ':' || !key && buffer[i] == '\n'){
      if(key){
        sw = current;
        current = str_init("");
        key = 0;
      } else {
        if(buffer[oi] == '\n') current->c[current->len - 1] = 0;
        parray_set(table, sw->c, (void*)str_init(current->c));
        str_clear(current);
        sw = NULL;
        key = 1;
      }
      continue;
    } else str_pushl(current, buffer + i, 1);
  }
  parray_set(table, sw->c, (void*)str_init(current->c));
  parray_set(table, "Body", (void*)str_init(buffer + header_eof + 4));
  str_free(current);
  *_table = table;
  return 0;
}s

void http_build(str** _dest, int code, char* code_det, char* header_vs, char* content, size_t len){
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
  //str_push(*_dest, content);
  for(size_t i = 0; i != len; i++){
    str_pushl(*_dest, content + i, 1);
  }
  free(dest);
}

typedef struct {
  int fd;
  int port;
  lua_State* L;
  struct sockaddr_in cli;
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

void i_write_header(lua_State* L, int header_top, str** _resp, char* content, size_t len){
  str* resp;
  lua_pushvalue(L, header_top);

  str* header_vs = str_init("");
  lua_pushnil(L);
    
  for(;lua_next(L, header_top) != 0;){
      char* key = (char*)luaL_checklstring(L, -2, NULL);
      if(strcmp(key, "Code") != 0){
        str_push(header_vs, key);
        str_push(header_vs, ": ");
        str_push(header_vs, (char*)luaL_checklstring(L, -1, NULL));
        str_push(header_vs, "\r\n");
      }
      lua_pop(L, 1);
  }

  lua_pushvalue(L, header_top);
  lua_pushstring(L, "Code");
  lua_gettable(L, header_top);
  int code = luaL_checkinteger(L, -1);
    
  char code_det[50] = {0};
  http_code(code, code_det);
  http_build(&resp, code,  code_det, header_vs->c, content, len);

  str_free(header_vs);

  *_resp = resp;
}

void client_fd_errors(int client_fd){
  if(client_fd>=0) return;

  switch(client_fd){
    case -1:
      p_fatal("client fd already closed\n");
    case -2:
      p_fatal("request was partial\n");
    default:
      p_fatal("unknown negative client_fd value");
  }
}
int l_write(lua_State* L){
  int res_idx = 1;

  lua_pushvalue(L, 1);
  lua_pushstring(L, "_request");
  lua_gettable(L, -2);

  int head = strcmp(luaL_checkstring(L, -1), "HEAD") == 0;
  
  lua_pushvalue(L, res_idx);
  lua_pushstring(L, "client_fd");
  lua_gettable(L, res_idx);
  int client_fd = luaL_checkinteger(L, -1);

  client_fd_errors(client_fd);

  size_t len;
  char* content = (char*)luaL_checklstring(L, 2, &len);
  
  lua_pushvalue(L, res_idx);
  lua_pushstring(L, "header");
  lua_gettable(L, -2);
  int header_top = lua_gettop(L);

  lua_pushstring(L, "_sent");
  lua_gettable(L, -2);
  str* resp;
  if(lua_isnil(L, -1)){
    if(head) i_write_header(L, header_top, &resp, "", 0);
    else i_write_header(L, header_top, &resp, content, len);

    lua_pushvalue(L, header_top);
    lua_pushstring(L, "_sent");
    lua_pushinteger(L, 1);
    lua_settable(L, -3);
  } else {
    if(head) return 0;
    resp = str_init(content);
  }

  send(client_fd, resp->c, resp->len, 0);

  str_free(resp);
  return 0;
}

int l_send(lua_State* L){
  int res_idx = 1;
  
  lua_pushvalue(L, res_idx);
  lua_pushstring(L, "client_fd");
  lua_gettable(L, res_idx);
  int client_fd = luaL_checkinteger(L, -1);
  
  client_fd_errors(client_fd);

  size_t len;
  char* content = (char*)luaL_checklstring(L, 2, &len);

  lua_pushvalue(L, res_idx);
  lua_pushstring(L, "header");
  lua_gettable(L, -2);
  int header = lua_gettop(L);

  str* resp;
  lua_pushvalue(L, 1);
  lua_pushstring(L, "_request");
  lua_gettable(L, -2);

  if(strcmp(luaL_checkstring(L, -1), "HEAD") == 0){
    i_write_header(L, header, &resp, "", 0);
  } else 
    i_write_header(L, header, &resp, content, len);

  send(client_fd, resp->c, resp->len, 0);
  
  lua_pushstring(L, "client_fd");
  lua_pushinteger(L, -1);
  lua_settable(L, res_idx);
  closesocket(client_fd);

  str_free(resp);
  return 0;
}

int l_close(lua_State* L){
  int res_idx = 1;
  
  lua_pushvalue(L, res_idx);
  lua_pushstring(L, "client_fd");
  lua_gettable(L, res_idx);
  int client_fd = luaL_checkinteger(L, -1);
  client_fd_errors(client_fd);

  lua_pushstring(L, "client_fd");
  lua_pushinteger(L, -1);
  lua_settable(L, res_idx);
  closesocket(client_fd);

  return 0;
}

int l_serve(lua_State* L){
  int res_idx = 1;
  
  lua_pushvalue(L, res_idx);
  lua_pushstring(L, "client_fd");
  lua_gettable(L, res_idx);
  int client_fd = luaL_checkinteger(L, -1);
  client_fd_errors(client_fd);

  char* path = (char*)luaL_checkstring(L, 2);

  //continue here

  return 0;
}

int file_parse(lua_State* L, char* buffer, str* content_type, char** end_buffer){
  str* boundary = str_init(""); //usually add + 2 to the length when using
  int state = 0;
  for(int i = 0; content_type->c[i] != '\0'; i++){
    if(state == 2) str_pushl(boundary, content_type->c + i, 1);
    if(content_type->c[i] == ';') state = 1;
    if(content_type->c[i] == '=' && state == 1) state = 2;
  }
  //printf("%s\n",boundary->c);
  
  lua_newtable(L);
  int base_T = lua_gettop(L);

  for(int i = 1;;i++){
    lua_newtable(L);
    int file_T = lua_gettop(L);

    str* file = str_init("");
    char* ind = strstr(buffer, boundary->c);
    if(ind == NULL) break;
    char* ending_ind = strstr(ind + boundary->len + 2, boundary->c);
    if(ending_ind == NULL) break;

    char* header_eof = strstr(ind + boundary->len + 2, "\r\n\r\n");
    str* current = str_init("");
    int key = -1;
    for(char* s = ind + boundary->len + 2; s != header_eof; s++){

      if(*s == ':'){ //todo: collapse Content-Disposition
        lua_pushlstring(L, current->c, current->len);
        key = lua_gettop(L);
        str_clear(current);
      } else if(*s == '\n') {
        if(key != -1){
          luaI_tsets(L, file_T , luaL_checkstring(L, key), current->c);
          key = -1;
          str_clear(current);
        }
      } else if(*s != '\r' && !(*s == ' ' && strcmp(current->c, "") == 0)){
        str_pushl(current, s, 1);
      }
    }
    str_free(current);
    luaI_tsets(L, file_T , luaL_checkstring(L, key), current->c);

    str_pushl(file, header_eof + 2, ending_ind - header_eof - 4);
    //printf("'%s'\n",file->c);
    lua_pushstring(L, "content");
    lua_pushlstring(L, file->c, file->len);
    lua_settable(L, file_T);

    buffer = ending_ind;
    //printf("'%s'\n", buffer);
    str_free(file);

    lua_pushinteger(L, i);
    lua_pushvalue(L, file_T);
    lua_settable(L, base_T);
    //l_pprint(L);
  }
  str_free(boundary);
  
  *end_buffer = buffer + boundary->len + 2;
  lua_pushvalue(L, base_T);
  return base_T;
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

  pthread_mutex_lock(&mutex);
  int old_top = lua_gettop(args->L);
  lua_getglobal(args->L, "_G");

  i_dcopy(args->L, L, NULL);
  
  lua_settop(args->L, old_top);
  //l_pprint(L);
  lua_setglobal(L, "_G");
  pthread_mutex_unlock(&mutex);

  //read full request
  int64_t bytes_received = recv_full_buffer(client_fd, &buffer, &header_eof);
  //ignore if header is just fucked
  if(bytes_received >= -1){
    //str** table;
    parray_t* table;
    //checks for a valid header
    if(parse_header(buffer, header_eof, &table) != -1){
      
      //int k = stable_key(table, "Path", len);
      //int R = stable_key(table, "Request", len);
      //int T = stable_key(table, "Content-Type", len);

      str* sk = (str*)parray_get(table, "Path");
      str* sR = (str*)parray_get(table, "Request");
      str* sT = (str*)parray_get(table, "Content-Type");
      //printf("%s\n", sk->c);
      char portc[10] = {0};
      sprintf(portc, "%i", args->port);

      str* aa = str_init(portc);
      //if(table[k]->c[table[k]->len - 1] != '/') str_push(table[k], "/");
      str_push(aa, sk->c);
      //if(aa->c[aa->len - 1] != '/') str_push(aa, "/");

      void* v = parray_find(paths, aa->c);

      str_free(aa);
      if(v == NULL){
        //this should not be here
        str* resp;
        http_build(&resp, 404, "Not Found","text/html", "<h1>404</h1>", 12);
        send(client_fd, resp->c, resp->len, 0);
        str_free(resp);
      } else {
        lua_newtable(L);
        int req_idx = lua_gettop(L);
        lua_newtable(L);
        int res_idx = lua_gettop(L);

        char* new_cont;
        int pf = file_parse(L, buffer + header_eof, sT, &new_cont);
        
        luaI_tsetv(L, req_idx, "files", pf);

        for(int i = 0; i != table->len; i+=1){
          //printf("'%s' :: '%s'\n",table[i]->c, table[i+1]->c);
          luaI_tsets(L, req_idx, table->P[i].key->c, ((str*)table->P[i].value)->c);
        }

        luaI_tsets(L, req_idx, "ip", inet_ntoa(args->cli.sin_addr));

        if(bytes_received == -1){
          client_fd = -2;
        }

        luaI_tsetb(L, req_idx, "partial", bytes_received == -1);
        luaI_tseti(L, req_idx, "_bytes", bytes_received);
        //printf("%s\n",table[T]->c);
        
        //functions
        luaI_tsetcf(L, res_idx, "send", l_send);
        //luaI_tsetcf(L, res_idx, "serve", l_serve);
        luaI_tsetcf(L, res_idx, "write", l_write);
        luaI_tsetcf(L, res_idx, "close", l_close);

        //values
        luaI_tseti(L, res_idx, "client_fd", client_fd);
        luaI_tsets(L, res_idx, "_request", sR->c);

        //header table
        lua_newtable(L);
        int header_idx = lua_gettop(L);
        luaI_tseti(L, header_idx, "Code", 200);
        luaI_tsets(L, header_idx, "Content-Type", "text/html");
        
        luaI_tsetv(L, res_idx, "header", header_idx);

        //the function(s)
        //get all function that kinda match
        parray_t* owo = (parray_t*)v;
        uint64_t passes = 0;
        for(int i = 0; i != owo->len; i++){
          //though these are arrays of arrays we have to iterate *again*
          struct sarray_t* awa = (struct sarray_t*)owo->P[i].value;

          for(int z = 0; z != awa->len; z++){
            char* path;
            struct lchar* wowa = awa->cs[z];
            if(strcmp(wowa->req, "all") == 0 || strcmp(wowa->req, sR->c) == 0 ||
                (strcmp(sR->c, "HEAD") && strcmp(wowa->req, "GET"))){
                  luaI_tseti(L, res_idx, "passes", passes);
                  passes++;

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
        parray_lclear(owo); //dont free the rest

        lua_pushstring(L, "client_fd");
        lua_gettable(L, res_idx);
        client_fd = luaL_checkinteger(L, -1);

      }
      
    }

    parray_clear(table, 1);
  }

  if(client_fd > 0) closesocket(client_fd);
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
  if((server_fd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
    p_fatal("error opening socket\n");

  server_addr.sin_family = AF_INET;
  server_addr.sin_addr.s_addr = INADDR_ANY;
  server_addr.sin_port = htons(port);

  //bind to port
  if(bind(server_fd, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0)
    p_fatal("failed to bind to port\n");

  if(listen(server_fd, max_con) < 0)
    p_fatal("failed to listen\n");
  /*
  lua_rawgeti(L, LUA_REGISTRYINDEX, ports[port]);
  lua_pushstring(L, "/");
  lua_gettable(L, -2);
  lua_pushstring(L, "fn");
  lua_gettable(L, -2);
  int aa = lua_gettop(L);*/
  if (pthread_mutex_init(&mutex, NULL) != 0)
    p_fatal("mutex init failed\n");

  if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, (const char*)&(int){1}, sizeof(int)) < 0)
    p_fatal("SO_REUSEADDR refused\n");

  for(;;){
    struct sockaddr_in client_addr;
    socklen_t client_addr_len = sizeof(client_addr);
    int* client_fd = malloc(sizeof(int));

    if((*client_fd = accept(server_fd, (struct sockaddr*)&client_addr, &client_addr_len)) < 0)
      p_fatal("failed to accept\n");
    //printf("%s\n",inet_ntoa(client_addr.sin_addr));
    //printf("%i\n",threads);
    //open a state to call shit, should be somewhat thread safe

    thread_arg_struct* args = malloc(sizeof * args);
    args->fd = *client_fd;
    args->port = port;
    args->cli = client_addr;
    args->L = L;

    pthread_mutex_lock(&mutex);
    threads++;
    pthread_mutex_unlock(&mutex);

    //send request to handle_client()
    pthread_t thread_id;
    pthread_create(&thread_id, NULL, handle_client, (void*)args);
    pthread_detach(thread_id);
    
    free(client_fd);
  }

}

int l_req_com(lua_State* L, char* req){
  lua_pushstring(L, "port");
  lua_gettable(L, 1);
  int port = luaL_checkinteger(L, -1);

  char portc[10] = {0};
  sprintf(portc, "%i", port);//, lua_tostring(L, 2));
  str* portss = str_init(portc);
  str_push(portss, (char*)lua_tostring(L, 2));
  //if(portss->c[portss->len - 1] != '/') str_push(portss, "/");

  struct lchar* awa;
  lua_getglobal(L, "string");
  lua_pushstring(L, "dump");
  lua_gettable(L, -2);
  lua_pushvalue(L, 3);
  lua_call(L, 1, 1);

  size_t len;
  char* a = (char*)luaL_checklstring(L, -1, &len);
  awa = malloc(len + 1);
  awa->c = a;
  awa->len = len;
  strcpy(awa->req, req);
  //printf("%s\n",awa->req);

  if(paths == NULL)
    paths = parray_init();

  //please free this
  void* v_old_paths = parray_get(paths, portss->c);
  struct sarray_t* old_paths;
  if(v_old_paths == NULL){
    old_paths = malloc(sizeof * old_paths);
    old_paths->len = 0;
    old_paths->cs = malloc(sizeof * old_paths->cs);
  } else old_paths = (struct sarray_t*)v_old_paths;

  old_paths->len++;
  old_paths->cs = realloc(old_paths->cs, sizeof * old_paths->cs * old_paths->len);
  old_paths->cs[old_paths->len - 1] = awa;

  parray_set(paths, portss->c, (void*)old_paths);
  str_free(portss);
  return 1;
}

#define gen_reqs(T)\
int l_##T##q (lua_State* L){\
  l_req_com(L, #T);\
  return 1;\
}
gen_reqs(GET);
gen_reqs(HEAD);
gen_reqs(POST);
gen_reqs(PUT);
gen_reqs(DELETE);
gen_reqs(CONNECT);
gen_reqs(OPTIONS);
gen_reqs(TRACE);
gen_reqs(PATCH);
gen_reqs(all); //non standard lol, like expressjs use keyword :3
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
  int mt = lua_gettop(L);
  luaI_tsetcf(L, mt, "GET", l_GETq);
  luaI_tsetcf(L, mt, "HEAD", l_HEADq);
  luaI_tsetcf(L, mt, "POST", l_POSTq);
  luaI_tsetcf(L, mt, "PUT", l_PUTq);
  luaI_tsetcf(L, mt, "DELETE", l_DELETEq);
  luaI_tsetcf(L, mt, "CONNECT", l_CONNECTq);
  luaI_tsetcf(L, mt, "OPTIONS", l_OPTIONSq);
  luaI_tsetcf(L, mt, "TRACE", l_TRACEq);
  luaI_tsetcf(L, mt, "PATCH", l_PATCHq);
  luaI_tsetcf(L, mt, "all", l_allq);
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
