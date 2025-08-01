#include "net/common.h"
#include "net/util.h"
#include "net/lua.h"
#include "net/luai.h"

#include <fcntl.h>

#include <netdb.h>
#include <netinet/tcp.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <netdb.h>

#include <openssl/ssl.h>
#include <openssl/err.h>

#include <assert.h>
#include <signal.h>

#define pab(M) {printf(M);abort();}

_Atomic int has_ssl_init = 0;

void ssl_init(){
  if(has_ssl_init == 0){
    has_ssl_init = 1;
    SSL_library_init();
    SSL_load_error_strings();
  }
}

SSL* ssl_connect(SSL_CTX* ctx, int sockfd, const char* hostname){
  SSL* ssl = SSL_new(ctx);
  if(hostname != NULL)
    SSL_set_tlsext_host_name(ssl, hostname);

  SSL_set_fd(ssl, sockfd);

  if(SSL_connect(ssl) < 0){
    SSL_free(ssl);

    return NULL;
  }

  return ssl;
}

int get_host(char* hostname, char* port){
  int sockfd;
  struct addrinfo  hints;

  memset(&hints, 0, sizeof(hints));
  hints.ai_family = AF_INET;
  hints.ai_socktype = SOCK_STREAM;
  hints.ai_flags = 0;
  hints.ai_protocol = 0;

  struct addrinfo *result, *awa;
  if(getaddrinfo(hostname, port, &hints, &result) != 0)
    return -1;

  for(awa = result; awa != NULL; awa = awa->ai_next){
    sockfd = socket(awa->ai_family, awa->ai_socktype, awa->ai_protocol);

    if(sockfd == -1) continue;

    if(connect(sockfd, awa->ai_addr, awa->ai_addrlen) != -1)
      break;

    close(sockfd);
    sockfd = -1;
  }

  freeaddrinfo(result);

  return sockfd;
}

struct chunked_encoding_state {
  int reading_length;
  int chunk_length;
  str* buffer;
  str* content;
};

//remove this eventually
int chunked_encoding_round(char* input, int length, struct chunked_encoding_state* state){
  //printf("'%s'\n", input);
  for(int i = 0; i < length; i++){
    //printf("%i/%i\n", i, length);
    if(state->reading_length){
    str_pushl(state->buffer, input + i, 1);

      if(state->buffer->len >= 2 && memmem(state->buffer->c + state->buffer->len - 2, 2, "\r\n", 2)){

        str_popb(state->buffer, 2);
        state->chunk_length = strtoll(state->buffer->c, NULL, 16);
        str_clear(state->buffer);
        state->reading_length = 0;
      }
    } else {
      int len = lesser(state->chunk_length - state->buffer->len, length - i);
      str_pushl(state->buffer, input + i, len);
      i += len;

      if(state->buffer->len >= state->chunk_length){
        state->reading_length = 1;
        str_pushl(state->content, state->buffer->c, state->buffer->len);
        str_clear(state->buffer);
      }
    }
  }

  //printf("buffer '%s'\n", state->buffer->c);

  return 0;
}

struct url {
  str* proto,
     * domain,
     * port,
     * path;
};

struct url parse_url(char* url, int len){
  struct url awa = {0};
  str* buffer = str_init("");
  int read = 0;

  for(int i = 0; i != len; i++){
    if(url[i] == ':'){
      if(awa.proto == NULL && i + 2 < len && url[i + 1] == '/' && url[i + 2] == '/'){
        awa.proto = buffer;
        buffer = str_init("");
        i += 2;
      } else if (awa.port == NULL){
        awa.domain = buffer;
        buffer = str_init("");
        read = 1;
      }
    } else if(read != 2 && url[i] == '/'){
      if(read == 1){
        awa.port = buffer;
      } else {
        awa.domain = buffer;
      }
      buffer = str_init("");
      read = 2;
      i--;
    } else {
      str_pushl(buffer, url + i, 1);
    }
  }

