#ifndef UTILS_H
#define UTILS_H

// Return the same pointer or print error message and exit with code 1
void* check_pointer_after_malloc(void* ptr);

// Return maximum of given integer arguments
int64_t max(uint8_t items_count, ...);

#endif
