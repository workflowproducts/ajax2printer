#pragma once

#include <stdarg.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#ifdef __linux
// linux waitpid
#include <sys/types.h>
#include <sys/wait.h>
#endif

#include "a2p_salloc.h"
#include "a2p_string.h"
#include "a2p_sunlogf.h"

#define sunny_exec(csock, ...) s_exec(csock, VA_NUM_ARGS(__VA_ARGS__), ##__VA_ARGS__);
int s_exec(int csock, size_t args, ...);
extern char **environ;
int clearenv(void);
