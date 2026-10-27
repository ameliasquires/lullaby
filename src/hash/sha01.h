#include "lua.h"
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

#define sha01_bs 64
#define sha01_digest_len 20

struct sha01_hash {
  uint8_t* buffer;
  uint32_t h0, h1, h2, h3, h4;
  size_t bufflen;
  size_t total;
  uint8_t version;
};

struct sha01_hash* sha01_init(uint8_t ver);
struct sha01_hash* sha01_clone(struct sha01_hash* hash);
int sha01_free(struct sha01_hash* hash);
void sha01_round(struct sha01_hash* hash);
void sha01_update(uint8_t* input, size_t len, struct sha01_hash* hash);
void sha01_digest(struct sha01_hash* hash, uint8_t* out);

// lua ---

int l_sha01_obj(lua_State* L, struct sha01_hash* hash);
int l_sha1(lua_State* L);
int l_sha0(lua_State* L);