  if(read == 0) awa.domain = buffer;
  else if(read == 1) awa.port = buffer;
  else awa.path = buffer;

  return awa;
}

void free_url(struct url u){
  if(u.proto != NULL) str_free(u.proto);
  if(u.domain != NULL) str_free(u.domain);
  if(u.path != NULL) str_free(u.path);
  if(u.port != NULL) str_free(u.port);
}

#define BUFFER_LEN 4096
struct wss_data {
  SSL* ssl;
  SSL_CTX* ctx;
  int sock;
  str* buffer; 
};

struct ws_frame_info {
  int fin;
  int rsv1;
  int rsv2;
  int rsv3;
  int opcode;
  int mask;
  int payload;
};

int i_ws_read(lua_State* L){
  lua_pushstring(L, "_");
  lua_gettable(L, 1);
  struct wss_data* data = lua_touserdata(L, -1);
  char buffer[BUFFER_LEN] = {0};
  int total_len = 0;
  int len;

  for(; (len = SSL_read(data->ssl, buffer + total_len, 2 - total_len)) > 0;){ 
    total_len += len;
    if(total_len >= 2) break;
  }

  if(len < 0) luaI_error(L, len, "SSL_read error");
 
  uint64_t payload = 0;
  struct ws_frame_info frame_info = {.fin = (buffer[0] >> 7) & 1, .rsv1 = (buffer[0] >> 6) & 1,
    .rsv2 = (buffer[0] >> 5) & 1, .rsv3 = (buffer[0] >> 4) & 1, .opcode = buffer[0] & 0b1111,
    .mask = (buffer[1] >> 7) & 1, .payload = buffer[1] & 0b1111111};
  //printf("fin: %i\npayload: %i\n", frame_info.fin, frame_info.payload);
  memset(buffer, 0, total_len);
  total_len = 0;

  if(frame_info.payload <= 125) payload = frame_info.payload;
  else if(frame_info.payload == 126) {
    for(; (len = SSL_read(data->ssl, buffer + total_len, 2 - total_len)) > 0;){ 
      total_len += len;
      if(total_len >= 2) break;
    }

    if(len < 0) luaI_error(L, len, "SSL_read error");

    payload = (buffer[0] & 0xff) << 8 | (buffer[1] & 0xff);
  } else {
    for(; (len = SSL_read(data->ssl, buffer + total_len, 8 - total_len)) > 0;){
        total_len += len;
        if(total_len >= 8) break;
    }

    if(len < 0) luaI_error(L, -1, "SSL_read error");


    payload = ((uint64_t)buffer[0] & 0xff) << 56 | ((uint64_t)buffer[1] & 0xff) << 48 | ((uint64_t)buffer[2] & 0xff) << 40 |
      ((uint64_t)buffer[3] & 0xff) << 32 | (buffer[4] & 0xff) << 24 | (buffer[5] & 0xff) << 16 | (buffer[6] & 0xff) << 8 | (buffer[7] & 0xff);
  }
  //printf("final payload: %lu\n", payload);

  str* message = str_init("");
  memset(buffer, 0, BUFFER_LEN);
  for(; message->len != payload && (len = SSL_read(data->ssl, buffer, lesser(payload - message->len, BUFFER_LEN))) > 0;){ 
    str_pushl(message, buffer, len);
    memset(buffer, 0, len);
  }

  if(len < 0) {
    str_free(message);
    luaI_error(L, len, "SSL_read error");
  }
 
  lua_newtable(L);
  int idx = lua_gettop(L);
  luaI_tsetsl(L, idx, "content", message->c, message->len);
  luaI_tseti(L, idx, "opcode", frame_info.opcode);

  str_free(message);
  return 1;
}

