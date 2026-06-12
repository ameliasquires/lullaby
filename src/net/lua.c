#include "lua.h"
#include "luai.h"
#include "util.h"
#include "common.h"
#include "../hash/fnv.h"
#include "websocket.h"

int l_write(lua_State* L){
  int res_idx = 1;

  lua_pushvalue(L, 1);
  lua_pushstring(L, "_request");
  lua_gettable(L, -2);

  int head = strcmp(luaL_checkstring(L, -1), "HEAD") == 0;

  lua_pushvalue(L, res_idx);
  lua_pushstring(L, "_");
  lua_gettable(L, res_idx);
  struct net_data* ctx = lua_touserdata(L, -1);
  

  client_fd_errors(ctx->sock);

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

  net_ctx_write(ctx, resp->c, resp->len);

  str_free(resp);
  return 0;
}

int l_send(lua_State* L){
  int res_idx = 1;
  lua_pushvalue(L, res_idx);
  lua_pushstring(L, "_");
  lua_gettable(L, res_idx);
  struct net_data* ctx = lua_touserdata(L, -1);

  client_fd_errors(ctx->sock);

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

  net_ctx_write(ctx, resp->c, resp->len);

  if(ctx->ssl != NULL) SSL_shutdown(ctx->ssl);
  else closesocket(ctx->sock);
  ctx->sock = -1;

  lua_pushboolean(L, 0);
  lua_setfield(L, res_idx, "open");

  //printf("%i | %i\n'%s'\n%i\n",client_fd,a,resp->c,resp->len);
  str_free(resp);
  return 0;
}

int l_neterror(lua_State* L){
  int res_idx = 1;
  lua_getfield(L, res_idx, "_");
  struct net_data* ctx = lua_touserdata(L, -1);

  client_fd_errors(ctx->sock);

  net_error(ctx, luaL_checkinteger(L, 2));

  lua_pushboolean(L, 0);
  lua_setfield(L, res_idx, "open");

  return 0;
}

int l_close(lua_State* L){
  int res_idx = 1;

  lua_pushvalue(L, res_idx);
  lua_pushstring(L, "_");
  lua_gettable(L, res_idx);
  struct net_data* ctx = lua_touserdata(L, -1);
  client_fd_errors(ctx->sock);

  if(ctx->ssl != NULL) SSL_shutdown(ctx->ssl);
  else closesocket(ctx->sock);
  ctx->sock = -1;

  lua_pushboolean(L, 0);
  lua_setfield(L, res_idx, "open");

  return 0;
}

int l_stop(lua_State* L){
  int res_idx = 1;

  lua_pushstring(L, "_stop");
  lua_pushboolean(L, 1);
  lua_settable(L, res_idx);

  return 0;
}

int l_load(lua_State* L){
  ssize_t alen;
  if(lua_gettop(L) >= 2) {
    alen = luaL_checkinteger(L, 2);
  } else {
    alen = -1;
  }

  lua_getfield(L, 1, "_bytes");
  int64_t bytes = luaL_checkinteger(L, -1);

  lua_getfield(L, 1, "content-length");
  luaI_assert(L, lua_type(L, -1) != LUA_TNIL);
  uint64_t content_length = strtol(luaL_checkstring(L, -1), NULL, 10);

  if(bytes >= content_length){
    lua_pushinteger(L, 0);
    return 1;
  }

  lua_getfield(L, 1, "res");
  lua_getfield(L, -1, "_");
  struct net_data* ctx = lua_touserdata(L, -1);

  lua_getfield(L, 1, "_data");
  struct file_parse* data = lua_touserdata(L, -1);

  client_fd_errors(ctx->sock);

  lua_getfield(L, 1, "body");
  int body_idx = lua_gettop(L);
  lua_getfield(L, 1, "files");
  int files_idx = lua_gettop(L);

  if(alen == -1) str_loadatleast(data->current, content_length);
  char* buffer = malloc(sizeof * buffer * BUFFER_SIZE);
  ssize_t total = bytes;
  int over = 1;

  if(alen == -1) alen = content_length - bytes;
  else alen = bytes + alen;

  for(;total < alen;){
    ssize_t read = BUFFER_SIZE;
    if(total + BUFFER_SIZE > alen) read = alen - total;

    memset(buffer, 0, BUFFER_SIZE);

    ssize_t out = net_ctx_read(ctx, buffer, read);

    if(out == 0) {
      over = 0;
      break;
    }
    if(out == -1) {
      luaI_error(L, -2, "net read error");
    }

    total += out;

    http_body_parse(L, &files_idx, &body_idx, buffer, NULL, out, data);
  }

  lua_pushinteger(L, total);
  lua_setfield(L, 1, "_bytes");

  free(buffer);

  lua_pushinteger(L, over);
  return 1;
}

