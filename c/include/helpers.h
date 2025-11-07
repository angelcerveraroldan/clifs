#include <stddef.h>

// A simple string
typedef struct cf_str_t {
    // The number of characters in `data`.
    size_t len;
    // The maximum number of characters that `data` can store before needing
    // reallocation.
    size_t cap;
    // A pointer to the characters this string represents.
    char *data;
} cfstr_t;

// Change the the `data` field of `s` to a new string. This will handle any
// internal re-allocation that is needed.
int cstr_set(cfstr_t *s, const char *new_str);

void free_cstr(cfstr_t *);

// Returns index of the slash before the final path component (leaf).
// Trims trailing '/' (except root). Returns -1 if no parent/leaf split exists.
int last_dash(const char *path);
