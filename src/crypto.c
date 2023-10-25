#include "crypto.h"

unsigned i_lr(unsigned y, unsigned offset){
    return ( y << offset ) | ( y >> (32 - offset));
}

unsigned i_rr(unsigned x, unsigned n) {
    return (x >> n % 32) | (x << (32-n) % 32);
}
