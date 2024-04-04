#include "../lua.h"
#include <stdint.h>

struct loselose_hash {
  uint64_t hash;
};

struct loselose_hash loselose_init();
void loselose_update(uint8_t*, size_t, struct loselose_hash*);
uint64_t loselose_final(struct loselose_hash*);

uint64_t loselose(uint8_t* in, size_t len);

int l_loselose(lua_State*);
int l_loselose_init(lua_State*);
int l_loselose_update(lua_State*);
int l_loselose_final(lua_State*);
