#pragma once

#include "a2p_sunlogf.h"
#include <stdlib.h>

#define VA_NUM_ARGS(...)                                                                                                         \
	VA_NUM_ARGS_IMPL(11, ##__VA_ARGS__, 45, 44, 43, 42, 41, 40, 39, 38, 37, 36, 35, 34, 33, 32, 31, 30, 29, 28, 27, 26, 25, 24,  \
		23, 22, 21, 20, 19, 18, 17, 16, 15, 14, 13, 12, 11, 10, 9, 8, 7, 6, 5, 4, 3, 2, 1, 0)
#define VA_NUM_ARGS_IMPL(_0, _1, _2, _3, _4, _5, _6, _7, _8, _9, _10, _11, _12, _13, _14, _15, _16, _17, _18, _19, _20, _21,     \
						 _22, _23, _24, _25, _26, _27, _28, _29, _30, _31, _32, _33, _34, _35, _36, _37, _38, _39, _40, _41,     \
						 _42, _43, _44, _45, N, ...)                                                                             \
	N

void *salloc(size_t size);

#define SFREE(A)                                                                                                                 \
	if (A != NULL) {                                                                                                             \
		free(A);                                                                                                                 \
		A = NULL;                                                                                                                \
	}

// Convienence macro for error checking
#define check(L, A, M, ...)                                                                                                      \
	if (!(A)) {                                                                                                                  \
		sunlogf(L, M, ##__VA_ARGS__);                                                                                            \
		errno = 0;                                                                                                               \
		goto error;                                                                                                              \
	}
