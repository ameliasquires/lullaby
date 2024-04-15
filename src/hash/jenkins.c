#include "../util.h"
#include "../crypto.h"
#include <stdio.h>
#include <stdint.h>

struct jenkins_oaat_hash jenkins_oaat_init(){
  return (struct jenkins_oaat_hash){.hash = 0};
}

void jenkins_oaat_update(uint8_t* in, size_t len, struct jenkins_oaat_hash* hash){
  for(int i = 0; i != len;){
    hash->hash += in[i++];
    hash->hash += hash->hash << 10;
    hash->hash ^= hash->hash >> 6;
  }
}

uint32_t jenkins_oaat_final(struct jenkins_oaat_hash* hash){
  uint32_t h = hash->hash;
  h += h << 3;
  h ^= h >> 11;
  h += h << 15;

  return h;
}


uint32_t jenkins_oaat(uint8_t* in, size_t len){
  struct jenkins_oaat_hash a = jenkins_oaat_init();
  jenkins_oaat_update(in, len, &a);
  return jenkins_oaat_final(&a);
}

lua_common_hash_clone(jenkins_oaat, oaat);
lua_common_hash_init_update(jenkins_oaat, oaat);

int l_oaat_final(lua_State* L){
  struct jenkins_oaat_hash* a = (struct jenkins_oaat_hash*)lua_touserdata(L, 1);
  uint32_t u = jenkins_oaat_final(a);
  char digest[64];
  sprintf(digest,"%04x",u);
  lua_pushstring(L, digest);
  return 1;
}

int l_oaat(lua_State* L){ 
  if(lua_gettop(L) == 0) return l_oaat_init(L);
  size_t len = 0;
  uint8_t* a = (uint8_t*)luaL_checklstring(L, 1, &len);

  char digest[64];

  uint32_t u = jenkins_oaat(a, len);
  sprintf(digest,"%04x",u);
  lua_pushstring(L, digest);
  return 1;
}
