#include "lua.h"
#include "luai.h"
#include "common.h"

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

  int a = send(client_fd, resp->c, resp->len, 0);
  
  //
  lua_pushstring(L, "client_fd");
  lua_pushinteger(L, -1);
  lua_settable(L, res_idx);
  closesocket(client_fd);
  //printf("%i | %i\n'%s'\n%i\n",client_fd,a,resp->c,resp->len);
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

int l_stop(lua_State* L){
  int res_idx = 1;

  lua_pushstring(L, "_stop");
  lua_pushboolean(L, 1);
  lua_settable(L, res_idx);

  return 0;
}

int l_roll(lua_State* L){
  int alen;
  if(lua_gettop(L) > 2) {
    alen = luaL_checkinteger(L, 2);
  } else {
    alen = -1;
  }

  lua_pushvalue(L, 1);
  lua_pushstring(L, "_bytes");
  lua_gettable(L, 1);
  int bytes = luaL_checkinteger(L, -1);

  lua_pushstring(L, "Content-Length");
  lua_gettable(L, 1);
  if(lua_type(L, -1) == LUA_TNIL) {
    lua_pushinteger(L, -1);
    return 1;
  }
  int content_length = strtol(luaL_checkstring(L, -1), NULL, 10);
  lua_pushstring(L, "_data");
  lua_gettable(L, 1);
  struct file_parse* data = (void*)lua_topointer(L, -1); 

  lua_pushvalue(L, 1);
  lua_pushstring(L, "client_fd");
  lua_gettable(L, 1);
  int client_fd = luaL_checkinteger(L, -1);
  client_fd_errors(client_fd);
  
  fd_set rfd;
  FD_ZERO(&rfd);
  FD_SET(client_fd, &rfd);
  //printf("* %li / %li\n", bytes, content_length);
  if(bytes >= content_length){
    lua_pushinteger(L, -1);
    return 1;
  }

  if(select(client_fd+1, &rfd, NULL, NULL, &((struct timeval){.tv_sec = 0, .tv_usec = 0})) == 0){
    lua_pushinteger(L, 0);
    return 1;
  }


  //time_start(recv)
  if(alen == -1) alen = content_length - bytes;
  //printf("to read: %i\n", alen);
  char* buffer = malloc(alen * sizeof * buffer);
  int r = recv(client_fd, buffer, alen, 0);
  if(r <= 0){
    lua_pushinteger(L, r - 1);
    return 1;
  }
  //time_end("recv", recv)

  lua_pushstring(L, "_bytes");
  lua_pushinteger(L, bytes + r);
  lua_settable(L, 1);

  lua_pushstring(L, "Body");
  lua_gettable(L, 1);
  int body_idx = lua_gettop(L);

  lua_pushstring(L, "files");
  lua_gettable(L, 1);
  int files_idx = lua_gettop(L);
  //time_start(parse)
  rolling_file_parse(L, &files_idx, &body_idx, buffer, NULL, r, data);
  //time_end("parse", parse)
  luaI_tsetv(L, 1, "Body", body_idx);
  luaI_tsetv(L, 1, "files", files_idx);

  free(buffer);
  lua_pushinteger(L, r);
  return 1;
}

#define bsize 512
int l_sendfile(lua_State* L){
  int res_idx = 1;
  
  lua_pushvalue(L, res_idx);
  lua_pushstring(L, "client_fd");
  lua_gettable(L, res_idx);
  int client_fd = luaL_checkinteger(L, -1);
  client_fd_errors(client_fd);

  lua_pushvalue(L, res_idx);
  lua_pushstring(L, "header");
  lua_gettable(L, -2);
  int header = lua_gettop(L);

  char* path = (char*)luaL_checkstring(L, 2);

  if(access(path, F_OK)) {
    p_fatal("file not found"); //TODO: use diff errors here
  }
  if(access(path, R_OK)){
    p_fatal("missing permissions");
  }

  char* ext = strrchr(path, '.');
  if(ext){
    char* content_type = map_get(mime_type, ext + 1);

    if(content_type) 
      {luaI_tsets(L, header, "Content-Type", content_type);}
  }

  str* r;
  i_write_header(L, header, &r, "", 0);
  send(client_fd, r->c, r->len, 0);
  str_free(r);

  char* buffer = calloc(sizeof* buffer, bsize + 1);
  FILE* fp = fopen(path, "rb");
  fseek(fp, 0L, SEEK_END);
  size_t sz = ftell(fp);
  fseek(fp, 0L, SEEK_SET);

  for(int i = 0; i < sz; i += bsize){
    fread(buffer, sizeof * buffer, bsize, fp);
    send(client_fd, buffer, bsize > sz - i ? sz - i : bsize, 0);
  }

  free(buffer);
  fclose(fp);

  return 0;
}
