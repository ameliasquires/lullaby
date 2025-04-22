#include "../crypto.h"
#include <stdio.h>
#include <stdint.h>

int fletcher8_free_l(lua_State* L){return 0;}
int fletcher16_free_l(lua_State* L){return 0;}
int fletcher32_free_l(lua_State* L){return 0;}

struct fletcher8_hash fletcher8_init(){
  return (struct fletcher8_hash){.s1 = 0, .s2 = 0};
}

void fletcher8_update(uint8_t *aa, size_t len, struct fletcher8_hash *hash){
  for(int i = 0; i != len; i++){
      hash->s1 = (hash->s1 + aa[i]) % 15;
      hash->s2 = (hash->s2 + hash->s1) % 15;
  }
}

uint8_t fletcher8_final(struct fletcher8_hash *hash){
  return (hash->s2 << 4) | hash->s1;
}

uint8_t fletcher8(uint8_t *aa, size_t len){
  struct fletcher8_hash a = fletcher8_init();
  fletcher8_update(aa, len, &a);
  return fletcher8_final(&a);
}

struct fletcher16_hash fletcher16_init(){
  return (struct fletcher16_hash){.s1 = 0, .s2 = 0};
}

void fletcher16_update(uint8_t *aa, size_t len, struct fletcher16_hash *hash){
  for(int i = 0; i != len; i++){
      hash->s1 = (hash->s1 + aa[i]) % 255;
      hash->s2 = (hash->s2 + hash->s1) % 255;
  }
}

uint16_t fletcher16_final(struct fletcher16_hash *hash){
  return (hash->s2 << 8) | hash->s1;
}

uint16_t fletcher16(uint8_t *aa, size_t len){
  struct fletcher16_hash a = fletcher16_init();
  fletcher16_update(aa, len, &a);
  return fletcher16_final(&a);
}

struct fletcher32_hash fletcher32_init(){
  return (struct fletcher32_hash){.s1 = 0, .s2 = 0};
}

void fletcher32_update(uint8_t *aa, size_t len, struct fletcher32_hash *hash){
  for(int i = 0; i != len; i++){
      hash->s1 = (hash->s1 + aa[i]) % 65535;
      hash->s2 = (hash->s2 + hash->s1) % 65535;
  }
}

uint32_t fletcher32_final(struct fletcher32_hash *hash){
  return (hash->s2 << 16) | hash->s1;
}

uint32_t fletcher32(uint8_t *aa, size_t len){
  struct fletcher32_hash a = fletcher32_init();
  fletcher32_update(aa, len, &a);
  return fletcher32_final(&a);
}

common_hash_clone(fletcher32);
common_hash_clone(fletcher16);
common_hash_clone(fletcher8);

common_hash_init_update(fletcher8);
common_hash_init_update(fletcher16);
common_hash_init_update(fletcher32);

int l_fletcher8_final(lua_State* L){
  struct fletcher8_hash* a = (struct fletcher8_hash*)lua_touserdata(L, 1);
  uint8_t u = fletcher8_final(a);
  char digest[8];
  sprintf(digest,"%02x",u);
  lua_pushstring(L, digest);
  return 1;
}

int l_fletcher16_final(lua_State* L){
  struct fletcher16_hash* a = (struct fletcher16_hash*)lua_touserdata(L, 1);
  uint16_t u = fletcher16_final(a);
  char digest[16];
  sprintf(digest,"%04x",u);
  lua_pushstring(L, digest);
  return 1;
}

int l_fletcher32_final(lua_State* L){
  struct fletcher32_hash* a = (struct fletcher32_hash*)lua_touserdata(L, 1);
  uint32_t u = fletcher32_final(a);
  char digest[32];
  sprintf(digest,"%08x",u);
  lua_pushstring(L, digest);
  return 1;
}

int l_fletcher32(lua_State* L){
  if(lua_gettop(L) == 0) return l_fletcher32_init(L);
  size_t len = 0;
  uint8_t* a = (uint8_t*)luaL_checklstring(L, 1, &len);
  
  char digest[32];

  uint32_t u = fletcher32(a, len);
  sprintf(digest,"%08x",u);
  lua_pushstring(L, digest);

  return 1;
}

int l_fletcher16(lua_State* L){
  if(lua_gettop(L) == 0) return l_fletcher16_init(L);
  size_t len = 0;
  uint8_t* a = (uint8_t*)luaL_checklstring(L, 1, &len);
  
  char digest[16];

  uint16_t u = fletcher16(a, len);
  sprintf(digest,"%04x",u);
  lua_pushstring(L, digest);

  return 1;
}

int l_fletcher8(lua_State* L){
  if(lua_gettop(L) == 0) return l_fletcher8_init(L);
  size_t len = 0;
  uint8_t* a = (uint8_t*)luaL_checklstring(L, 1, &len);
  
  char digest[8];

  uint8_t u = fletcher8(a, len);
  sprintf(digest,"%02x",u);
  lua_pushstring(L, digest);

  return 1;
}
