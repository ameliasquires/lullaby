#include "../lua.h"
#include <stdint.h>

static const uint32_t c1 = 0xcc9e2d51;
static const uint32_t c2 = 0x1b873593;

uint32_t rot32(uint32_t val, int shift);
uint32_t fmix(uint32_t h);
uint32_t mur(uint32_t a, uint32_t h);
uint32_t hash32len0to4(uint8_t* in, size_t len);
uint32_t UNALIGNED_LOAD32(uint8_t *p);
uint32_t hash32len5to12(uint8_t* in, size_t len);
uint32_t hash32len13to24(uint8_t* in, size_t len);
uint32_t cityhash32(uint8_t* in, size_t len);

//64 version

static const uint64_t k0 = 0xc3a5c85c97cb3127ULL;
static const uint64_t k1 = 0xb492b66fbe98f273ULL;
static const uint64_t k2 = 0x9ae16a3b2f90404fULL;

uint64_t UNALIGNED_LOAD64(uint8_t *p);
uint64_t rot64(uint64_t val, int shift);
uint64_t hashlen16(uint64_t u, uint64_t v, uint64_t mul);
uint64_t shiftmix(uint64_t val);
uint64_t hashlen0to16(uint8_t* in, size_t len);
uint64_t hashlen17to32(uint8_t* in, size_t len);
uint64_t hashlen33to64(uint8_t* in, size_t len);
void WeakHashLen32WithSeeds(uint64_t w, uint64_t x, uint64_t y, uint64_t z, uint64_t a, uint64_t b, uint64_t*p1, int64_t*p2);
void pWeakHashLen32WithSeeds(uint8_t* s, uint64_t a, uint64_t b, uint64_t* p1, int64_t* p2);
uint64_t hash128to64(uint64_t f, uint64_t s);
uint64_t HashLen16_2(uint64_t u, uint64_t v);
uint64_t cityhash64(uint8_t* in, size_t len);
void citymurmur(uint8_t* in, size_t len, uint64_t f, uint64_t s, uint64_t* o1, uint64_t* o2);
void cityhash128withseed(uint8_t* in, size_t len, uint64_t f, uint64_t s, uint64_t* o1, uint64_t* o2);
void cityhash128(uint8_t* in, size_t len, uint64_t* f, uint64_t* s);

int l_cityhash32(lua_State*);
int l_cityhash64(lua_State*);
int l_cityhash128(lua_State*);
