#include <stdlib.h>

#include "include/decoder.h"
#include "include/utils.h"
#include "include/bit_array.h"


void decoder_init(decoder* self, void(*writer)(uint8_t*, uint32_t, void*), void* writer_params)
{
    self->writer_params = writer_params;
    self->writer = writer;
    adaptive_tree_init(&self->tree);
    bit_buffer_init(&self->value_to_write);
    self->output_buffer_position = 0;
    self->current_node = self->tree.root;
    self->current_state = STATE_IDLE;
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
    adaptive_tree_delete(&self->tree);
    bit_buffer_delete(&self->value_to_write);
    free(self->output_buffer);
    self->output_buffer = NULL;
    self->output_buffer_position = 0;
    self->current_node = NULL;
    self->current_state = STATE_IDLE;
}

static uint8_t write_value_to_buffer(decoder* self)
{
    uint8_t result = (uint8_t)self->value_to_write.last_item;
    self->output_buffer[self->output_buffer_position] = result;
    self->output_buffer_position++;
    bit_buffer_clear(&self->value_to_write);
    if (self->output_buffer_position >= DECODER_OUTPUT_BUFFER_SIZE)
    {
        decoder_flush(self);
    }
    return result;
}

void decoder_write(decoder* self, uint8_t* data, uint32_t length)
{
    for (uint64_t i = 0; i < (length * 8); i++)
    {
        switch (self->current_state)
        {
            case STATE_IDLE:
            {
                if (self->tree.root->is_nyt)
                {
                    self->current_state = STATE_READING_VALUE;
                    bit_buffer_push_bit(&self->value_to_write, bit_array_get_bit(data, i));
                }
                else
                {
                    self->current_state = STATE_READING_NODE;
                    if (bit_array_get_bit(data, i))
                    {
                        self->current_node = (adaptive_node*)self->current_node->right;
                    }
                    else
                    {
                        self->current_node = (adaptive_node*)self->current_node->left;
                    }
                }
                
                break;
            }
            case STATE_READING_NODE:
            {
                if (adaptive_node_get_type(self->current_node) == NODE_TYPE_INTERNAL)
                {
                    if (bit_array_get_bit(data, i))
                    {
                        self->current_node = (adaptive_node*)self->current_node->right;
                    }
                    else
                    {
                        self->current_node = (adaptive_node*)self->current_node->left;
                    }
                }
                else
                {
                    if (self->current_node->is_nyt)
                    {
                        self->current_state = STATE_READING_VALUE;
                        bit_buffer_push_bit(&self->value_to_write, bit_array_get_bit(data, i));
                    }
                    else
                    {
                        adaptive_tree_update(&self->tree, self->current_node->value);
                        self->current_node = self->tree.root;
                        self->current_state = STATE_IDLE;
                    }
                }
                
                break;
            }
            case STATE_READING_VALUE:
            {
                bit_buffer_push_bit(&self->value_to_write, bit_array_get_bit(data, i));
                if (bit_buffer_get_size(&self->value_to_write) >= 8)
                {
                    adaptive_tree_update(&self->tree, write_value_to_buffer(self));
                    self->current_node = self->tree.root;
                    self->current_state = STATE_IDLE;
                }

                break;
            }
            
            default:
            {
                break;
            }
        }
    }
}

void decoder_flush(decoder* self)
{
    self->writer(self->output_buffer, self->output_buffer_position, self->writer_params);
    self->output_buffer_position = 0;
}
