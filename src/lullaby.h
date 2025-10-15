#ifndef LULLABY_H
#define LULLABY_H
#pragma once
#include "config.h"

extern int _print_errors;

static struct config lullaby_config[] = {
  {.name = "print_errors", .type = c_int, .value = {.c_int = &_print_errors}},
  {.type = c_none}
};

#endif
