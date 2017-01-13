#pragma once

#include <stdarg.h>
#include <stdlib.h>
#include <string.h>
#include <syslog.h>
#include <unistd.h>

#include "a2p_config.h"
#include "a2p_string.h"

void sunlogf(int int_error_level, const char *str_format, ...);
