#ifndef __UTIL_H
#define __UTIL_H

#include "types/str.h"
#include "types/parray.h"

#define color_black "\e[30m"
#define color_red "\e[31m"
#define color_green "\e[32m"
#define color_yellow "\e[33m"
#define color_blue "\e[34m"
#define color_magenta "\e[35m"
#define color_cyan "\e[36m"
#define color_lgray "\e[37m"
#define color_gray "\e[90m"
#define color_lred "\e[91m"
#define color_lgreen "\e[92m"
#define color_lyellow "\e[93m"
#define color_lblue "\e[94m"
#define color_lmagenta "\e[95m"
#define color_lcyan "\e[96m"
#define color_white "\e[97m"
#define color_reset "\e[0m"

#define i_swap(A,B) double temp = A; A = B; B = temp;

int gen_parse(char*,int, parray_t**);
void p_fatal(const char*);
void p_error(const char*);
char* strnstr(const char*, const char*, size_t);

#endif //__UTIL_H