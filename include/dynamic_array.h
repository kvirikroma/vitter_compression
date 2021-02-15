#ifndef DYNAMIC_ARRAY_H
#define DYNAMIC_ARRAY_H

#include <stdint.h>


typedef struct {
    uint64_t length;
    uint64_t allocated;
    uint64_t first_item;
} dynamic_array;

// initializes dynamic array
extern dynamic_array* array_init();

// deletes dynamic array
extern void array_delete(dynamic_array*);

// frees memory using each item as pointer
extern void array_delete_each(dynamic_array*);

// appends value to the end
extern dynamic_array* array_append_value(dynamic_array*, uint64_t);

// returns value from the end and decreases length
extern uint64_t array_pop_value(dynamic_array*);

// returns address of item or nullptr if it doesn't exist
extern uint64_t* array_get_by_index(dynamic_array*, uint64_t);

// reallocates minimal memory size to contain data
extern dynamic_array* array_shrink_to_fit(dynamic_array*);

// extends array by the items of other array
extern dynamic_array* array_extend(dynamic_array*, const dynamic_array*);

// extends array by the qwords from mem
extern dynamic_array* array_extend_from_mem(dynamic_array*, const uint64_t*, uint64_t);

// zeroes array length
extern void array_clear(dynamic_array*);

// casts the dynamic array to usual array in-place ( O(n) )
extern uint64_t* array_to_usual(dynamic_array*);


#endif
