#ifndef ENCODER_H
#define ENCODER_H

#include <stdint.h>

#include "adaptive_tree.h"
#include "coder.h"

#define BITS_IN_ARRAY_ITEM (sizeof(uint64_t) * 8)
#define ENCODER_FLUSH_ON_BITS (BITS_IN_ARRAY_ITEM * 1280)


typedef struct
{
    coder base;
    bit_buffer output_buffer;
    struct {} vtbl;
}
encoder;


void encoder_init(encoder*, void(*)(const uint8_t*, uint32_t, void*), void*);

void encoder_delete(coder*, bool);

void encoder_write(coder*, const uint8_t*, uint32_t);

void encoder_flush(coder*);


#endif
