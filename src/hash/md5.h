#include "../lua.h"
#include <stdint.h>

struct md5_hash {
  uint8_t* buffer;
  uint32_t a0, b0, c0, d0;
  size_t bufflen;
  size_t total;
};

struct md5_hash md5_init();
void md5_update(uint8_t*, size_t, struct md5_hash*);
void md5_final(struct md5_hash*, char*);
void md5(uint8_t*,size_t,char out_stream[64]);

int l_md5(lua_State*);
int l_md5_init(lua_State*);
int l_md5_update(lua_State*);
int l_md5_final(lua_State*);
