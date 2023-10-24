#include "crypto.h"

unsigned i_lr(unsigned y, unsigned offset){
    return ( y << offset ) | ( y >> (32 - offset));
}
