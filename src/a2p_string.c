#include "a2p_string.h"

// case uri with percent encoded hex to utf-8
char *uri_to_cstr(char *loop_ptr, size_t inputstring_len) {
	char *result_text;
	char *result_ptr;
	size_t result_len;
	size_t chunk_len;
	char *x;

	// Dangerous loops ahead. We could go infinite if we aren't
	//   careful. So lets check for interrupts.
	result_text = (char *)salloc(1);
	result_len = 0;
	char buffer[3];
	buffer[2] = 0;

	while (inputstring_len > 0) {
		chunk_len = 1;

		// sunlogf( 7, "loop_ptr: %s, chunk_len: %i, inputlen: %i", loop_ptr, chunk_len, inputstring_len );
		// sunlogf( 7, "result_ptr: %s, result_len: %i ", result_ptr, result_len );

		// check for % characters
		//   if found, decode as percent encoded hex
		if (strncmp(loop_ptr, "%", 1) == 0) {
			x = loop_ptr + 1;
			// sunlogf( 7, "percent detected");

			// check if two digits  00..7F  //SELECT net.uri_to_text(E':%20:') => space character;
			if ((strncmp(x, "0", 1) >= 0 && strncmp(x, "7", 1) <= 0) &&
				((strncmp(x + 1, "0", 1) >= 0 && strncmp(x + 1, "9", 1) <= 0) ||
					(strncasecmp(x + 1, "a", 1) >= 0 && strncasecmp(x + 1, "f", 1) <= 0))) {
				// sunlogf( 7, "We have a one byte char. strtol:%ld;", strtol( x, 0, 16) );
				result_text = (char *)realloc(result_text, result_len + 1);
				memcpy(buffer, x, 2);
				result_text[result_len] = (char)strtol(buffer, 0, 16);
				result_len += 1;
				chunk_len = 3;

				// check if four digits C2..DF  //SELECT net.uri_to_text(E':%c4%b3'); => combined ij char
			} else if ((strncasecmp(x, "c", 1) == 0) &&
					   ((strncmp(x + 1, "2", 1) >= 0 && strncmp(x + 1, "9", 1) <= 0) ||
						   (strncasecmp(x + 1, "a", 1) >= 0 && strncasecmp(x + 1, "f", 1) <= 0))) {
				// sunlogf( 7, "We have a two byte char 'C' x:%s;", x);
				result_text = (char *)realloc(result_text, result_len + 2);
				memcpy(buffer, x, 2);
				result_text[result_len] = (char)strtol(buffer, 0, 16);
				memcpy(buffer, x + 3, 2);
				result_text[result_len + 1] = (char)strtol(buffer, 0, 16);
				result_len += 2;
				chunk_len = 6;

				// check if four digits C2..DF
			} else if ((strncasecmp(x, "d", 1) == 0) &&
					   ((strncmp(x + 1, "0", 1) >= 0 && strncmp(x + 1, "9", 1) <= 0) ||
						   (strncasecmp(x + 1, "a", 1) >= 0 && strncasecmp(x + 1, "f", 1) <= 0))) {
				// sunlogf( 7, "We have a two byte char 'D'");
				result_text = (char *)realloc(result_text, result_len + 2);
				memcpy(buffer, x, 2);
				result_text[result_len] = (char)strtol(buffer, 0, 16);
				memcpy(buffer, x + 3, 2);
				result_text[result_len + 1] = (char)strtol(buffer, 0, 16);
				result_len += 2;
				chunk_len = 6;

				// check if six digits E0, E1..EC, ED, EE..EF   //SELECT net.uri_to_text(E':%ef%b9%a0:'); light ampersand
			} else if ((strncasecmp(x, "e", 1) == 0) &&
					   ((strncmp(x + 1, "0", 1) >= 0 && strncmp(x + 1, "9", 1) <= 0) ||
						   (strncasecmp(x + 1, "a", 1) >= 0 && strncasecmp(x + 1, "f", 1) <= 0))) {
				// sunlogf( 7, "We have a three byte char.");
				result_text = (char *)realloc(result_text, result_len + 3);
				memcpy(buffer, x, 2);
				result_text[result_len] = (char)strtol(buffer, 0, 16);
				memcpy(buffer, x + 3, 2);
				result_text[result_len + 1] = (char)strtol(buffer, 0, 16);
				memcpy(buffer, x + 6, 2);
				result_text[result_len + 2] = (char)strtol(buffer, 0, 16);
				result_len += 3;
				chunk_len = 9;

				// check if eight digits F0, F1..F3, F4  //SELECT net.uri_to_text(E':%f0%9d%90%80:'); bold A
			} else if ((strncasecmp(x, "f", 1) == 0) && ((strncmp(x + 1, "0", 1) >= 0 && strncmp(x + 1, "4", 1) <= 0))) {
				// sunlogf( 7, "We have a four byte char.");
				result_text = (char *)realloc(result_text, result_len + 4);
				memcpy(buffer, x, 2);
				result_text[result_len] = (char)strtol(buffer, 0, 16);
				memcpy(buffer, x + 3, 2);
				result_text[result_len + 1] = (char)strtol(buffer, 0, 16);
				memcpy(buffer, x + 6, 2);
				result_text[result_len + 2] = (char)strtol(buffer, 0, 16);
				memcpy(buffer, x + 9, 2);
				result_text[result_len + 3] = (char)strtol(buffer, 0, 16);
				result_len += 4;
				chunk_len = 12;

				// not a valid character
			} else {
				// sunlogf( 7, "Invalid starting character detected. Returning literal percent character.");
				result_text = (char *)realloc(result_text, result_len + chunk_len);
				result_ptr = result_text + result_len;
				memcpy(result_ptr, loop_ptr, chunk_len);
				result_len += chunk_len;
			}

			// in case of + return a space
		} else if (strncmp(loop_ptr, "+", 1) == 0) {
			// sunlogf( 7, "plus detected");
			result_text = (char *)realloc(result_text, result_len + 1);
			result_ptr = result_text + result_len;
			memcpy(result_ptr, " ", 1);
			result_len += 1;

			// in case of everything else, just add to output as is
		} else {
			// sunlogf( 7, "char detected: %s;", loop_ptr);
			result_text = (char *)realloc(result_text, result_len + chunk_len);
			result_ptr = result_text + result_len;
			memcpy(result_ptr, loop_ptr, chunk_len);
			result_len += chunk_len;
		}
		// to debug: uncomment these three lines at the same time:
		// result_text = (char *)realloc( result_text, result_len+1 );
		// result_text[result_len] = 0;
		// sunlogf( 7, "result_len: %i, result_text: %s", result_len, result_text );

		// looping
		loop_ptr += chunk_len;
		inputstring_len -= chunk_len;
	}
	// sunlogf( 7, "end");
	result_text = (char *)realloc(result_text, result_len + 1);
	result_text[result_len] = '\0';
	return result_text;
}

