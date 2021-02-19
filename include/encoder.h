#ifndef ENCODER_H
#define ENCODER_H

#include <stdint.h>

#include "adaptive_tree.h"

#define BITS_IN_ARRAY_ITEM (sizeof(uint64_t) * 8)
#define ENCODER_FLUSH_ON_BITS (BITS_IN_ARRAY_ITEM * 640)


typedef struct
{
    adaptive_tree tree;
    bit_buffer output_buffer;
    void(*writer)(const uint8_t* bytes, uint32_t count, void* params);
    void* writer_params;
}
encoder;


void encoder_init(encoder*, void(*)(const uint8_t*, uint32_t, void*), void*);

void encoder_delete(encoder*, bool);

void encoder_write_bytes(encoder*, const uint8_t*, uint32_t);

void encoder_flush(encoder*);


#endif
