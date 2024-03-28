#include "lua.h"
#include "util.h"
#include "hash/md5.h"
#include "hash/sha01.h"
#include "hash/sha2xx.h"
#include "hash/pearson.h"
#include "hash/xxh.h"
#include "hash/adler.h"
#include "hash/bsdchecksum.h"
#include "hash/crc.h"
#include "hash/fletcher.h"
#include "hash/sysvchecksum.h"
#include "hash/xor.h"
#include "hash/buzhash.h"
#include "hash/cityhash.h"
#include "hash/djb2.h"
#include "hash/farmhash.h"
#include "hash/fasthash.h"
#include "hash/fnv.h"
#include "hash/jenkins.h"
#include "hash/loselose.h"
#include "hash/metrohash.h"
#include "hash/murmur.h"
#include "hash/pjw.h"
#include "hash/sdbm.h"
#include "hash/sha2-256.h"
#include "hash/spookyhash.h"
#include "hash/blake2.h"
#include "hash/blake.h"

#include "encode/uuencode.h"
#include "encode/base64.h"
#include "encode/baseN.h"

uint8_t rotl8(uint8_t, uint8_t);
uint16_t rotl16(uint16_t, uint16_t);
unsigned rotl32(unsigned, unsigned);
unsigned rotr32(unsigned, unsigned);
uint64_t rotl64(uint64_t, uint64_t);
uint64_t rotr64(uint64_t, uint64_t);