// returns unencoded key for value as char
char *getpar(char *query, char *key) {
	//@ gets converted into _
	char *answer;
	char *end;
	char *result;
	size_t answer_len;
	size_t key_len;

	key = cat_cstr(key, "=");
	key_len = strlen(key);

	do {
		// sunlogf(7, "query:%s, key:%s, key_len:%i", query, key, key_len);
		if (strncmp(query, key, key_len) == 0) {
			answer = query + key_len;

			// strstr to find answer length.
			end = strstr(answer, "&");
			if (end == 0) {
				char *ret = uri_to_cstr(answer, strlen(answer));
				free(key);
				return ret;
			}

			answer_len = (size_t)(end - answer);
			// sunlogf( 7, "answer_len: %i", answer_len);
			result = (char *)salloc(answer_len + 1);
			memcpy(result, answer, answer_len);
			result[answer_len] = 0;
			char *ret = uri_to_cstr(result, answer_len);
			free(result);
			free(key);
			return ret;
		}
		// sunlogf( 7, "rrsg");
		query = strstr(query, "&");
		if (query != 0)
			query += 1;
	} while (query != 0);

	// didn't find anything
	free(key);
	return cat_cstr("");
}

/*
 * c string concatenation for building commands in a way
 *      that is more easy to read than nesting
 */
char *c_cat(int args, ...) {
	va_list ap;
	va_list bp;
	// va_list cp;
	char *output;

	int i;
	va_start(ap, args);
	va_copy(bp, ap);
	// va_copy (cp, ap);
	size_t total_len = 0;
	size_t lengths[args];

	// store lengths for the args
	for (i = 0; i < args; i = i + 1) {
		char *temp = va_arg(ap, char *);
		size_t len;
		if (temp) {
			len = strlen(temp);
		} else {
			len = 0;
		}
		lengths[i] = len;
		total_len += len;
		// sunlogf( 7, "total len:%d", total_len );
	}
	va_end(ap);

	// allocate a field large enough for everything
	output = (char *)salloc(total_len + 1);
	char *result = output;
	char *temp_ptr;

	// fill the new field
	for (i = 0; i < args; i = i + 1) {
		temp_ptr = va_arg(bp, char *);
		if (lengths[i] > 0) {
			// sunlogf( 7, "\nlengths[i]: %i", lengths[i]);
			memcpy(result, temp_ptr, lengths[i]);
			result += lengths[i];
		}
		// sunlogf( 7, "\ni: %i, lengths:%d   result:%s   output_len:%d   output:%s;", i, lengths[i], result, strlen(output),
		// output );
		// sunlogf( 7, "\nva_arg(cp, char *):%s:\n", va_arg(cp, char *));
	}

	// add a null terminator
	*result = '\0';
	va_end(bp);

	return output;
}

/*
 * c string concatenation for building commands in a way
 *      that is more easy to read than nesting
 */
// cat_append is just like cat_cstr except the first argument is free()d
char *c_append(int args, ...) {
	va_list ap;
	va_list bp;
	// va_list cp;
	char *output;

	int i;
	va_start(ap, args);
	va_copy(bp, ap);
	// va_copy (cp, ap);
	size_t total_len = 0;
	size_t lengths[args];

	// store lengths for the args
	for (i = 0; i < args; i = i + 1) {
		size_t len = strlen(va_arg(ap, char *));
		lengths[i] = len;
		total_len += len;
		// sunlogf( 7, "i:%d\n", i );
		// sunlogf( 7, "total len:%d", total_len );
	}
	va_end(ap);

	// allocate a field large enough for everything
	output = (char *)salloc(total_len + 1);
	char *result = output;
	char *temp_ptr;

	// fill the new field
	for (i = 0; i < args; i = i + 1) {
		temp_ptr = va_arg(bp, char *);
		if (lengths[i] > 0) {
			// sunlogf( 7, "\nlengths[i]: %i", lengths[i]);
			memcpy(result, temp_ptr, lengths[i]);
			result += lengths[i];
		}

		// free first arg
		if (i == 0) {
			free(temp_ptr);
		}
		// sunlogf( 7, "\ni: %i, lengths:%d   result:%s   output_len:%d   output:%s;", i, lengths[i], result, strlen(output),
		// output );
		// sunlogf( 7, "\nva_arg(cp, char *):%s:\n", va_arg(cp, char *));
	}

	// add a null terminator
	*result = '\0';
	va_end(bp);

	return output;
}
