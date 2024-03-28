#include "../lua.h"
#include <stdint.h>

struct jenkins_oaat_hash {
  uint32_t hash;
};

uint32_t jenkins_oaat(uint8_t* in, size_t len);
struct jenkins_oaat_hash jenkins_oaat_init();
void jenkins_oaat_update(uint8_t*, size_t, struct jenkins_oaat_hash*);
uint32_t jenkins_oaat_final(struct jenkins_oaat_hash*);

int l_oaat(lua_State*);
int l_oaat_init(lua_State*);
int l_oaat_update(lua_State*);
int l_oaat_final(lua_State*);
