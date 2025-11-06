#include <stddef.h>

// A simple string
typedef struct cstr_t {
	// The number of characters in `data`.
	size_t len;
	// The maximum number of characters that `data` can store before needing
	// reallocation.
	size_t cap;
	// A pointer to the characters this string represents.
	char *data;
} cstr_t;

// Change the the `data` field of `s` to a new string. This will handle any
// internal re-allocation that is needed.
int cstr_set(cstr_t * s, const char * new_str);

void free_cstr(cstr_t *);
