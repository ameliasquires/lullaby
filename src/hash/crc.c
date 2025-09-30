#include "../crypto.h"
#include <stdio.h>
#include <stdint.h>
#include <stdio.h>
#include <stdint.h>

int crc32_free_l(lua_State* L){return 0;}
int crc16_free_l(lua_State* L){return 0;}
int crc8_free_l(lua_State* L){return 0;}

struct crc32_hash crc32_init(){
  return (struct crc32_hash){.crc = 0xFFFFFFFF};
}

void crc32_update(uint8_t* data, size_t len, struct crc32_hash* hash){
  for(int i = 0; i < len; i++){
    uint32_t extract = data[i];
    for(int z = 0; z < 8; z++){
      uint32_t b = (extract^hash->crc)&1;
      hash->crc>>=1;
      if(b) hash->crc^=0xEDB88320;
      extract>>=1;
    }
  }
}

uint32_t crc32_final(struct crc32_hash* hash){
  return -(hash->crc+1);
}

uint32_t crc32(uint8_t* data, size_t len){
  struct crc32_hash a = crc32_init();
  crc32_update(data, len, &a);
  return crc32_final(&a);
}

struct crc16_hash crc16_init(){
  return (struct crc16_hash){.crc = 0x0};
}

void crc16_update(uint8_t *aa, size_t len, struct crc16_hash *hash){
  for(int i = 0; i != len; i++){
    uint8_t a = aa[i];
    hash->crc ^= a;
    for (int z = 0; z < 8; z++){
      if (hash->crc & 1) hash->crc = (hash->crc >> 1) ^ 0xA001;
      else hash->crc = (hash->crc >> 1);
    }
  }
}

uint16_t crc16_final(struct crc16_hash *hash){
  return hash->crc;
}

uint16_t crc16(uint8_t *aa, size_t len){
  struct crc16_hash a = crc16_init();
  crc16_update(aa, len, &a);
  return crc16_final(&a);
}

struct crc8_hash crc8_init(){
  return (struct crc8_hash){.crc = 0x00};
}

void crc8_update(uint8_t *aa, size_t len, struct crc8_hash *hash){
  for(int i = 0; i != len; i++){
    uint8_t a = aa[i];

    for (int z = 0; z < 8; z++){
      uint8_t b = (hash->crc ^ a) & 1;
      hash->crc >>= 1;
      if(b) hash->crc ^= 0x8c;
      a >>=1;
    }
  }
}

uint8_t crc8_final(struct crc8_hash *hash){
  return hash->crc;
}

uint8_t crc8(uint8_t *aa, size_t len){
  struct crc8_hash a = crc8_init();
  crc8_update(aa, len, &a);
  return crc8_final(&a);
}

common_hash_clone(crc32);
common_hash_clone(crc16);
common_hash_clone(crc8);

common_hash_init_update(crc32);
common_hash_init_update(crc16);
common_hash_init_update(crc8);

int l_crc8_final(lua_State* L){
  struct crc8_hash* a = (struct crc8_hash*)lua_touserdata(L, 1);
  uint32_t u = crc8_final(a);
  char digest[8];
  sprintf(digest,"%x",u);
  lua_pushstring(L, digest);
  return 1;
}

int l_crc16_final(lua_State* L){
  struct crc16_hash* a = (struct crc16_hash*)lua_touserdata(L, 1);
  uint32_t u = crc16_final(a);
  char digest[16];
  sprintf(digest,"%04x",u);
  lua_pushstring(L, digest);
  return 1;
}

int l_crc32_final(lua_State* L){
  struct crc32_hash* a = (struct crc32_hash*)lua_touserdata(L, 1);
  uint32_t u = crc32_final(a);
  char digest[32];
  sprintf(digest,"%08x",u);
  lua_pushstring(L, digest);
  return 1;
}

int l_crc8(lua_State* L){
  if(lua_gettop(L) == 0) return l_crc8_init(L);
  size_t len = 0;
  uint8_t* a = (uint8_t*)luaL_checklstring(L, 1, &len);

  char digest[8];

  uint8_t u = crc8(a, len);
  sprintf(digest,"%x",u);
  lua_pushstring(L, digest);

  return 1;
}

int l_crc16(lua_State* L){
  if(lua_gettop(L) == 0) return l_crc16_init(L);
  size_t len = 0;
  uint8_t* a = (uint8_t*)luaL_checklstring(L, 1, &len);

  char digest[16];

  uint16_t u = crc16(a, len);
  sprintf(digest,"%x",u);
  lua_pushstring(L, digest);

  return 1;
}

int l_crc32(lua_State* L){
  if(lua_gettop(L) == 0) return l_crc32_init(L);
  size_t len = 0;
  uint8_t* a = (uint8_t*)luaL_checklstring(L, 1, &len);

  char digest[32];

  uint32_t u = crc32(a, len);
  sprintf(digest,"%x",u);
  lua_pushstring(L, digest);

  return 1;
}