int i_ws_write(lua_State* L){
  lua_pushstring(L, "_");
  lua_gettable(L, 1);
  struct wss_data* data = lua_touserdata(L, -1);

  uint64_t clen;
  const char* content = luaL_tolstring(L, 2, &clen);
  str* send_data = str_init("");

  str_pushl(send_data, (const char[]){0b10000001}, 1);
  if(clen <= 125) str_pushl(send_data, (const char[]){(0x1 << 7) | clen}, 1);
  else if(clen <= 65535)
    str_pushl(send_data, (const char[]){(0x1 << 7) | 126, (clen >> 8) & 0xff, clen & 0xff}, 3);
  else
    str_pushl(send_data, (const char[]){(0x1 << 7) | 127, (clen >> 56) & 0xff, (clen >> 48) & 0xff,
        (clen >> 40) & 0xff, (clen >> 32) & 0xff, (clen >> 24) & 0xff, (clen >> 16) & 0xff,
        (clen >> 8) & 0xff, clen & 0xff}, 9);
  str_pushl(send_data, (const char[]){0, 0, 0, 0}, 4);
  str_pushl(send_data, content, clen);

  int s = SSL_write(data->ssl, send_data->c, send_data->len);
  str_free(send_data);

  if(s <= 0) luaI_error(L, s, "SSL_write error");
  lua_pushinteger(L, 1);

  return 1;
}

int i_ws_close(lua_State* L){
  lua_pushstring(L, "_");
  lua_gettable(L, 1);
  struct wss_data* data = lua_touserdata(L, -1);

  if(data != NULL && data->buffer != NULL){
    str_free(data->buffer);

    SSL_set_shutdown(data->ssl, SSL_RECEIVED_SHUTDOWN | SSL_SENT_SHUTDOWN);
    SSL_shutdown(data->ssl);
    SSL_free(data->ssl);
    SSL_CTX_free(data->ctx);

    if(data->sock != -1) close(data->sock);

    free(data);
  }

  lua_pushstring(L, "_");
  lua_pushnil(L);
  lua_settable(L, 1);
  return 0;
}



int l_wss(lua_State* L){  
  uint64_t len = 0;
  char* request_url = (char*)lua_tolstring(L, 1, &len);
  struct url awa = parse_url(request_url, len);
  if(awa.proto != NULL && strcmp(awa.proto->c, "ws") == 0){
    //send to l_ws, todo
    abort();
  }

  char* port = awa.port == NULL ? "443" : awa.port->c;
  char* path = awa.path == NULL ? "/" : awa.path->c;
  int sock = get_host(awa.domain->c, port);
  signal(SIGPIPE, SIG_IGN);

  ssl_init();
  SSL_CTX* ctx = SSL_CTX_new(SSLv23_client_method());
  SSL* ssl = ssl_connect(ctx, sock, awa.domain->c);

  char* request = calloc(512, sizeof * request);
  sprintf(request, "GET %s HTTP/1.1\r\nHost: %s\r\nConnection: Upgrade\r\nUpgrade: websocket\r\n"\
      "Sec-Websocket-Key: aWxvdmVsb3ZlbG92ZXlvdQ==\r\nSec-Websocket-Version: 13\r\n\r\n", path, awa.domain->c);

  int s = SSL_write(ssl, request, strlen(request));
  free_url(awa);
  free(request);

  if(s <= 0){
    luaI_error(L, s, "SSL_write error");
  }

  char buffer[BUFFER_LEN];
  len = 0;
  str* a = str_init("");
  char* header_eof = NULL;
  for(; (len = SSL_read(ssl, buffer, BUFFER_LEN)) > 0;){
    str_pushl(a, buffer, len);
    if((header_eof = memmem(a->c, a->len, "\r\n\r\n", 4)) != NULL){
      break;
    }
    memset(buffer, 0, BUFFER_LEN);
  }

  if(len < 0) luaI_error(L, len, "SSL_read error");

  if(header_eof == NULL){
    luaI_error(L, -1, "error with header formating");
  }

  struct wss_data *data = malloc(sizeof * data);
  data->ssl = ssl;
  data->ctx = ctx;
  data->sock = sock;
  data->buffer = str_init("");//str_initl(header_eof, extra_len);
  str_free(a);

  lua_newtable(L);
  int idx = lua_gettop(L);

  luaI_tsetlud(L, idx, "_", data);
  luaI_tsetcf(L, idx, "read", i_ws_read);
  luaI_tsetcf(L, idx, "write", i_ws_write);
  luaI_tsetcf(L, idx, "close", i_ws_close);
  
  lua_newtable(L);
  int meta_idx = lua_gettop(L);

  luaI_tsetcf(L, meta_idx, "__gc", i_ws_close);

  lua_pushvalue(L, meta_idx);
  lua_setmetatable(L, idx);

  lua_pushvalue(L, idx);
 
  //verify stuff here
  //parray_t* owo = NULL;
  //parse_header(a->c, header_eof - a->c, &owo);
  
  return 1;
}