#define common_hash_init_update(hashname) lua_common_hash_init_update(hashname, hashname)
#define lua_common_hash_init_update(hashname, luaname) lua_common_hash_init(hashname, luaname) lua_common_hash_update(hashname, luaname)
#define lua_common_hash_init(hashname, luaname)\
 int l_##luaname##_init(lua_State* L){\
  lua_newtable(L);\
  int t = lua_gettop(L);\
  \
  struct hashname##_hash* a = (struct hashname##_hash*)lua_newuserdata(L, sizeof * a);\
  int ud = lua_gettop(L);\
  *a = hashname##_init();\
  \
  luaI_tsetv(L, t, "ud", ud);\
  luaI_tsetcf(L, t, "update", l_##luaname##_update);\
  luaI_tsetcf(L, t, "final", l_##luaname##_final);\
  \
  lua_pushvalue(L, t);\
  return 1;\
}\

#define lua_common_hash_init_warg(hashname, luaname, hcode, arg)\
 int l_##luaname##_init(lua_State* L){\
  hcode;\
  lua_newtable(L);\
  int t = lua_gettop(L);\
  \
  struct hashname##_hash* a = (struct hashname##_hash*)lua_newuserdata(L, sizeof * a);\
  int ud = lua_gettop(L);\
  *a = hashname##_init(arg);\
  \
  luaI_tsetv(L, t, "ud", ud);\
  luaI_tsetcf(L, t, "update", l_##luaname##_update);\
  luaI_tsetcf(L, t, "final", l_##luaname##_final);\
  \
  lua_pushvalue(L, t);\
  return 1;\
}\

#define lua_common_hash_update(hashname, luaname)\
int l_##luaname##_update(lua_State* L){\
  lua_pushstring(L, "ud");\
  lua_gettable(L, 1);\
  \
  struct hashname##_hash* a = (struct hashname##_hash*)lua_touserdata(L, -1);\
  size_t len = 0;\
  uint8_t* b = (uint8_t*)luaL_checklstring(L, 2, &len);\
  \
  hashname##_update(b, len, a);\
  \
  lua_pushvalue(L, 1);\
  return 1;\
}

static const luaL_Reg crypto_function_list [] = {
      {"sha0",l_sha0}, {"sha1",l_sha1}, {"sha256",l_sha256}, {"sha224",l_sha224},
      {"setpearson",l_setpearson}, {"pearson",l_pearson}, {"xxh64",l_xxh64},
      {"xxh32",l_xxh32},  {"fletcher8",l_fletcher8},
      {"fletcher16",l_fletcher16}, {"fletcher32",l_fletcher32},
      {"sysvchecksum",l_sysvchecksum}, {"xor8",l_xor8}, {"setbuzhash",l_setbuzhash},
      {"cityhash32", l_cityhash32},
      {"cityhash64", l_cityhash64}, {"cityhash128", l_cityhash128}, {"md5",l_md5},
      {"farmhash32", l_farmhash32}, {"farmhash64", l_farmhash64},
      {"fasthash32", l_fasthash32}, {"fasthash64", l_fasthash64},
      {"loselose", l_loselose},
      {"metrohash64_v1", l_metrohash64_v1}, {"metrohash64_v2", l_metrohash64_v2},
      {"metrohash128_v1", l_metrohash128_v1}, {"metrohash128_v2", l_metrohash128_v2},
      {"murmur1_32", l_murmur1_32}, {"murmur2_32", l_murmur2_32}, {"pjw", l_pjw},
      {"sdbm", l_sdbm}, {"sha512", l_sha512}, {"sha384", l_sha384}, {"sha512_t", l_sha512_t},
      {"spookyhash128_v1", l_spookyhash128_v1}, {"spookyhash128_v2", l_spookyhash128_v2},
      {"spookyhash64_v1", l_spookyhash64_v1}, {"spookyhash64_v2", l_spookyhash64_v2},
      {"spookyhash32_v1", l_spookyhash32_v1}, {"spookyhash32_v2", l_spookyhash32_v2},
      {"blake2b", l_blake2b}, {"blake2s", l_blake2s}, {"blake256", l_blake256},
      {"blake224", l_blake224}, {"blake512", l_blake512}, {"blake384", l_blake384},

      {"adler32",l_adler32}, {"adler32_init",l_adler32_init}, {"adler32_update",l_adler32_update}, {"adler32_final",l_adler32_final},
      {"bsdchecksum",l_bsdchecksum}, {"bsdchecksum_init",l_bsdchecksum_init}, {"bsdchecksum_update",l_bsdchecksum_update}, {"bsdchecksum_final",l_bsdchecksum_final},
      {"buzhash8",l_buzhash8}, {"buzhash16",l_buzhash16},
      {"crc8",l_crc8}, {"crc8_init",l_crc8_init}, {"crc8_update",l_crc8_update}, {"crc8_final",l_crc8_final},
      {"crc16",l_crc16}, {"crc16_init",l_crc16_init}, {"crc16_update",l_crc16_update}, {"crc16_final",l_crc16_final},
      {"crc32",l_crc32}, {"crc32_init",l_crc32_init}, {"crc32_update",l_crc32_update}, {"crc32_final",l_crc32_final},
      {"djb2", l_djb2}, {"djb2_init", l_djb2_init}, {"djb2_update", l_djb2_update}, {"djb2_final", l_djb2_final},
      {"fletcher8",l_fletcher8}, {"fletcher8_init",l_fletcher8_init}, {"fletcher8_update",l_fletcher8_update}, {"fletcher8_final",l_fletcher8_final},
      {"fletcher16",l_fletcher16}, {"fletcher16_init",l_fletcher16_init}, {"fletcher16_update",l_fletcher16_update}, {"fletcher16_final",l_fletcher16_final},
      {"fletcher32",l_fletcher32}, {"fletcher32_init",l_fletcher32_init}, {"fletcher32_update",l_fletcher32_update}, {"fletcher32_final",l_fletcher32_final},
      {"fnv_0", l_fnv_0}, {"fnv_0_init", l_fnv_0_init}, {"fnv_0_update", l_fnv_0_update}, {"fnv_0_final", l_fnv_0_final},
      {"fnv_1", l_fnv_1}, {"fnv_1_init", l_fnv_1_init}, {"fnv_1_update", l_fnv_1_update}, {"fnv_1_final", l_fnv_1_final},
      {"fnv_a", l_fnv_a}, {"fnv_a_init", l_fnv_a_init}, {"fnv_a_update", l_fnv_a_update}, {"fnv_a_final", l_fnv_a_final},
      {"oaat", l_oaat}, {"oaat_init", l_oaat_init}, {"oaat_update", l_oaat_update}, {"oaat_final", l_oaat_final},
      
      {"uuencode",l_uuencode},
      {"uudecode",l_uudecode},

      {"base64encode",l_base64encode},
      {"base64decode",l_base64decode},

      {"baseconvert",l_baseconvert},

      {NULL,NULL}
};

