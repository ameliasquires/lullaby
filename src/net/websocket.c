#include "websocket.h"
#include "common.h"
#include "util.h"
#include "../lua.h"
#include "../hash/sha01.h"
#include "../encode/base64.h"
#include <limits.h>
#include <ctype.h>
#include "../error.h"

//why?????
const char* magic = "258EAFA5-E914-47DA-95CA-C5AB0DC85B11";

//calloc or zero {out}
void hex_decode(char* in, char* out, uint64_t* outlen){
  uint64_t len = 0;
  for(int i = 0; in[i] != '\0'; i++, len++){
    int value = 0;
    char c = in[i];
    if(c >= '0' && c <= '9')
      value = c - '0';
    else if(c >= 'A' && c <= 'F')
      value = 10 + (c - 'A');
    else if(c >= 'a' && c <= 'f')
      value = 10 + (c - 'a');

    out[(i/2)] += value << (((i + 1) % 2) * 4);
    //out[i/2] += strtoul((char[]){in[i], in[i+1]}, 0, 16);
  }

  *outlen = len/2;
}

struct ws_frame_info ws_frame_decode(char* buffer){
  struct ws_frame_info frame_info = {.fin = (buffer[0] >> 7) & 1, .rsv1 = (buffer[0] >> 6) & 1,
    .rsv2 = (buffer[0] >> 5) & 1, .rsv3 = (buffer[0] >> 4) & 1, .opcode = buffer[0] & 0b1111,
    .mask = (buffer[1] >> 7) & 1, .payload = buffer[1] & 0b1111111};
  return frame_info;
}

void ws_frame_build(struct ws_frame_info frame, uint64_t extended, char* mkey, str* out){
  uint8_t d = (frame.fin << 7) | (frame.rsv1 << 6) | (frame.rsv2 << 5) | (frame.rsv3 << 4) | frame.opcode;
  str_pushc(out, d);
  frame.payload = extended;
  if(extended > 125) frame.payload = 126;
  if(extended > UINT16_MAX) frame.payload = 127;
  d = (frame.mask << 7) | frame.payload;
  str_pushc(out, d);

  if(frame.payload == 126){
    uint16_t sh = extended;
    for(int i = 2; i != 0; i--)
      str_pushc(out, ((uint8_t*)&sh)[i - 1]);
  } else if(frame.payload == 127){
    for(int i = 8; i != 0; i--)
      str_pushc(out, ((uint8_t*)&extended)[i - 1]);
  }

  if(frame.mask){
    str_pushl(out, mkey, 4);
  }
}

#define BUFFER_LEN 4096

int ws_read(struct net_data* data, struct ws_frame_info* frame_info){
  char buffer[BUFFER_LEN] = {0};
  int total_len = 0;
  int len;

  for(; (len = net_ctx_read(data, buffer + total_len, 2 - total_len)) > 0;){ 
    total_len += len;
    if(total_len >= 2) break;
  }

  if(len < 0 || total_len <= 0) return -1;

  uint64_t payload = 0;
  *frame_info = ws_frame_decode(buffer);
  memset(buffer, 0, total_len);
  total_len = 0;

  if(frame_info->payload <= 125) payload = frame_info->payload;
  else if(frame_info->payload == 126) {
    for(; (len = net_ctx_read(data, buffer + total_len, 2 - total_len)) > 0;){ 
      total_len += len;
      if(total_len >= 2) break;
    }

    if(len < 0) return -1;

    payload = (buffer[0] & 0xff) << 8 | (buffer[1] & 0xff);
  } else {
    for(; (len = net_ctx_read(data, buffer + total_len, 8 - total_len)) > 0;){
      total_len += len;
      if(total_len >= 8) break;
    }

    if(len < 0) return -1;

    payload = ((uint64_t)buffer[0] & 0xff) << 56 | ((uint64_t)buffer[1] & 0xff) << 48 | ((uint64_t)buffer[2] & 0xff) << 40 |
      ((uint64_t)buffer[3] & 0xff) << 32 | (buffer[4] & 0xff) << 24 | (buffer[5] & 0xff) << 16 | (buffer[6] & 0xff) << 8 | (buffer[7] & 0xff);
  }

  total_len = 0;
  uint8_t mask[4] = {0};
  if(frame_info->mask){
    for(; (len = net_ctx_read(data, buffer + total_len, 4 - total_len)) > 0;){
      total_len += len;
      if(total_len >= 4) break;
    }
    mask[0] = buffer[0];
    mask[1] = buffer[1];
    mask[2] = buffer[2];
    mask[3] = buffer[3];
  }

  uint64_t i = 0;
  memset(buffer, 0, BUFFER_LEN);
  for(; data->buffer->len != payload && (len = net_ctx_read(data, buffer, lesser(payload - data->buffer->len, BUFFER_LEN))) > 0;){ 
    if(frame_info->mask){
      for(int z = 0; z != len; z++,i++)
        buffer[z] ^= mask[i % 4];
    }
    str_pushl(data->buffer, buffer, len);
    memset(buffer, 0, len);
  }

  if(len < 0) return -1;

  return 1;
}

