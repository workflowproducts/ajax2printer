#pragma once

#include <ctype.h>
#include <errno.h>
#include <regex.h>
#include <stdarg.h>
#include <stdio.h>
#include <string.h>

#include "a2p_salloc.h"
#include "a2p_sunlogf.h"

char *uri_to_cstr(char *loop_ptr, size_t inputstring_len);
char *getpar(char *query, char *key);

char *c_cat(int args, ...);
char *c_append(int args, ...);

#define cat_cstr(...) c_cat(VA_NUM_ARGS(__VA_ARGS__), ##__VA_ARGS__)
#define cat_append(...) c_append(VA_NUM_ARGS(__VA_ARGS__), ##__VA_ARGS__)
// cat_append is just like cat_cstr except the first argument is free()d
