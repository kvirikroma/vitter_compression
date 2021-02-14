#ifndef ENCODER_H
#define ENCODER_H

#include "adaptive_tree.h"


typedef struct
{
    adaptive_tree tree;
    bit_buffer* output_buffer;
    void(*writer)(uint8_t* bytes, uint32_t count, void* params);
    void* writer_params;
}
encoder;


void encoder_init(encoder*, void(*)(uint8_t*, uint32_t, void*), void*);

void encoder_delete(encoder*, bool);

void encoder_write_bytes(encoder*, uint8_t*, uint32_t);

void encoder_flush(encoder*);


#endif