int l_ws_read(lua_State* L){
  lua_getfield(L, 1, "_ws");
  struct net_data* data = lua_touserdata(L, -1);
  struct ws_frame_info frame = {};
  int c = ws_read(data, &frame);
  if(c == -1){
    str_clear(data->buffer);
    return luaI_error(L, "SSL_read error");
  }

  lua_newtable(L);
  int idx = lua_gettop(L);
  luaI_tsetsl(L, idx, "content", data->buffer->c, data->buffer->len);
  luaI_tseti(L, idx, "opcode", frame.opcode);

  str_clear(data->buffer);
  return 1;
}

int l_ws_write(lua_State* L){
  lua_getfield(L, 1, "_ws");
  struct net_data* data = lua_touserdata(L, -1);
  struct ws_frame_info frame = {};
  frame.rsv1 = 0;
  frame.fin = 1;
  frame.mask = 0;
  frame.opcode = 0b0001;

  size_t len;
  const char* s = lua_tolstring(L, 2, &len);

  str* f = str_init("");
  ws_frame_build(frame, len, NULL, f);
  str_pushl(f, s, len);
  write(data->sock, f->c, f->len);
  str_free(f);

  return 0;
}

int l_websocket_upgrade(lua_State* L){
  int res_idx = 1;
  int req_idx = 2;
  
  lua_getfield(L, res_idx, "_");
  struct net_data* ctx = lua_touserdata(L, -1);

  lua_getfield(L, req_idx, "sec-websocket-key");
  const char* wskey = luaL_checklstring(L, -1, NULL);
  str* newkey = str_init(wskey);
  str_push(newkey, magic);
  char* sha = calloc(1512, sizeof * sha);
  printf("%s\n", newkey->c);
  sha1((uint8_t*)newkey->c, newkey->len, sha);
  printf("%s\n", sha);
  char* bin = calloc(512, sizeof * bin);
  uint64_t len;
  hex_decode(sha, bin, &len);
  char* b64 = calloc(512 * 3, sizeof * b64);
  en_base64(bin, len, b64);

  char* upgrade = calloc(8192, sizeof * upgrade);
  sprintf(upgrade, "HTTP/1.1 101 Switching Protocols\r\n"
                   "upgrade: websocket\r\n"
                   "connection: upgrade\r\n"
                   "sec-websocket-accept: %s\r\n"
                   "\r\n", b64);

  printf("%s\n", upgrade);
  net_ctx_write(ctx, upgrade, strlen(upgrade));
  free(upgrade);
  free(sha);
  free(bin);
  free(b64);
  str_free(newkey);

  struct net_data *data = calloc(1, sizeof * data);
  data->sock = ctx->sock;
  data->ssl = ctx->ssl;
  data->ctx = ctx->ctx;
  data->buffer = str_init("");

  luaI_tsetnil(L, req_idx, "load");
  luaI_tsetlud(L, res_idx, "_ws", data);
#warning "missing ws commands"
  luaI_tsetcf(L, res_idx, "send", l_ws_write);
  //luaI_tsetcf(L, res_idx, "write", );
  //luaI_tsetcf(L, res_idx, "sendfile", );
  luaI_tsetcf(L, res_idx, "read", l_ws_read);
  //luaI_tsetcf(L, res_idx, "close", );

  return 1;
}
