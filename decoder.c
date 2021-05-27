#include <stdlib.h>

#include "include/decoder.h"
#include "include/utils.h"
#include "include/bit_array.h"


void decoder_init(decoder* self, void(*writer)(const uint8_t*, uint32_t, void*), void* writer_params)
{
    self->base.writer_params = writer_params;
    self->base.writer = writer;
    adaptive_tree_init(&self->base.tree);
    bit_buffer_init(&self->value_to_write);
    self->output_buffer_position = 0;
    self->current_node = self->base.tree.root;
    self->current_state = STATE_READING_VALUE;
    self->output_buffer = check_pointer_after_malloc(malloc(DECODER_OUTPUT_BUFFER_SIZE));

    self->base.vtbl.write = decoder_write;
    self->base.vtbl.flush = decoder_flush;
    self->base.vtbl.deinit = decoder_delete;
    self->vtbl.flush_final = decoder_final_flush;
}

void decoder_delete(coder* self, bool free_params)
{
    if (free_params)
    {
        free(self->writer_params);
    }
    self->writer = NULL;
    self->writer_params = NULL;
    adaptive_tree_delete(&self->tree);
    bit_buffer_delete(&((decoder*)self)->value_to_write);
    free(((decoder*)self)->output_buffer);
    ((decoder*)self)->output_buffer = NULL;
    ((decoder*)self)->output_buffer_position = 0;
    ((decoder*)self)->current_node = NULL;
    ((decoder*)self)->current_state = STATE_READING_VALUE;
}

static uint8_t write_value_to_buffer(decoder* self, uint8_t value)
{
    self->output_buffer[self->output_buffer_position] = value;
    self->output_buffer_position++;
    bit_buffer_clear(&self->value_to_write);
    if (self->output_buffer_position >= DECODER_OUTPUT_BUFFER_SIZE)
    {
        self->base.vtbl.flush((coder*)self);
    }
    return value;
}

static void check_current_node(decoder* self)
{
    if (self->current_node == NULL)
    {
        fprintf(stderr, "An error occurred due to incorrect data or a bug. Exiting");
        exit(1);
    }
}

static uint8_t write_bit_value_to_buffer(decoder* self)
{
    uint8_t result = (uint8_t)self->value_to_write.last_item;
    return write_value_to_buffer((decoder*)self, result);
}

void decoder_write(coder* self, const uint8_t* data, uint32_t length)
{
    for (uint64_t i = 0; i < (length * 8); i++)
    {
        check_current_node((decoder*)self);
        switch (((decoder*)self)->current_state)
        {
            case STATE_READING_NODE:
            {
                switch (adaptive_node_get_type(((decoder*)self)->current_node))
                {
                    case NODE_TYPE_INTERNAL:
                    {
                        if (bit_array_get_bit(data, i))
                        {
                            ((decoder*)self)->current_node = (adaptive_node*)((decoder*)self)->current_node->right;
                        }
                        else
                        {
                            ((decoder*)self)->current_node = (adaptive_node*)((decoder*)self)->current_node->left;
                        }
                        break;
                    }
                    case NODE_TYPE_NYT:
                    {
                        ((decoder*)self)->current_state = STATE_READING_VALUE;
                        bit_buffer_push_bit(&((decoder*)self)->value_to_write, bit_array_get_bit(data, i));
                        break;
                    }
                    case NODE_TYPE_LEAF:
                    {
                        adaptive_tree_update(&self->tree, write_value_to_buffer((decoder*)self, ((decoder*)self)->current_node->value));
                        if (bit_array_get_bit(data, i))
                        {
                            ((decoder*)self)->current_node = (adaptive_node*)self->tree.root->right;
                        }
                        else
                        {
                            ((decoder*)self)->current_node = (adaptive_node*)self->tree.root->left;
                        }
                        break;
                    }
                }
                
                break;
            }
            case STATE_READING_VALUE:
            {
                bit_buffer_push_bit(&((decoder*)self)->value_to_write, bit_array_get_bit(data, i));
                if (bit_buffer_get_size(&((decoder*)self)->value_to_write) >= 8)
                {
                    adaptive_tree_update(&self->tree, write_bit_value_to_buffer((decoder*)self));
                    ((decoder*)self)->current_node = self->tree.root;
                    ((decoder*)self)->current_state = STATE_READING_NODE;
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

void decoder_flush(coder* self)
{
    self->writer(((decoder*)self)->output_buffer, ((decoder*)self)->output_buffer_position, self->writer_params);
    ((decoder*)self)->output_buffer_position = 0;
}

void decoder_final_flush(decoder* self)
{
    if (adaptive_node_get_type(self->current_node) == NODE_TYPE_LEAF)
    {
        write_value_to_buffer(self, self->current_node->value);
    }
    self->base.vtbl.flush((coder*)self);
}