struct _srequest_state {
  SSL* ssl;
  SSL_CTX* ctx;
  int sock;
  str* buffer; //anything pre-existing
  struct chunked_encoding_state* state;
};

int _srequest_free(void** _state){
  struct _srequest_state* state = *((struct _srequest_state**)_state);
  SSL_set_shutdown(state->ssl, SSL_RECEIVED_SHUTDOWN | SSL_SENT_SHUTDOWN);
  SSL_shutdown(state->ssl);
  SSL_free(state->ssl);
  SSL_CTX_free(state->ctx);
  close(state->sock);

  if(state->state != NULL){
    str_free(state->state->buffer);
    str_free(state->state->content);
    free(state->state);
  }

  if(state->buffer != NULL){
    str_free(state->buffer);
  }
  free(state);
  return 0;
}

int _srequest_read(uint64_t reqlen, str** _output, void** _state){
  struct _srequest_state* state = *((struct _srequest_state**)_state);
  str* output = *_output;
  //states using chunked encoding should skip this
  if(state->buffer != NULL){
    str_pushl(output, state->buffer->c, state->buffer->len);
    str_free(state->buffer);
    state->buffer = NULL;
  }

  char buffer[BUFFER_LEN];
  memset(buffer, 0, BUFFER_LEN);
  uint64_t len;
  for(; (len = SSL_read(state->ssl, buffer, BUFFER_LEN)) > 0;){
    if(state->state != NULL){
      chunked_encoding_round(buffer, len, state->state);
    } else {
      str_pushl(output, buffer, len);
    }
    memset(buffer, 0, BUFFER_LEN);
  }

  if(state->state != NULL){
    str_pushl(output, state->state->content->c, state->state->content->len);
    str_clear(state->state->content);
  }

  *_output = output;
  return 1; 
}

int _srequest_chunked_encoding(char* input, int length, struct chunked_encoding_state* state){
  //printf("'%s'\n", input);
  for(int i = 0; i < length; i++){
    //printf("%i/%i\n", i, length);
    if(state->reading_length){
    str_pushl(state->buffer, input + i, 1);

      if(state->buffer->len >= 2 && memmem(state->buffer->c + state->buffer->len - 2, 2, "\r\n", 2)){

        str_popb(state->buffer, 2);
        state->chunk_length = strtoll(state->buffer->c, NULL, 16);
        str_clear(state->buffer);
        state->reading_length = 0;
      }
    } else {
      int len = lesser(state->chunk_length - state->buffer->len, length - i);
      str_pushl(state->buffer, input + i, len);
      i += len;

      if(state->buffer->len >= state->chunk_length){
        state->reading_length = 1;
        str_pushl(state->content, state->buffer->c, state->buffer->len);
        str_clear(state->buffer);
      }
    }
  }

  //printf("buffer '%s'\n", state->buffer->c);

  return 0;
}

