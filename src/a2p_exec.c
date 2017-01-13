#include "a2p_exec.h"

// safe system execute function
int s_exec(int csock, size_t args, ...) {
	// to use: int int_test = sunny_exec( "/usr/local/pgsql/bin/pg_ctl", "-D", "/opt/3comets/data", "stop" );
	//         int int_test = sunny_exec( "/bin/echo", "test1" );
	//         printf( "%i: ", int_test );

	// #### secure execute of programs ###
	pid_t pid;
	int status;
	pid_t ret;

	// set errno to "Success" in case caller functions have errors in them
	errno = 0;

	// fork causes the existing program to split into two identical processes.
	pid = fork();
	if (pid == -1) {
		sunlogf(3, "Fork error.");
	} else if (pid != 0) {
		while ((ret = waitpid(pid, &status, 0)) == -1) {
			if (errno != EINTR) {
				sunlogf(3, "Error waiting for child process.");
				break;
			}
		}
		// keep this code in case you have problems
		if ((ret != -1) && errno != 0 && (!WIFEXITED(status) || !WEXITSTATUS(status)) && errno != 10) {
			sunlogf(3, "Child has unexpected status. errno: %d", errno);
		}
		return 1;
	} else {
		close(csock);
		// the first item is our program executable
		// assemble a nice array of all our args with a null element at the end
		va_list ap;
		va_list bp;
		size_t i;
		size_t len = 0;
		// arr_args[0] is the program path
		// arr_args[1] is the program name
		// arr_args[2-args] are the arguments to the program
		size_t lengths[args + 1];
		size_t prog_len; // put args[0] len here
		size_t prog_name_len = 0;

		// allocate prog and an array large enough for everything
		va_start(ap, args);
		va_copy(bp, ap); // powerpc can't do two va_starts. use va_copy instead.

		// fill prog
		char *prog = va_arg(ap, char *);
		prog_len = strlen(prog) + 1;

		// get program name length from path
		for (i = 0; i < prog_len; i = i + 1) {
			if (strncmp("/", prog + i, 1) == 0) {
				prog_name_len = prog_len - (i + 1);
			}
		}
		if (prog_name_len == 0) {
			sunlogf(3, "First arg must be a complete path, e.g. /bin/touch. You tried '%s'.", prog);
			return 1;
		}
		lengths[0] = prog_name_len;

		// get the rest of the lengths
		for (i = 1; i < args; i = i + 1) {
			len = strlen(va_arg(ap, char *)) + 1;
			lengths[i] = len;
		}
		va_end(ap);

		// set up arr_args
		char *arr_args[args + 1];
		for (i = 0; i < args; i = i + 1) {
			arr_args[i] = (char *)salloc(lengths[i]);
		}

		// get prog name from path
		char *prog_name = (char *)((prog + prog_len) - prog_name_len);

		len = strlen(prog_name) + 1;
		memcpy(arr_args[0], prog_name, len - 1);
		arr_args[0][len - 1] = '\0';

		// we don't need the path again so run va_arg once to pass it.
		va_arg(bp, char *);
		for (i = 1; i < args; i = i + 1) {
			memcpy(arr_args[i], va_arg(bp, char *), lengths[i] - 1);
			arr_args[i][lengths[i] - 1] = '\0';
			// sunlogf(3, "arr_args[i]: %s, %i", arr_args[i], i);
		}
		va_end(bp);

		// add a null element to the array
		arr_args[args] = (char *)salloc(1);
		arr_args[args] = NULL; // WRONG: arr_args[args] = '\0'; ALSO WRONG: arr_args[args][0] = '\0';

		char *pathbuf;
		size_t n;

		if (clearenv() != 0) {
			sunlogf(3, "Command clearenv not available. Exiting.");
		}

		n = confstr(_CS_PATH, NULL, 0);
		if (n == 0) {
			sunlogf(3, "Command confstr not available. Exiting.");
		}

		if ((pathbuf = salloc(n)) == NULL) {
			sunlogf(3, "Command salloc errored. Exiting.");
		}

		if (confstr(_CS_PATH, pathbuf, n) == 0) {
			sunlogf(3, "Command confstr errored. Exiting.");
		}

		if (setenv("PATH", pathbuf, 1) == -1) {
			sunlogf(3, "Command setenv PATH errored. Exiting.");
		}

		free(pathbuf); // void, no test

		if (setenv("IFS", " \t\n", 1) == -1) {
			sunlogf(3, "Command setenv IFS errored. Exiting.");
		}

		// Initialize env as a sanitized copy of environment
		// sunlogf( 5, "%s %s %s %s %s", prog, arr_args[0], arr_args[1], arr_args[2], arr_args[3]);
		if (execv(prog, arr_args) == -1) {
			sunlogf(3, "Error executing '%s'\n", prog);
			for (i = 0; i < args; i = i + 1) {
				free(arr_args[i]);
			}
			free(arr_args[args]);
			_exit(127);
		}
		sunlogf(3, "Error.");
		for (i = 0; i < args; i = i + 1) {
			free(arr_args[i]);
		}
		free(arr_args[args]);
		// This process terminates. The calling (parent) process will now continue.
		//   we only return a value if there was an error. (errno will be set)
		// the following line won't execute unless there is an error.
		sunlogf(5, "Error in sunny_exec: '%s'\n", prog);
	}

	return -1;
}

int clearenv(void) {
	static char *namebuf = NULL;
	static size_t lastlen = 0;

	while (environ != NULL && environ[0] != NULL) {
		size_t len = strcspn(environ[0], "=");
		if (len == 0) {
			/* Handle empty variable name (corrupted environ[]) */
		}
		if (len > lastlen) {
			namebuf = realloc(namebuf, len + 1);
			if (namebuf == NULL) {
				/* Handle error */
			}
			lastlen = len;
		}
		memcpy(namebuf, environ[0], len);
		namebuf[len] = '\0';
		if (unsetenv(namebuf) == -1) {
			/* Handle error */
		}
	}
	return 0;
}
