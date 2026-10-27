#include "lua.h"
#include "util.h"
#include "hash/md5.h"
#include "hash/sha01.h"
#include "hash/sha2xx.h"
#include "hash/crc.h"
#include "hash/xor.h"
#include "hash/fnv.h"
#include "hash/murmur.h"
#include "hash/sha2-256.h"
#include "hash/blake2.h"
#include "hash/blake.h"

#include "encode/uuencode.h"
#include "encode/base64.h"
#include "encode/baseN.h"

#include "config.h"

uint8_t rotl8(uint8_t, uint8_t);
uint16_t rotl16(uint16_t, uint16_t);
unsigned rotl32(unsigned, unsigned);
unsigned rotr32(unsigned, unsigned);
uint64_t rotl64(uint64_t, uint64_t);
uint64_t rotr64(uint64_t, uint64_t);

uint32_t swap32(uint32_t);

int tp(lua_State*);

#define common_hash_init_update(hashname) lua_common_hash_init_update(hashname, hashname)
#define lua_common_hash_init_update(hashname, luaname) lua_common_hash_init(hashname, luaname) lua_common_hash_update(hashname, luaname)
#define lua_common_hash_init(hashname, luaname) lua_common_hash_init_ni(hashname, luaname, hashname##_init(), hashname##_free_l)
#define lua_common_hash_init_l(hashname, luaname) lua_common_hash_init_ni(hashname, luaname, hashname##_init_l(L), hashname##_free_l)

#define common_hash_clone(hashname) lua_common_hash_clone(hashname, hashname)
#define lua_common_hash_clone(hashname, luaname) lua_common_hash_clone_oargs(hashname, luaname, l_##luaname##_init(L), *b = *a)
#define lua_common_hash_clone_oargs(hashname, luaname, oinit, copy)\
  int l_##luaname##_clone(lua_State* L){\
    struct hashname##_hash* a = (struct hashname##_hash*)lua_touserdata(L, -1);\
    oinit;\
    struct hashname##_hash* b = (struct hashname##_hash*)lua_touserdata(L, -1);\
    copy;\
    return 1;\
  }

#define lua_common_hash_meta(luaname)\
  int _##luaname##_hash_add(lua_State*L){\
    lua_pushvalue(L, 1);\
    l_##luaname##_clone(L);\
    lua_pushvalue(L, -1);\
    lua_pushvalue(L, 2);\
    return l_##luaname##_update(L);\
  }\
