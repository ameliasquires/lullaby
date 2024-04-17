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
#define lesser(A,B) ((A)>(B)?(B):(A))
#define inter(V,I) (I * ceil((double)V / I))
#define time_start(name)\
    clock_t _begin##name = clock();
#define time_end(desc, name)\
    clock_t _end##name = clock();\
    printf("%s took %f\n",desc, (double)(_end##name - _begin##name) / CLOCKS_PER_SEC);

int gen_parse(char*,int, parray_t**);

#define p_fatal(M) _p_fatal(M, __LINE__, __FILE__, __func__ );
void _p_fatal(const char*, int, const char*, const char*);
void p_error(const char*);
char* strnstr(const char*, const char*, size_t);

#endif //__UTIL_H