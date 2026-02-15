#include "../lua.h"
#include "common.h"

struct ws_frame_info {
  int fin;
  int rsv1;
  int rsv2;
  int rsv3;
  int opcode;
  int mask;
  int payload;
};

int ws_read(struct net_data* data, struct ws_frame_info* frame_info);
struct ws_frame_info ws_frame_decode(char* buffer);
int l_websocket_upgrade(lua_State* L);
