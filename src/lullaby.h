#ifndef LULLABY_H
#define LULLABY_H
#pragma once
#include "config.h"
#include "lua.h"

extern int _print_errors;

static struct config lullaby_config[] = {
#warning todo?
  {.name = "print_errors", .type = c_int, .value = {.c_int = &_print_errors}},
  {.type = c_none}
};

static const luaL_Reg lullaby_function_list [] = {
  {NULL,NULL}
};

#endif