int _##luaname##_common_hash(lua_State* L){\
  lua_newtable(L);\
  int ti = lua_gettop(L);\
  luaI_tsetcf(L, ti, "update", l_##luaname##_update);\
  luaI_tsetcf(L, ti, "final", l_##luaname##_final);\
  \
  lua_pushvalue(L, 2);\
  lua_gettable(L, ti);\
  return 1;\
}

#define lua_common_hash_meta_def(luaname, exitf)\
  lua_newtable(L);\
int mt = lua_gettop(L);\
luaI_tsetcf(L, mt, "__index", _##luaname##_common_hash);\
luaI_tsetcf(L, mt, "__add", _##luaname##_hash_add);\
luaI_tsetcf(L, mt, "__gc", exitf);\
lua_pushvalue(L, mt);\
lua_setmetatable(L, ud);\

#define lua_common_hash_init_ni(hashname, luaname, initf, exitf)\
  lua_common_hash_meta(luaname);\
int l_##luaname##_init(lua_State* L){\
  \
  struct hashname##_hash* a = (struct hashname##_hash*)lua_newuserdata(L, sizeof * a);\
  int ud = lua_gettop(L);\
  *a = initf;\
  lua_common_hash_meta_def(luaname, exitf);\
  lua_pushvalue(L, ud);\
  return 1;\
}

#define lua_common_hash_update(hashname, luaname)\
  int l_##luaname##_update(lua_State* L){\
    struct hashname##_hash* a = (struct hashname##_hash*)lua_touserdata(L, -2);\
    size_t len = 0;\
    uint8_t* b = (uint8_t*)luaL_checklstring(L, -1, &len);\
    \
    hashname##_update(b, len, a);\
    \
    lua_pushvalue(L, -2);\
    return 1;\
  }

#define clean_lullaby_crypto luaI_nothing

static const luaL_Reg crypto_function_list [] = {
  {"murmur1_32", l_murmur1_32}, {"murmur2_32", l_murmur2_32}, 

  {"crc8",l_crc8}, {"crc8_init",l_crc8_init}, {"crc8_update",l_crc8_update}, {"crc8_final",l_crc8_final},
  {"crc16",l_crc16}, {"crc16_init",l_crc16_init}, {"crc16_update",l_crc16_update}, {"crc16_final",l_crc16_final},
  {"crc32",l_crc32}, {"crc32_init",l_crc32_init}, {"crc32_update",l_crc32_update}, {"crc32_final",l_crc32_final},
  {"fnv_0", l_fnv_0}, {"fnv_0_init", l_fnv_0_init}, {"fnv_0_update", l_fnv_0_update}, {"fnv_0_final", l_fnv_0_final},
  {"fnv_1", l_fnv_1}, {"fnv_1_init", l_fnv_1_init}, {"fnv_1_update", l_fnv_1_update}, {"fnv_1_final", l_fnv_1_final},
  {"fnv_a", l_fnv_a}, {"fnv_a_init", l_fnv_a_init}, {"fnv_a_update", l_fnv_a_update}, {"fnv_a_final", l_fnv_a_final},
  {"xor8",l_xor8}, {"xor8_init",l_xor8_init}, {"xor8_update",l_xor8_update}, {"xor8_final",l_xor8_final},
  {"md5",l_md5}, {"md5_init",l_md5_init}, {"md5_update",l_md5_update}, {"md5_final",l_md5_final},
  {"sha0",l_sha0},
  {"sha1",l_sha1},
  {"sha512", l_sha512}, {"sha512_init", l_sha512_init}, {"sha512_update", l_sha512_update}, {"sha512_final", l_sha512_final},
  {"sha384", l_sha384}, {"sha384_init", l_sha384_init}, {"sha384_update", l_sha384_update}, {"sha384_final", l_sha384_final},
  {"sha512_t", l_sha512_t}, {"sha512_t_init", l_sha512_t_init}, {"sha512_t_update", l_sha512_t_update}, {"sha512_t_final", l_sha512_t_final},
  {"sha256",l_sha256}, {"sha256_init",l_sha256_init}, {"sha256_update",l_sha256_update}, {"sha256_final",l_sha256_final},
  {"sha224",l_sha224}, {"sha224_init",l_sha224_init}, {"sha224_update",l_sha224_update}, {"sha224_final",l_sha224_final},
  {"blake256", l_blake256}, {"blake256_init", l_blake256_init}, {"blake256_update", l_blake256_update}, {"blake256_final", l_blake256_final},
  {"blake224", l_blake224}, {"blake224_init", l_blake224_init}, {"blake224_update", l_blake224_update}, {"blake224_final", l_blake224_final},
  {"blake512", l_blake512}, {"blake512_init", l_blake512_init}, {"blake512_update", l_blake512_update}, {"blake512_final", l_blake512_final},
  {"blake384", l_blake384}, {"blake384_init", l_blake384_init}, {"blake384_update", l_blake384_update}, {"blake384_final", l_blake384_final},
  {"blake2s", l_blake2s}, {"blake2s_init", l_blake2s_init}, {"blake2s_update", l_blake2s_update}, {"blake2s_final", l_blake2s_final}, 
  {"blake2b", l_blake2b}, {"blake2b_init", l_blake2b_init}, {"blake2b_update", l_blake2b_update}, {"blake2b_final", l_blake2b_final}, 

  {"uuencode",l_uuencode},
  {"uudecode",l_uudecode},

  {"base64encode",l_base64encode},
  {"base64decode",l_base64decode},

  {"baseconvert",l_baseconvert},

  {NULL,NULL}
};

static struct config crypto_config[] = {
  {.type = c_none}
};
