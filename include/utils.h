#ifndef UTILS_H
#define UTILS_H

#include <stdint.h>


// Return the same pointer or print error message and exit with code 1
void* check_pointer_after_malloc(void* ptr);

// Return maximum of given integer arguments
int64_t max(uint32_t items_count, ...);

// Return minimum of given integer arguments
int64_t min(uint32_t items_count, ...);

#endif
