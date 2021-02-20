#ifndef DECODER_H
#define DECODER_H

#include "adaptive_tree.h"

#define DECODER_OUTPUT_BUFFER_SIZE 204800


typedef enum
{
    STATE_READING_NODE,
    STATE_READING_VALUE
}
decoding_state;

typedef struct
{
    adaptive_tree tree;
    uint8_t* output_buffer;
    uint32_t output_buffer_position;
    bit_buffer value_to_write;
    void(*writer)(const uint8_t* bytes, uint32_t count, void* params);
    void* writer_params;
    decoding_state current_state;
    adaptive_node* current_node;
}
decoder;


void decoder_init(decoder*, void(*)(const uint8_t*, uint32_t, void*), void*);

void decoder_delete(decoder*, bool);

void decoder_write(decoder*, const uint8_t*, uint32_t);

void decoder_flush(decoder*);


#endif
