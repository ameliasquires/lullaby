#include "i_util.h"
#include <stdio.h>
#include <stdlib.h>

void p_fatal(const char* m){
  fprintf(stderr, "%s[ fatal ] %s %s\n",color_red, m, color_reset);
  exit(EXIT_FAILURE);
}

void p_error(const char* m){
  fprintf(stderr, "%s[ error ]%s %s\n",color_red, color_reset, m);
}
