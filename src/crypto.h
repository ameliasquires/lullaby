#include "lua.h"
#include "i_util.h"
#include "hash/md5.h"
#include "hash/sha01.h"
#include "hash/sha2xx.h"

unsigned i_lr(unsigned, unsigned);
unsigned i_rr(unsigned, unsigned);

static const luaL_Reg crypto_function_list [] = {
      {"md5",l_md5},
      {"sha0",l_sha0},
      {"sha1",l_sha1},
      {"sha256",l_sha256},
      {"sha224",l_sha224},

      {NULL,NULL}
};

