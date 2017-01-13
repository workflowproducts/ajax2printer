#include "a2p_sunlogf.h"

void sunlogf(int int_error_level, const char *str_format, ...) {
	// We get valgrind errors with vsyslog
	// use sunlogf only for runtime errors

	// errors that require action from sysadmin:
	// 0 LOG_EMERG: system halt, e.g. failure accessing file, database, etc. system admin will be notified.
	// 1 LOG_ALERT resource error, e.g. too many connections. system admin will be notified.
	// 2 LOG_CRIT serious, uncommon, and unexpected event. may lead to more serious error.

	// errors that DO NOT require action from admin:
	// 3 LOG_ERR any failure not requiring immediate sysadmin action
	// 4 LOG_WARNING url and arguments.
	// 5 LOG_NOTICE summary of success. top level args and results.
	// 6 LOG_INFO announce each function name possibly with args and result.
	// 7 LOG_DEBUG important variables internal to important functions.
	// syslog( 5, "err_lvl: %i", int_error_level);
	// syslog( 5, "0LOG_EMERG: %i", LOG_EMERG);
	// syslog( 5, "1LOG_ALERT: %i", LOG_ALERT);
	// syslog( 5, "2LOG_CRIT: %i", LOG_CRIT);
	// syslog( 5, "3LOG_ERR: %i", LOG_ERR);
	// syslog( 5, "4LOG_WARNING: %i", LOG_WARNING);
	// syslog( 5, "5LOG_NOTICE: %i", LOG_NOTICE);
	// syslog( 5, "6LOG_INFO: %i", LOG_INFO);
	// syslog( 5, "7LOG_DEBUG: %i", LOG_DEBUG);

	// ########################################################
	// ################### COMMAND TO USE #####################
	// #### syslog -w -F '$Message' -k Sender envelope_mac ####
	// ########################################################

	// open log file:
	if (DEBUG_LEVEL >= int_error_level) {
		va_list va_arg;

		// put last fixed param here (str_format) to get the ... params
		va_start(va_arg, str_format); // accepts 255 chars.

		// str_pid format
		char str_pid[8] = "0000000"; // was 4 but that seemed small- justin
		sprintf(str_pid, "%d ", getpid());

		// all strings so no need to free
		char *log_level =
			int_error_level == 0
				? "EMERG  "
				: int_error_level == 1
					  ? "ALERT  "
					  : int_error_level == 2
							? "CRIT   "
							: int_error_level == 3
								  ? "ERR    "
								  : int_error_level == 4
										? "WARN   "
										: int_error_level == 5 ? "NOTICE " : int_error_level == 6 ? "INFO   " : "DEBUG  ";
		char *str_new_format = cat_cstr(str_pid, log_level, str_format);
		if (int_error_level > 5) {
			vsyslog(5, str_new_format, va_arg);
		} else {
			vsyslog(int_error_level, str_new_format, va_arg);
		}
		free(str_new_format);
		va_end(va_arg);
	}
	// closelog();
}
