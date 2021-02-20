#ifndef BIT_BUFFER_H
#define BIT_BUFFER_H

#include <stdint.h>
#include <stdbool.h>

#include "dynamic_array.h"


typedef struct {
    dynamic_array* data;
    uint64_t last_item;
    uint64_t last_item_size;
} bit_buffer;

void bit_buffer_init(bit_buffer*);

void bit_buffer_delete(bit_buffer*);

void bit_buffer_clear(bit_buffer*);

void bit_buffer_push_bit(bit_buffer*, bool);

bool bit_buffer_pop_bit(bit_buffer*);

bool bit_buffer_get_bit(bit_buffer*, uint64_t);

uint64_t bit_buffer_get_size(bit_buffer*);

void bit_buffer_reverse(bit_buffer*);

dynamic_array* bit_buffer_get_full_part(bit_buffer*, bool);

void bit_buffer_extend(bit_buffer*, bit_buffer*);

void bit_buffer_extend_reversed(bit_buffer*, bit_buffer*);

bool bit_buffer_starts_with(bit_buffer*, bit_buffer*);

void bit_buffer_extend_from_memory(bit_buffer*, const uint8_t*, uint32_t);

void bit_buffer_push_byte(bit_buffer*, uint8_t);


#endif
