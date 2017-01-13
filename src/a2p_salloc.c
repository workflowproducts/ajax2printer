#include "a2p_salloc.h"

// No socket or libpq connection yet, so skip those steps
void *salloc(size_t size) {
	void *void_return = malloc(size);
	//	void *void_return = NULL; //for debugging only
	if (void_return) {
		return void_return;
	}
	// Darn it. Oh Hey! Maybe memory will be okay in 2 seconds =)
	sleep(2);
	void_return = malloc(size);
	if (void_return) {
		return void_return;
	}
	// No? =( Oh Well. Let's error and then return
	sunlogf(4, "Malloc Error.");
	exit(1);
}
