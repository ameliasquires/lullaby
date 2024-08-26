#include "../crypto.h"
#include <inttypes.h>
#include <stdint.h>

struct fnv_1_hash fnv_1_init(enum fnv_version A){
  return (struct fnv_1_hash){.A = A, .hash = (A != v_0) * 0xcbf29ce484222325};
}

void fnv_1_update(uint8_t* in, size_t len, struct fnv_1_hash* hash){
  uint64_t prime = 0x100000001b3;

  for(int i = 0; i != len; i++){
    switch(hash->A){
      case v_1:
      case v_0:
        hash->hash *= prime;
        hash->hash ^= in[i];
        break;
      case v_a:
        hash->hash ^= in[i];
        hash->hash *= prime;
        break;
    }
  }
}

uint64_t fnv_1_final(struct fnv_1_hash* hash){
  return hash->hash;
}

uint64_t fnv_1(uint8_t* in, size_t len, enum fnv_version A){
  struct fnv_1_hash a = fnv_1_init(A);
  fnv_1_update(in, len, &a);
  return fnv_1_final(&a);
}

lua_common_hash_clone(fnv_1, fnv_0);
lua_common_hash_clone(fnv_1, fnv_1);
lua_common_hash_clone(fnv_1, fnv_a);

lua_common_hash_update(fnv_1, fnv_1);
lua_common_hash_update(fnv_1, fnv_0);
lua_common_hash_update(fnv_1, fnv_a);

lua_common_hash_init_ni(fnv_1, fnv_1, fnv_1_init(v_1));
lua_common_hash_init_ni(fnv_1, fnv_0, fnv_1_init(v_0));
lua_common_hash_init_ni(fnv_1, fnv_a, fnv_1_init(v_a));

#define aaa(v)\
int l_fnv_##v##_final(lua_State* L){\
  struct fnv_1_hash* a = (struct fnv_1_hash*)lua_touserdata(L, 1);\
  uint64_t u = fnv_1_final(a);\
  char digest[64];\
  sprintf(digest,"%16"PRIx64,u);\
  lua_pushstring(L, digest);\
  return 1;\
}

aaa(0);
aaa(1);
aaa(a);

int l_fnv_0(lua_State* L){
  if(lua_gettop(L) == 0) return l_fnv_0_init(L);
  size_t len = 0;
  uint8_t* a = (uint8_t*)luaL_checklstring(L, 1, &len);

  char digest[64];

  uint64_t u = fnv_1(a, len, v_0);
  sprintf(digest,"%16"PRIx64,u);
  lua_pushstring(L, digest);
  return 1;
}

int l_fnv_1(lua_State* L){ 
  if(lua_gettop(L) == 0) return l_fnv_1_init(L);
  size_t len = 0;
  uint8_t* a = (uint8_t*)luaL_checklstring(L, 1, &len);

  char digest[64];

  uint64_t u = fnv_1(a, len, v_1);
  sprintf(digest,"%16"PRIx64,u);
  lua_pushstring(L, digest);
  return 1;
}

int l_fnv_a(lua_State* L){ 
  if(lua_gettop(L) == 0) return l_fnv_a_init(L);
  size_t len = 0;
  uint8_t* a = (uint8_t*)luaL_checklstring(L, 1, &len);

  char digest[64];

  uint64_t u = fnv_1(a, len, v_a);
  sprintf(digest,"%16"PRIx64,u);
  lua_pushstring(L, digest);
  return 1;
}
