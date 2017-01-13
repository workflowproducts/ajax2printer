#include "a2p_request.h"

char *str_query(char *str_request) {
	char *ret = (char *)salloc(1);
	ret[0] = 0;
	char *form_data;
	char *form_data_ptr = NULL;
	// int   form_data_len;

	// find the form_data by request type
	if (strncmp(str_request, "GET ", 4) == 0) {
		form_data_ptr = str_uri_path(str_request);
		form_data = strstr(form_data_ptr, "?");
		if (form_data == 0) {
			sunlogf(7, "no form data;\n\n");
			free(form_data_ptr);
			return ret;
		}
		form_data = form_data + 1; // advance cursor past "?"
		form_data = cat_cstr(form_data);
		free(form_data_ptr);
	} else {
		// rewritten to work with safari, still doesn't work
		char *temp1 = strstr(str_request, "\r\n\r\n");
		char *temp2 = strstr(str_request, "\n\n");
		if (temp1 == 0 && temp2 == 0) {
			sunlogf(7, "no form data;\n\n");
			return ret;
		} else if (temp1 != 0 && temp2 == 0) {
			form_data = temp1 + 4;
		} else if (temp1 == 0 && temp2 != 0) {
			form_data = temp2 + 2;
		} else {
			if (temp1 < temp2) {
				form_data = temp1 + 4;
			} else {
				form_data = temp2 + 2;
			}
		}
		form_data = cat_cstr(form_data);
		/*
		form_data = strstr( str_request, "\r\n\r\n" );
		if ( form_data == 0 ) {
		  sunlogf( 7, "no form data;\n\n");
		  return ret;
		}
		form_data = form_data + 4; // advance cursor past "\r\n\r\n"
		*/
	}

	// return just the form_data
	free(ret);
	return form_data;
	/*
	form_data_len = strlen( form_data );
	free(ret);
	ret = (char*)salloc(form_data_len+1);
	memcpy( ret, form_data, form_data_len );
	ret[form_data_len] = 0;

	if (form_data_ptr != NULL) {
	  free(form_data_ptr);
	}
	return ret;
	*/
}

char *str_uri_path(char *str_request) {
	char *ret = (char *)salloc(1);
	ret[0] = 0;
	char *uri;
	char *uri_end;
	size_t uri_len;

	// if the request is not long enough to have a URI then abort
	if (strlen(str_request) < 5) {
		syslog(LOG_NOTICE, "request too short to parse;");
		return ret;
	}

	// find uri start character
	if (strncmp(str_request, "GET ", 4) == 0) {
		uri = str_request + 4;
	} else if (strncmp(str_request, "HEAD ", 5) == 0 || strncmp(str_request, "POST ", 5) == 0) {
		uri = str_request + 5;
	} else {
		syslog(LOG_NOTICE, "unknown request type;");
		return ret;
	}
	free(ret);

	// return just the Request-URI
	uri_end = strstr(uri, " ");
	uri_len = (size_t)(uri_end - uri);
	// sunlogf( 7, " uri_len: %i", uri_len );
	char *str_ret = (char *)salloc(uri_len + 1);
	memcpy(str_ret, uri, uri_len);
	str_ret[uri_len] = 0;
	return str_ret;
}