int l_srequest(lua_State* L){
  int params = lua_gettop(L);

  int check = 1;
  uint64_t ilen = 0;
  char* request_url = (char*)lua_tolstring(L, 1, &ilen);
  struct url awa = parse_url(request_url, ilen);
  if(awa.proto != NULL && strcmp(awa.proto->c, "http") == 0){
    //send to l_request, todo
    abort();
  }
  const char* host = awa.domain == NULL ? request_url : awa.domain->c;
  const char* port = awa.port == NULL ? "443" : awa.port->c; 
  char* path = awa.path == NULL ? "/" : awa.path->c; 

  int sock = get_host((char*)host, (char*)port);
  if(sock == -1){
    //p_fatal("could not resolve address");
    //abort();
    luaI_error(L, -1, "error resolving address");
  }

  ssl_init();
  SSL_CTX* ctx = SSL_CTX_new(SSLv23_client_method());
  SSL* ssl = ssl_connect(ctx, sock, host);
  if(ssl == NULL) luaI_error(L, -1, "ssl_connect error");
 
  char* cont = "";
  size_t cont_len = 0;
  if(params >= check + 1){
    check++;
    switch(lua_type(L, check)){
      case LUA_TNUMBER:
      case LUA_TSTRING:
        cont = (char*)luaL_tolstring(L, check, &cont_len);
        break;
      default:
        p_fatal("cant send type");
        break;
    }
  }

  str* header = str_init("");
  if(params >= check + 1){
    check++;
    lua_pushnil(L);

    for(;lua_next(L, check) != 0;){
      str_push(header, "\r\n");
      str_push(header, lua_tostring(L, -2));
      str_push(header, ": ");
      str_push(header, lua_tostring(L, -1));
      lua_pop(L, 1);
    }
  }
  
  char* action = "GET";
  if(params >= check + 1){
    check++;
    action = (char*)lua_tostring(L, check);
  }

  //char* req = "GET / HTTP/1.1\nHost: amyy.cc\nConnection: Close\n\n";

  char* request = calloc(cont_len + header->len + 512, sizeof * request);
  sprintf(request, "%s %s HTTP/1.1\r\nHost: %s\r\nConnection: Close%s\r\n\r\n%s", action, path, host, header->c, cont); 
  //printf("%s\n", request);
  str_free(header);

  int s = SSL_write(ssl, request, strlen(request));
  free(request);

  if(s <= 0) luaI_error(L, s, "SSL_write error");

  str* a = str_init("");
  char buffer[BUFFER_LEN];
  int len = 0;
  int extra_len = 0;
  char* header_eof = NULL;

  for(; (len = SSL_read(ssl, buffer, BUFFER_LEN)) > 0;){
    int blen = a->len;
    str_pushl(a, buffer, len);
    int offset = blen >= 4 ? 4 : blen;
    if((header_eof = memmem(a->c + blen - offset, len + offset, "\r\n\r\n", 4)) != NULL){
      extra_len = a->len - (header_eof - a->c);
      break;
    }
    memset(buffer, 0, BUFFER_LEN);
  }

  if(len < 0) luaI_error(L, len, "SSL_read error");

  if(header_eof != NULL){
    lua_newtable(L);
    int idx = lua_gettop(L);

    parray_t* owo = NULL;
    //handle errors
    int err = parse_header(a->c, header_eof - a->c, &owo);
    assert(err == 0);

    for(int i = 0; i != owo->len; i++){
      luaI_tsets(L, idx, (owo->P[i].key)->c, ((str*)owo->P[i].value)->c);
    }
    //done out of pure laziness, parse_header was meant for requests but works fine for responses, change this later
    luaI_treplk(L, idx, "Path", "code");
    luaI_treplk(L, idx, "Request", "version");
    luaI_treplk(L, idx, "Version", "code-name");

    lua_pushstring(L, "code");
    lua_gettable(L, idx);
    int code = atoi(lua_tostring(L, -1));
    luaI_tseti(L, idx, "code", code);
    
    void* encoding = parray_get(owo, "Transfer-Encoding");

    struct _srequest_state *read_state = calloc(sizeof * read_state, 1);
    read_state->ctx = ctx;
    read_state->ssl = ssl;
    read_state->sock = sock;
    if(encoding != NULL){
      if(strcmp(((str*)encoding)->c, "chunked") == 0){
        struct chunked_encoding_state* state = calloc(sizeof * state, 1);
        state->reading_length = 1;
        state->buffer = str_init("");
        state->content = str_init("");

        chunked_encoding_round(header_eof + 4, extra_len - 4, state);
        memset(buffer, 0, BUFFER_LEN);
        
        read_state->buffer = str_init("");
        read_state->state = state; 
      }
    } else {
      read_state->buffer = str_initl(header_eof + 4, extra_len - 4);
    }
    parray_clear(owo, STR);

    luaI_newstream(L, _srequest_read, _srequest_free, read_state);
    int v = lua_gettop(L);
    luaI_tsetv(L, idx, "content", v);
    lua_pushvalue(L, idx);
  } else {
    luaI_error(L, -1, "error with header");
  }
  str_free(a);

  /*SSL_set_shutdown(ssl, SSL_RECEIVED_SHUTDOWN | SSL_SENT_SHUTDOWN);
  SSL_shutdown(ssl);
  SSL_free(ssl);
  SSL_CTX_free(ctx);

  close(sock);*/
  return 1;
}

