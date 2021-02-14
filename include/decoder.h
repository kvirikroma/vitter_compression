#ifndef DECODER_H
#define DECODER_H

#include "adaptive_tree.h"

#define DECODER_OUTPUT_BUFFER_SIZE 1024


typedef struct
{
    adaptive_tree tree;
    uint8_t* output_buffer;
    uint32_t output_buffer_position;
    bit_buffer input_buffer;
    void(*writer)(uint8_t* bytes, uint32_t count, void* params);
    void* writer_params;
}
decoder;


void decoder_init(decoder*, void(*)(uint8_t*, uint32_t, void*), void*);

void decoder_delete(decoder*, bool);

void decoder_write(decoder*, uint8_t*, uint32_t);

void decoder_flush(decoder*);


#endif
