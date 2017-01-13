#include "a2p_ini.h"
#include "a2p_string.h"
#include "a2p_sunlogf.h"
#include <getopt.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>

// ###############################################
// configuration variables

extern char PORT[256];
extern long DEBUG_LEVEL;
extern char *str_tls_crt;
extern char *str_tls_key;
extern bool bol_tls;
extern char *str_path_lp;

#define BACKLOG 128
#define BUF_LEN 1024

void usage();
bool parse_options(int argc, char **argv);
