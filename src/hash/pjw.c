#include "../util.h"
#include "../crypto.h"

#include <stdio.h>
#include <stdint.h>

struct pjw_hash pjw_init(){
    return (struct pjw_hash){.hash = 0, .high = 0};
}

void pjw_update(uint8_t* in, size_t len, struct pjw_hash* hash){
    for(int i = 0; i != len; i++){
        hash->hash = (hash->hash << 4) + *in++;
        if((hash->high = (hash->hash & 0xf0000000)))
            hash->hash ^= hash->high >> 24;
        hash->hash &= ~hash->high;
    }
}

uint32_t pjw_final(struct pjw_hash* hash){
    return hash->hash;
}

uint32_t pjw(uint8_t* in, size_t len){
    struct pjw_hash a = pjw_init();
    pjw_update(in, len, &a);
    return pjw_final(&a);
}

common_hash_clone(pjw);
common_hash_init_update(pjw);

int l_pjw_final(lua_State* L){
  struct pjw_hash* a = (struct pjw_hash*)lua_touserdata(L, 1);
  uint32_t u = pjw_final(a);
  char digest[32];
  sprintf(digest,"%08x",u);
  lua_pushstring(L, digest);
  return 1;
}

int l_pjw(lua_State* L){ 
    if(lua_gettop(L) == 0) return l_pjw_init(L);
    size_t len = 0;
    uint8_t* a = (uint8_t*)luaL_checklstring(L, 1, &len);

    char digest[32];

    uint32_t u = pjw(a, len);
    sprintf(digest,"%08x",u);
    lua_pushstring(L, digest);
    return 1;
}
