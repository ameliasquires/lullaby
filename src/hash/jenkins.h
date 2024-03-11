#include "../lua.h"
#include <stdint.h>

uint32_t jenkins_oaat(uint8_t* in, size_t len);
int l_oaat(lua_State*);
