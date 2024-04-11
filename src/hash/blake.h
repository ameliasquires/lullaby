#include <stdint.h>
#include "../lua.h"

#define U8TO32_BIG(p)					      \
  (((uint32_t)((p)[0]) << 24) | ((uint32_t)((p)[1]) << 16) |  \
   ((uint32_t)((p)[2]) <<  8) | ((uint32_t)((p)[3])      ))

#define U32TO8_BIG(p, v)				        \
  (p)[0] = (uint8_t)((v) >> 24); (p)[1] = (uint8_t)((v) >> 16); \
  (p)[2] = (uint8_t)((v) >>  8); (p)[3] = (uint8_t)((v)      );

#define U8TO64_BIG(p) \
  (((uint64_t)U8TO32_BIG(p) << 32) | (uint64_t)U8TO32_BIG((p) + 4))

#define U64TO8_BIG(p, v)		      \
  U32TO8_BIG((p),     (uint32_t)((v) >> 32)); \
  U32TO8_BIG((p) + 4, (uint32_t)((v)      ));

#define wtf(b) (b[0] << 24)&0xff000000 | (b[1] << 16)&0xff0000 | (b[2] << 8)&0xff00 | b[3]&0xff

enum blake256_v {
    b256, b224
};

enum blake512_v {
    b512, b384
};

int l_blake256(lua_State* L);
int l_blake256_init(lua_State* L);
int l_blake256_update(lua_State* L);
int l_blake256_final(lua_State* L);

int l_blake224(lua_State* L);
int l_blake224_init(lua_State* L);
int l_blake224_update(lua_State* L);
int l_blake224_final(lua_State* L);

int l_blake512(lua_State* L);
int l_blake512_init(lua_State* L);
int l_blake512_update(lua_State* L);
int l_blake512_final(lua_State* L);

int l_blake384(lua_State* L);
int l_blake384_init(lua_State* L);
int l_blake384_update(lua_State* L);
int l_blake384_final(lua_State* L);