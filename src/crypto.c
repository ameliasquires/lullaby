#include "crypto.h"

uint8_t rotl8(uint8_t y, uint8_t offset){
    return ( y << offset ) | ( y >> (8 - offset));
}

uint16_t rotl16(uint16_t y, uint16_t offset){
    return ( y << offset ) | ( y >> (16 - offset));
}

unsigned rotl32(unsigned y, unsigned offset){
    return ( y << offset ) | ( y >> (32 - offset));
}

unsigned rotr32(unsigned x, unsigned n) {
    return (x >> n % 32) | (x << (32-n) % 32);
}

uint64_t rotl64(uint64_t y, uint64_t offset){
    return ( y << offset ) | ( y >> (64 - offset));
}

uint64_t rotr64(uint64_t x, uint64_t n) {
    return (x >> n) | (x << (64-n));
}