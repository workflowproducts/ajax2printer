#include <arpa/inet.h>
#include <arpa/inet.h>
#include <errno.h>
#include <netdb.h>
#include <netdb.h>
#include <netinet/in.h>
#include <pthread.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include <fcntl.h>

#include <tls.h>

#include "a2p_config.h"
#include "a2p_exec.h"
#include "a2p_request.h"
#include "a2p_string.h"

#define SERROR(...) sunlogf(3, __VA_ARGS__); goto error;
#define SWARN_NORESPONSE(...) sunlogf(5, __VA_ARGS__);
#define SERROR_NORESPONSE(...) sunlogf(3, __VA_ARGS__);
#define SFINISH_ERROR(...) sunlogf(3, __VA_ARGS__); goto error;

#define SERROR_LIBTLS_NOCONTEXT(M, ...) SERROR("LIBTLS: " M, ##__VA_ARGS__);
#define SERROR_NORESPONSE_LIBTLS_NOCONTEXT(M, ...) SERROR_NORESPONSE("LIBTLS: " M, ##__VA_ARGS__);
#define SERROR_LIBTLS_CONTEXT(C, M, ...) SERROR("LIBTLS: %s " M, tls_error(C), ##__VA_ARGS__);
#define SERROR_NORESPONSE_LIBTLS_CONTEXT(C, M, ...) SERROR_NORESPONSE("LIBTLS: %s " M, tls_error(C), ##__VA_ARGS__);

#define SFINISH_LIBTLS_NOCONTEXT(M, ...) SFINISH_ERROR("LIBTLS: " M "\n", ##__VA_ARGS__);
//#define SFINISH_LIBTLS_CONTEXT are you sure?
#define SFINISH_LIBTLS_CONTEXT(C, M, ...) SFINISH_ERROR("LIBTLS: %s " M "\n", tls_error(C), ##__VA_ARGS__);

// Global declarations
int term = 0;
void *handle(void *csock);

struct tls_config *tls_server_config;
struct tls *tls_server_context;
struct client_t {
	struct tls *tls_client_context;
	int csock;
};

// Main entry point
int main(int argc, char **argv) {
	if (parse_options(argc, argv) == false) {
		return 1;
	}

	// Declarations
	int sock;
	pthread_t thread;
	struct addrinfo hints, *res;
	int reuseaddr = 1; // True

	// Get the address info for later
	memset(&hints, 0, sizeof(hints));
	hints.ai_family = AF_INET;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_flags = AI_PASSIVE;
	sunlogf(9, "getaddrinfo: %s", PORT);
	check(2, getaddrinfo(NULL, PORT, &hints, &res) == 0, "Error Getting Address Info");

	sunlogf(9, "socket");
	// Create the socket so that we can bind
	sock = socket(res->ai_family, res->ai_socktype, res->ai_protocol);
	check(2, sock != -1, "Error Creating Socket");

	sunlogf(9, "setsockopt");
	// Enable the socket to reuse the address so that we can loop later
	check(2, setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, &reuseaddr, sizeof(int)) != -1, "Error Setting Socket Options");

	sunlogf(9, "bind");
	// Bind to the address so we can listen
	check(2, bind(sock, res->ai_addr, res->ai_addrlen) != -1, "Error binding");

	sunlogf(9, "listen");
	// Listen so that we can recieve data
	check(2, listen(sock, BACKLOG) != -1, "Error listening");
	freeaddrinfo(res);

	if (bol_tls) {
		tls_init();

		// Initialize a configuration context
		tls_server_config = tls_config_new();
		if (tls_server_config == NULL) {
			SERROR_LIBTLS_NOCONTEXT("tls_config_new() failed");
		}

		// Set the certificate file in the configuration context
		if (tls_config_set_cert_file(tls_server_config, str_tls_crt) != 0) {
			SERROR_LIBTLS_NOCONTEXT("tls_config_set_cert_file() failed");
		}

		// Set the key file in the configuration context
		if (tls_config_set_key_file(tls_server_config, str_tls_key) != 0) {
			SERROR_LIBTLS_NOCONTEXT("tls_config_set_key_file() failed");
		}

		// Set the allowed ciphers in the configuration context
		if (tls_config_set_ciphers(tls_server_config, "compat") != 0) {
			SERROR_LIBTLS_NOCONTEXT("tls_config_set_ciphers() failed");
		}

		// Create a server context
		tls_server_context = tls_server();
		if (tls_server_context == NULL) {
			SERROR_LIBTLS_NOCONTEXT("tls_server() failed");
		}

		// Attach the configuration context to the _server context
		if (tls_configure(tls_server_context, tls_server_config) != 0) {
			SWARN_NORESPONSE("str_global_tls_crt: %s", str_tls_crt);
			SWARN_NORESPONSE("str_global_tls_key: %s", str_tls_key);
			SERROR_LIBTLS_CONTEXT(tls_server_context, "tls_configure() failed");
		}
	}

	// Main loop
	while (term != 1) {
		socklen_t size = sizeof(struct sockaddr_in);
		struct sockaddr_in their_addr;

		// Accept connection (Blocking)
		int newsock = accept(sock, (struct sockaddr *)&their_addr, &size);
		if (newsock < 0) {
			perror("accept");
			exit(1);
		} else {
			printf("Got a connection from %s on port %d\n", inet_ntoa(their_addr.sin_addr), htons(their_addr.sin_port));

			struct client_t *client = salloc(sizeof(struct client_t));
			client->csock = newsock;
			if (bol_tls) {
				// The tls accept function takes the socket you send and makes a tls I/O
				// context out of it
				int int_status = tls_accept_socket(tls_server_context, &client->tls_client_context, newsock);
				if (int_status != 0) {
					SERROR_LIBTLS_CONTEXT(tls_server_context, "tls_accept_fds() failed");
				}
			}

			// Spawn new thread to handle request asynchronously
			if (pthread_create(&thread, NULL, handle, client) != 0) {
				sunlogf(1, "Failed to create thread");
			}
			if (pthread_detach(thread) != 0) {
				sunlogf(1, "Failed to create thread");
			}
		}
	}

	return 0;
