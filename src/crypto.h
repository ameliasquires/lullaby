#include "lua.h"
#include "i_util.h"
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

#include "encode/uuencode.h"
#include "encode/base64.h"

unsigned i_lr(unsigned, unsigned);
unsigned i_rr(unsigned, unsigned);
uint64_t i_lr64(uint64_t, uint64_t);
uint64_t i_rr64(uint64_t, uint64_t);


static const luaL_Reg crypto_function_list [] = {
      {"sha0",l_sha0}, {"sha1",l_sha1}, {"sha256",l_sha256}, {"sha224",l_sha224},
      {"setpearson",l_setpearson}, {"pearson",l_pearson}, {"xxh64",l_xxh64},
      {"xxh32",l_xxh32}, {"adler32",l_adler32}, {"bsdchecksum",l_bsdchecksum},
      {"crc8",l_crc8}, {"crc16",l_crc16}, {"crc32",l_crc32}, {"fletcher8",l_fletcher8},
      {"fletcher16",l_fletcher16}, {"fletcher32",l_fletcher32},
      {"sysvchecksum",l_sysvchecksum}, {"xor8",l_xor8}, {"setbuzhash",l_setbuzhash},
      {"buzhash8",l_buzhash8}, {"buzhash16",l_buzhash16}, {"cityhash32", l_cityhash32},
      {"cityhash64", l_cityhash64}, {"cityhash128", l_cityhash128}, {"md5",l_md5},
      {"djb2", l_djb2}, {"farmhash32", l_farmhash32}, {"farmhash64", l_farmhash64},
      {"fasthash32", l_fasthash32}, {"fasthash64", l_fasthash64}, {"fnv_0", l_fnv_0},
      {"fnv_1", l_fnv_1}, {"fnv_a", l_fnv_a}, {"oaat", l_oaat}, {"loselose", l_loselose},
      {"metrohash64_v1", l_metrohash64_v1}, {"metrohash64_v2", l_metrohash64_v2},
      {"metrohash128_v1", l_metrohash128_v1}, {"metrohash128_v2", l_metrohash128_v2},
      {"murmur1_32", l_murmur1_32}, {"murmur2_32", l_murmur2_32}, {"pjw", l_pjw},
      {"sdbm", l_sdbm}, {"sha512", l_sha512}, {"sha384", l_sha384}, {"sha512_t", l_sha512_t},
      {"spookyhash128_v1", l_spookyhash128_v1}, {"spookyhash128_v2", l_spookyhash128_v2},
      {"spookyhash64_v1", l_spookyhash64_v1}, {"spookyhash64_v2", l_spookyhash64_v2},
      {"spookyhash32_v1", l_spookyhash32_v1}, {"spookyhash32_v2", l_spookyhash32_v2},



      {"uuencode",l_uuencode},
      {"uudecode",l_uudecode},

      {"base64encode",l_base64encode},
      {"base64decode",l_base64decode},


      {NULL,NULL}
};

