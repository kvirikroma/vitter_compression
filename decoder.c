#include <stdlib.h>

#include "include/decoder.h"
#include "include/utils.h"


typedef enum
{
    STATE_IDLE,
    STATE_READING_NODE,
    STATE_READING_VALUE
}
decoding_state;


void decoder_init(decoder* self, void(*writer)(uint8_t*, uint32_t, void*), void* writer_params)
{
    self->writer_params = writer_params;
    self->writer = writer;
    bit_buffer_init(&self->input_buffer);
    adaptive_tree_init(&self->tree);
    self->output_buffer_position = 0;
    self->output_buffer = check_pointer_after_malloc(malloc(DECODER_OUTPUT_BUFFER_SIZE));
}

void decoder_delete(decoder* self, bool free_params)
{
    if (free_params)
    {
        free(self->writer_params);
    }
    self->writer = NULL;
    self->writer_params = NULL;
    bit_buffer_delete(&self->input_buffer);
    adaptive_tree_delete(&self->tree);
    free(self->output_buffer);
    self->output_buffer = NULL;
    self->output_buffer_position = 0;
}

void decoder_write(decoder* self, uint8_t* data, uint32_t length)
{
    bit_buffer_extend_from_memory(&self->input_buffer, data, length);
    decoding_state current_state = STATE_IDLE;
    uint64_t idle_was_before = 0;
    uint64_t current_bit = 0;
}

void decoder_flush(decoder*);
