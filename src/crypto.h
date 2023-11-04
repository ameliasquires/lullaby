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

#include "encode/uuencode.h"
#include "encode/base64.h"

unsigned i_lr(unsigned, unsigned);
unsigned i_rr(unsigned, unsigned);

static const luaL_Reg crypto_function_list [] = {
      {"md5",l_md5},
      {"sha0",l_sha0},
      {"sha1",l_sha1},
      {"sha256",l_sha256},
      {"sha224",l_sha224},
      {"setpearson",l_setpearson},
      {"pearson",l_pearson},
      {"xxh64",l_xxh64},
      {"xxh32",l_xxh32},
      {"adler32",l_adler32},
      {"bsdchecksum",l_bsdchecksum},
      {"crc8",l_crc8},
      {"crc16",l_crc16},
      {"crc32",l_crc32},
      {"fletcher8",l_fletcher8},
      {"fletcher16",l_fletcher16},
      {"fletcher32",l_fletcher32},
      {"sysvchecksum",l_sysvchecksum},
      {"xor8",l_xor8},

      {"uuencode",l_uuencode},
      {"uudecode",l_uudecode},

      {"base64encode",l_base64encode},
      {"base64decode",l_base64decode},


      {NULL,NULL}
};