int l_request(lua_State* L){
  const char* host = luaL_checkstring(L, 1);
  int sock = get_host((char*)host, (char*)lua_tostring(L, 2));
  
  char* path = "/";
  if(lua_gettop(L) >= 3)
    path = (char*)luaL_checkstring(L, 3);

  //char* req = "GET / HTTP/1.1\nHost: amyy.cc\nConnection: Close\n\n";

  char request[2000];
  sprintf(request, "GET %s HTTP/1.1\nHost: %s\nConnection: Close\n\n", path, host); 
  write(sock, request, strlen(request));

  str* a = str_init("");
  char buffer[512];
  int len = 0;

  for(; (len = read(sock, buffer, 511)) != 0;){
    str_pushl(a, buffer, len);
    memset(buffer, 0, 512);
  }

  lua_pushstring(L, a->c);

  return 1;
}

#define max_uri_size 2048

_Atomic size_t threads = 0;

void* handle_client(void *_arg){
  thread_arg_struct* args = (thread_arg_struct*)_arg;
  int client_fd = args->fd;
  char* buffer;
  int header_eof = -1;
  lua_State* L = args->L;
  luaL_openlibs(L);

  char* header = NULL;  

  int64_t bite = recv_header(client_fd, &buffer, &header);
  header_eof = header - buffer; 
  
  if(bite > 0){
    parray_t* table;

    //checks for a valid header
    int val = parse_header(buffer, header_eof, &table);

    if(val == -2) net_error(client_fd, 414);

    if(val >= 0){
      str* sk = (str*)parray_get(table, "Path");
      str* sR = (str*)parray_get(table, "Request");
      str* sT = (str*)parray_get(table, "Content-Type");
      str* sC = (str*)parray_get(table, "Cookie");
      struct file_parse* file_cont = calloc(1, sizeof * file_cont);

      lua_newtable(L);
      int files_idx = lua_gettop(L);
      lua_pushstring(L, "");
      int body_idx = lua_gettop(L);

      char portc[10] = {0};
      sprintf(portc, "%i", args->port);

      str* aa = str_init(portc);
      str_push(aa, sk->c);

      larray_t* params = larray_init();
      parray_t* v = route_match(paths, aa->c, &params);
      
      if(sT != NULL)
        rolling_file_parse(L, &files_idx, &body_idx, header + 4, sT, bite - header_eof - 4, file_cont);

      str_free(aa);
      if(v != NULL){
        lua_newtable(L);
        int req_idx = lua_gettop(L);
        lua_newtable(L);
        int res_idx = lua_gettop(L);
        
        //handle cookies
        if(sC != NULL){
          lua_newtable(L);
          int lcookie = lua_gettop(L);

          parray_t* cookie = parray_init();
          gen_parse(sC->c, sC->len, &cookie);
          for(int i = 0; i != cookie->len; i++){
            luaI_tsetsl(L, lcookie, cookie->P[i].key->c, ((str*)cookie->P[i].value)->c, ((str*)cookie->P[i].value)->len);
          }
          luaI_tsetv(L, req_idx, "cookies", lcookie);
          parray_clear(cookie, STR);

          parray_remove(table, "Cookie", NONE);
        }

        lua_pushlightuserdata(L, file_cont);

        int ld = lua_gettop(L);
        luaI_tsetv(L, req_idx, "_data", ld);
        luaI_tsetv(L, req_idx, "files", files_idx);
        luaI_tsetv(L, req_idx, "Body", body_idx);

        for(int i = 0; i != table->len; i+=1){
          //printf("'%s' :: '%s'\n",table[i]->c, table[i+1]->c);
          luaI_tsets(L, req_idx, table->P[i].key->c, ((str*)table->P[i].value)->c);
        }

        luaI_tsets(L, req_idx, "ip", inet_ntoa(args->cli.sin_addr));

        if(bite == -1){
          client_fd = -2;
        }

        luaI_tseti(L, req_idx, "_bytes", bite - header_eof - 4);
        luaI_tseti(L, req_idx, "client_fd", client_fd);
        luaI_tsetcf(L, req_idx, "roll", l_roll);
        
        //functions
        luaI_tsetcf(L, res_idx, "send", l_send);
        luaI_tsetcf(L, res_idx, "sendfile", l_sendfile);
        luaI_tsetcf(L, res_idx, "write", l_write);
        luaI_tsetcf(L, res_idx, "close", l_close);
        luaI_tsetcf(L, res_idx, "stop", l_stop);


        //values
        luaI_tseti(L, res_idx, "client_fd", client_fd);
        luaI_tsets(L, res_idx, "_request", sR->c);

        //header table
        lua_newtable(L);
        int header_idx = lua_gettop(L);
        luaI_tseti(L, header_idx, "Code", 200);
        luaI_tsets(L, header_idx, "Content-Type", "text/html");
        
        luaI_tsetv(L, res_idx, "header", header_idx);

        //get all function that kinda match
        parray_t* owo = (parray_t*)v;
        for(int i = 0; i != owo->len; i++){
          //though these are arrays of arrays we have to iterate *again*
          struct sarray_t* awa = (struct sarray_t*)owo->P[i].value;

          //push url params 
          lua_newtable(L);
          int new_param_idx = lua_gettop(L);

          int id = larray_geti(params, i);
          parray_t* par = params->arr[id].value;

          for(int z = 0; z != par->len; z++){
            luaI_tsets(L, new_param_idx, par->P[z].key->c, (char*)par->P[z].value);
          }
          parray_clear(par, FREE);

          luaI_tsetv(L, req_idx, "parameters", new_param_idx);

          for(int z = 0; z != awa->len; z++){
            struct lchar* wowa = awa->cs[z];
            //if request is HEAD, it is valid for GET and HEAD listeners 
            if(strcmp(wowa->req, "all") == 0 || strcmp(wowa->req, sR->c) == 0 ||
                (strcmp(sR->c, "HEAD") == 0 && strcmp(wowa->req, "GET") == 0)){
                  
                  luaL_loadbuffer(L, wowa->c, wowa->len, "fun");
                  int func = lua_gettop(L);
                  lua_assign_upvalues(L, func);

                  lua_pushvalue(L, res_idx); //push methods related to dealing with the request
                  lua_pushvalue(L, req_idx); //push info about the request

                  //call the function
                  if(lua_pcall(L, 2, 0, 0) != 0){
                    printf("(net thread) %s\n", lua_tostring(L, -1));
                    //send an error message if send has not been called
                    if(client_fd >= 0) net_error(client_fd, 500);
                    
                    goto net_end;
                  }

                  //check if res:stop() was called
                  lua_pushstring(L, "_stop");
                  lua_gettable(L, res_idx);
                  if(!lua_isnil(L, -1))
                    goto net_end;
                 
            }
          }
        }

net_end:
        larray_clear(params);
        parray_lclear(owo); //dont free the rest

        lua_pushstring(L, "client_fd");
        lua_gettable(L, res_idx);
        client_fd = luaL_checkinteger(L, -1);

      }

      if(file_cont->boundary != NULL) str_free(file_cont->current);
      if(file_cont->boundary != NULL) str_free(file_cont->boundary);
      if(file_cont->boundary_id != NULL) str_free(file_cont->boundary_id);

      free(file_cont);
    }
    parray_clear(table, STR);
  }

  if(client_fd != -1){
    shutdown(client_fd, 2);
    closesocket(client_fd);
  }

  free(args);
  free(buffer);
  lua_close(L);

  threads--;
  return NULL;
}

