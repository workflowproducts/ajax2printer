#include "a2p_config.h"

char PORT[256] = "5000";
long DEBUG_LEVEL = 5;
char *str_tls_crt = NULL;
char *str_tls_key = NULL;
bool bol_tls = false;
char *str_path_lp = NULL;
char *str_global_config_file = NULL;

#define SMATCH(s, n) strcmp(str_section, s) == 0 && strcmp(str_name, n) == 0
static int handler(void *str_user, const char *str_section, const char *str_name, const char *str_value) {
	if (str_user != NULL) {
	} // get rid of unused variable warning

	if (SMATCH("", "port")) {
		memcpy(PORT, str_value, strlen(str_value));

	} else if (SMATCH("", "log_level")) {
		DEBUG_LEVEL = strtol(str_value, NULL, 10);

	} else if (SMATCH("", "tls_crt")) {
		SFREE(str_tls_crt);
		str_tls_crt = strdup(str_value);
		check(1, str_tls_crt != NULL, "strdup failed");

	} else if (SMATCH("", "tls_key")) {
		SFREE(str_tls_key);
		str_tls_key = strdup(str_value);
		check(1, str_tls_key != NULL, "strdup failed");

	} else if (SMATCH("", "lp_path")) {
		SFREE(str_path_lp);
		str_path_lp = strdup(str_value);
		check(1, str_path_lp != NULL, "strdup failed");

	} else {
	}
	return 0;
error:
	return 1;
}
bool parse_options(int argc, char **argv) {
	str_global_config_file = strdup(PREFIX "/etc/ajax2printer/ajax2printer.conf");

	// clang-format off
	static struct option longopts[22] = {
		{	"help",			no_argument,		NULL, 'h'	},
		{	"version",		no_argument,		NULL, 'v'	},
		{	"config-file",	required_argument,	NULL, 'c'	},
		{	"port",			required_argument,	NULL, 'p'	},
		{	"log-level",	required_argument,	NULL, 'l'	},
		{	"tls-crt",		required_argument,	NULL, 'j'	},
		{	"tls-key",		required_argument,	NULL, 'k'	},
		{	"lp-path",		required_argument,	NULL, 'a'	},
		{	NULL,			0,					NULL,  0	}
	};
	// clang-format on

	int ch = 0;
	while ((ch = getopt_long(argc, argv, "hvc:p:l:j:k:a:", longopts, NULL)) != -1) {
		if (ch == '?') {
			// getopt_long prints an error in this case
			goto error;

		} else if (ch == 'h') {
			usage();
			goto error;
		} else if (ch == 'v') {
			printf("ajax2printer %s\n", VERSION);
			goto error;
		} else if (ch == 'c') {
			SFREE(str_global_config_file);
			str_global_config_file = strdup(optarg);
			check(1, str_global_config_file != NULL, "strdup failed");
		} else if (ch == 0) {
			fprintf(stderr, "no options\n");
			goto error;
		}
	}

	char *str_config_empty = "";
	ini_parse(str_global_config_file, handler, &str_config_empty);

	optind = 0;
	while ((ch = getopt_long(argc, argv, "hvc:p:l:j:k:a:", longopts, NULL)) != -1) {
		if (ch == '?') {
			// getopt_long prints an error in this case
			goto error;

		} else if (ch == 'h') {
		} else if (ch == 'v') {
		} else if (ch == 'c') {
		} else if (ch == 'p') {
			memcpy(PORT, optarg, strlen(optarg));

		} else if (ch == 'l') {
			DEBUG_LEVEL = strtol(optarg, NULL, 10);

		} else if (ch == 'j') {
			SFREE(str_tls_crt);
			str_tls_crt = strdup(optarg);
			check(1, str_tls_crt != NULL, "strdup failed");

		} else if (ch == 'k') {
			SFREE(str_tls_key);
			str_tls_key = strdup(optarg);
			check(1, str_tls_key != NULL, "strdup failed");

		} else if (ch == 'a') {
			SFREE(str_path_lp);
			str_path_lp = strdup(optarg);
			check(1, str_path_lp != NULL, "strdup failed");

		} else if (ch == 0) {
			fprintf(stderr, "no options");
			goto error;
		} else {
			usage();
			goto error;
		}
	}

	// Check if we have the tls cert/key
	if (str_tls_crt != NULL && str_tls_key != NULL) {
		bol_tls = true;

	// We don't, so just listen on http
	} else {
		bol_tls = false;
	}

	return true;
error:
	return false;
}

void usage() {
	printf("Usage: ajax2printer\012");
	printf("\t[-h               | --help]\012");
	printf("\t[-v               | --version]\012");
	printf("\t[-c <config-file> | --config-file=<config-file>]\012");
	printf("\t[-p <port>        | --port=<port>]\012");
	printf("\t[-j <tls-cert>    | --tls-cert=<tls-cert>]\012");
	printf("\t[-k <tls-key>     | --tls-key=<tls-key>]\012");
	printf("\t[-l <log-level>   | --log-level=<log-level>]\012");
	printf("\012");
	printf("For more information, run `man ajax2printer`\012");
}
