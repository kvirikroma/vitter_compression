#include <stdlib.h>

#include "include/encoder.h"


void encoder_init(encoder* self, void(*writer)(uint8_t*, uint32_t, void*), void* writer_params)
{
    adaptive_tree_init(&self->tree);
    self->writer_params = writer_params;
    self->writer = writer;
}

void encoder_delete(encoder* self, bool free_params)
{
    if (free_params)
    {
        free(self->writer_params);
    }
    self->writer_params = NULL;
    self->writer = NULL;
    adaptive_tree_delete(&self->tree);
}

void encoder_write_bytes(encoder* self, uint8_t* data, uint32_t length);

void encoder_flush(encoder* self);