error:
	return 1;
}

// handle function takes a client_t struct
void *handle(void *_client) {
	struct client_t *client = _client;
	char *buf = (char *)salloc(BUF_LEN + 1);
	char *request = (char *)salloc(1);
	request[0] = 0;
	char *response;
	ssize_t request_len = BUF_LEN;
	ssize_t read_total = 0;

	memset(buf, 0, BUF_LEN + 1);

	// keep reading until the end of the request
	while (strstr(request, "Content-Length: ") == NULL) {
		sunlogf(9, "request: %s", request);
		memset(buf, 0, BUF_LEN + 1);
		if (bol_tls) {
			request_len = tls_read(client->tls_client_context, buf, BUF_LEN);
		} else {
			request_len = read(client->csock, buf, BUF_LEN);
		}
		request = cat_append(request, buf);
		read_total = read_total + request_len;
		if (request_len == 0) {
			free(request);
			pthread_exit(NULL);
		}
		sunlogf(9, "request: %s", request);
	}

	char *ptr_content_length = strstr(request, "Content-Length: ") + strlen("Content-Length: ");
	size_t content_length = (size_t)strtol(ptr_content_length, NULL, 10);
	while (strstr(request, "\r\n\r\n") == NULL) {
		memset(buf, 0, BUF_LEN + 1);
		if (bol_tls) {
			request_len = tls_read(client->tls_client_context, buf, BUF_LEN);
		} else {
			request_len = read(client->csock, buf, BUF_LEN);
		}
		request = cat_append(request, buf);
		read_total = read_total + request_len;
	}

	char *ptr_query = strstr(request, "\r\n\r\n") + 4;
	content_length = content_length - strlen(ptr_query);
	while (content_length > 0) {
		memset(buf, 0, BUF_LEN + 1);
		if (bol_tls) {
			request_len = tls_read(client->tls_client_context, buf, BUF_LEN);
		} else {
			request_len = read(client->csock, buf, BUF_LEN);
		}
		request = cat_append(request, buf);
		read_total = read_total + request_len;
		content_length = content_length - (size_t)request_len;
	}

	// request_len = strlen(request);

	// Get various parameters
	char *query = str_query(request);
	sunlogf(9, "qry: %s", query);
	char *str_printer_name = getpar(query, "cups");
	char *str_msg = getpar(request, "text");
	sunlogf(9, "text='%s'", str_msg);
	char *str_file_name = salloc(20);

	// If we want to print to a named printer via cups
	if (strlen(str_printer_name) > 0) {
		// lp -d Zebra_R README

		// Get the peer port so we can name a unique file
		struct sockaddr_in addr;
		socklen_t socklen = sizeof(addr);
		check(2, getpeername(client->csock, (struct sockaddr *)&addr, &socklen) >= 0, "Failed to get peer address\n");
		int int_peer_port = htons(addr.sin_port);

		// Create file with content
		sprintf(str_file_name, "a2p_%d", int_peer_port);
		umask(002);
		FILE *fp = fopen(str_file_name, "wx");
		if (fp == NULL) {
			sunlogf(3, "Error: %s, Could not open file with name: %s", strerror(errno), str_file_name);
		}
		fwrite(str_msg, 1, strlen(str_msg), fp);
		if (ferror(fp)) {
			sunlogf(3, "Could not write to file with name: %s", str_file_name);
		}
		if (fclose(fp) != 0) {
			sunlogf(3, "Could not close file with name: %s", str_file_name);
		}

		// Send print command
		int int_status; // 1 = error, -1 means we're the parent
		int_status = sunny_exec(client->csock, "/usr/bin/lp", "-d", str_printer_name, str_file_name);
		sunlogf(7, "Success! LP command sent to printer %s.\n", str_printer_name);

		// Remove the file we created
		int_status = sunny_exec(client->csock, "/bin/rm", str_file_name);
		sunlogf(7, "Success! RM command sent to delete %s.\n", str_file_name);

		// Tell the browser we had success!
		response = "HTTP/1.1 200 OK\r\nAccess-Control-Allow-Origin: *\r\nContent-Type: application/json; "
				   "charset=UTF-8\r\n\r\n{\"stat\":\"true\"}";
	} else {
		// Tell the browser we failed!
		response = "HTTP/1.1 200 OK\r\nAccess-Control-Allow-Origin: *\r\nContent-Type: application/json; charset=UTF-8\r\n\r\n"
				   "{\"stat\":\"false\",\"dat\":\"The 'cups' parameter was empty. Please pass a named printer.\"}";
	}

	if (bol_tls) {
		request_len = tls_write(client->tls_client_context, response, strlen(response));
		if (tls_close(client->tls_client_context) != 0) {
			SERROR_NORESPONSE_LIBTLS_CONTEXT(client->tls_client_context, "tls_close() failed");
		}
		tls_free(client->tls_client_context);
	} else {
		write(client->csock, response, strlen(response));
	}

	check(2, close(client->csock) != -1, "Fail close(csock)");

	// Free everything involved with the request
	free(request);
	free(query);
	free(str_printer_name);
	free(str_ip);
	free(str_msg);
	free(str_file_name);

	free(buf);

	pthread_exit(NULL);
error:
	sunlogf(6, "handle failed");

	free(request);
	free(query);
	free(str_printer_name);
	free(str_ip);

	free(buf);

	pthread_exit(NULL);
}