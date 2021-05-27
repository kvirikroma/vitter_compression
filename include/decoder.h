#ifndef DECODER_H
#define DECODER_H

#include "adaptive_tree.h"
#include "coder.h"

#define DECODER_OUTPUT_BUFFER_SIZE 204800


typedef enum
{
    STATE_READING_NODE,
    STATE_READING_VALUE
}
decoding_state;

typedef struct decoder
{
    coder base;
    uint8_t* output_buffer;
    uint32_t output_buffer_position;
    bit_buffer value_to_write;
    decoding_state current_state;
    adaptive_node* current_node;
    struct
    {
        void(*flush_final)(struct decoder*);
    } vtbl;
}
decoder;


void decoder_init(decoder*, void(*)(const uint8_t*, uint32_t, void*), void*);

void decoder_delete(coder*, bool);

void decoder_write(coder*, const uint8_t*, uint32_t);

void decoder_flush(coder*);

void decoder_final_flush(decoder*);


#endif