#define bsize 32768
//#define bsize 12
int l_sendfile(lua_State* L){
  int res_idx = 1;
  int attachment = 0;

  char* path = (char*)luaL_checkstring(L, 2);
  char* filename = path;

  if(lua_gettop(L) > 2 && lua_type(L, 3) == LUA_TTABLE){
    lua_pushstring(L, "attachment");
    lua_gettable(L, 3);
    if(!lua_isnil(L, -1)) attachment = lua_toboolean(L, -1);

    lua_pushstring(L, "filename");
    lua_gettable(L, 3);
    if(!lua_isnil(L, -1)) filename = (char*)lua_tostring(L, -1);
  }

  luaI_assert(L, !access(path, F_OK) /*file not found*/);
  luaI_assert(L, !access(path, R_OK) /*missing permissions*/);

  lua_pushvalue(L, res_idx);
  lua_pushstring(L, "_");
  lua_gettable(L, res_idx);
  struct net_data* ctx = lua_touserdata(L, -1);
  client_fd_errors(ctx->sock);

  lua_pushvalue(L, res_idx);
  lua_pushstring(L, "header");
  lua_gettable(L, -2);
  int header = lua_gettop(L);

  lua_pushstring(L, "content-type");
  lua_gettable(L, header);

  char* ext = strrchr(path, '.');
  if(lua_isnil(L, -1) && ext && mime_type != NULL){
    char* content_type = map_get(mime_type, ext + 1);

    if(content_type) 
    {luaI_tsets(L, header, "content-type", content_type);}
  }

  char* buffer = calloc(sizeof* buffer, bsize + 1);
  FILE* fp = fopen(path, "rb");
  fseek(fp, 0L, SEEK_END);
  size_t sz = ftell(fp);
  fseek(fp, 0L, SEEK_SET);

  char size[256];
  sprintf(size, "%li", sz);
  luaI_tsets(L, header, "content-length", size);

  if(attachment) {
    char disp[256];
    sprintf(disp, "attachment; filename=\"%s\"", filename);
    luaI_tsets(L, header, "content-disposition", disp);
  } else {
    luaI_tsets(L, header, "content-disposition", "inline;");
  }

  str* r;
  i_write_header(L, header, &r, "", 0);
  net_ctx_write(ctx, r->c, r->len);
  str_free(r);

  for(size_t i = 0; i < sz; i += bsize){
    fread(buffer, sizeof * buffer, bsize, fp);
    if(net_ctx_write(ctx, buffer, bsize > sz - i ? sz - i : bsize) == -1)
      break;
  }

  lua_pushboolean(L, 0);
  lua_setfield(L, res_idx, "open");

  free(buffer);
  fclose(fp);

  return 0;
}

int l_connection_upgrade(lua_State* L){
  int res_idx = 1;
  lua_getfield(L, res_idx, "req");
  int req_idx = 2;

  lua_getfield(L, req_idx, "upgrade");
  uint64_t hash;
  size_t len;
  uint8_t* s = (uint8_t*)luaL_checklstring(L, -1, &len);
  hash = fnv_1(s, len, v_1);

  switch(hash){
    case 0xf042f81495060e72: //websocket
        l_websocket_upgrade(L);
      break;
    default:
      luaI_error(L, -1, "can't upgrade");
      break;
  }

  return 0;
}