int clean_lullaby_net(lua_State* L){
  if(mime_type != NULL){
    map_clear(mime_type, FREE);
  }
  return 0;
}

int start_serv(lua_State* L, int port){
  parse_mimetypes();
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


  if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, (const char*)&(int){1}, sizeof(int)) < 0)
    p_fatal("SO_REUSEADDR refused\n");

  //bind to port
  if(bind(server_fd, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0)
    p_fatal("failed to bind to port\n");

  if(listen(server_fd, max_con) < 0)
    p_fatal("failed to listen\n");

  for(;;){
    struct sockaddr_in client_addr;
    socklen_t client_addr_len = sizeof(client_addr);
    int* client_fd = malloc(sizeof(int));

    if((*client_fd = accept(server_fd, (struct sockaddr*)&client_addr, &client_addr_len)) < 0)
      p_fatal("failed to accept\n");

    if(threads >= max_con){
      //deny request
      net_error(*client_fd, 503);
      close(*client_fd);
      free(client_fd);

      continue;
    }

    thread_arg_struct* args = malloc(sizeof * args);

    args->fd = *client_fd;
    args->port = port;
    args->cli = client_addr;
    args->L = luaL_newstate();

    int old_top = lua_gettop(L);
    lua_getglobal(L, "_G");

    luaI_copyvars(L, args->L); 
    lua_settop(L, old_top);
    lua_set_global_table(args->L);

    threads++;

    //send request to handle_client()
    pthread_t thread_id;
    pthread_create(&thread_id, NULL, handle_client, (void*)args);
    pthread_detach(thread_id);
     
    //handle_client((void*)args);
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

  struct lchar* awa;
  str* uwu = str_init("");
  lua_pushvalue(L, 3);
  lua_dump(L, writer, (void*)uwu, 0);
  
  awa = malloc(sizeof * awa);
  awa->c = uwu->c;
  awa->len = uwu->len;
  strcpy(awa->req, req);
  free(uwu); //yes this *should* be str_free but awa kinda owns it now:p 

  if(paths == NULL)
    paths = parray_init();

  //please free this
  void* v_old_paths = parray_get(paths, portss->c);
  struct sarray_t* old_paths;
  if(v_old_paths == NULL){
    old_paths = malloc(sizeof * old_paths);
    old_paths->len = 0;
    old_paths->cs = malloc(sizeof old_paths->cs);
  } else old_paths = (struct sarray_t*)v_old_paths;

  old_paths->len++;
  old_paths->cs = realloc(old_paths->cs, sizeof old_paths->cs * old_paths->len);
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
gen_reqs(all); //non standard lol, like expressjs 'use' keyword :3

int l_listen(lua_State* L){

  if(lua_gettop(L) != 2) {
    p_fatal("not enough args");
  }
  if(lua_type(L, 1) != LUA_TFUNCTION) {
    p_fatal("(arg:1) expected a function");
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
    
  lua_pushstring(L, "port");
  lua_pushvalue(L, 2);
  lua_settable(L, -3);

  lua_pushvalue(L, 1); //the function
  lua_pushvalue(L, -2); //the server table

  lua_pcall(L, 1, 0, 0);

  start_serv(L, port);
  return 0;
}
