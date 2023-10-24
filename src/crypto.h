#include "lua.h"
#include "i_util.h"
#include "hash/md5.h"
#include "hash/sha01.h"

unsigned i_lr(unsigned, unsigned);

static const luaL_Reg crypto_function_list [] = {
      {"md5",l_md5},
      {"sha0",l_sha0},
      {"sha1",l_sha1},
 
      {NULL,NULL}
};

