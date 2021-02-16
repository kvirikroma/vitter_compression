#include <stdlib.h>

#include "include/utils.h"
#include "include/bit_buffer.h"
#include "include/bit_array.h"


void bit_buffer_init(bit_buffer* self)
{
    self->data = array_init();
    self->last_item = 0;
    self->last_item_size = 0;
}

void bit_buffer_delete(bit_buffer* self)
{
    array_delete(self->data);
    self->data = NULL;
    self->last_item = 0;
    self->last_item_size = 0;
}

void bit_buffer_clear(bit_buffer* self)
{
    array_clear(self->data);
    self->last_item = 0;
    self->last_item_size = 0;
}

void bit_buffer_push_bit(bit_buffer* self, bool bit_value)
{
    self->last_item |= ((uint64_t)bit_value) << self->last_item_size;
    self->last_item_size++;
    if (self->last_item_size >= 64)
    {
        self->data = array_append_value(self->data, self->last_item);
        self->last_item_size = 0;
        self->last_item = 0;
    }
}

bool bit_buffer_pop_bit(bit_buffer* self)
{
    if (self->last_item_size == 0)
    {
        self->last_item_size = 63;
        self->last_item = array_pop_value(self->data);
    }
    uint64_t result = 1 << self->last_item_size;
    result &= self->last_item;
    self->last_item ^= result;
    result >>= self->last_item_size;
    self->last_item_size--;
    return (bool)result;
}

bool bit_buffer_get_bit(bit_buffer* self, uint64_t bit_number)
{
    if (bit_number + 1 > self->data->length * 64)
    {
        return bit_array_get_bit(&self->last_item, bit_number - (self->data->length * 64));
    }
    else
    {
        return bit_array_get_bit(&self->data->first_item, bit_number);
    }
}

uint64_t bit_buffer_get_size(bit_buffer* self)
{
    return self->data->length + self->last_item_size;
}

void bit_buffer_reverse(bit_buffer* self)
{
    bit_buffer new_buffer;
    bit_buffer_init(&new_buffer);
    for (uint64_t i = bit_buffer_get_size(self); i > 0; i--)
    {
        bit_buffer_push_bit(&new_buffer, bit_buffer_get_bit(self, i - 1));
    }
    bit_buffer_delete(self);
    self->data = new_buffer.data;
    self->last_item = new_buffer.last_item;
    self->last_item_size = new_buffer.last_item_size;
}

dynamic_array* bit_buffer_get_full_part(bit_buffer* self, bool detach)
{
    if (detach)
    {
        dynamic_array* result = self->data;
        self->data = array_init();
        return result;
    }
    else
    {
        return self->data;
    }
}

void bit_buffer_extend(bit_buffer* self, bit_buffer* other)
{
    for (uint64_t i = 0; i < (other->data->length + other->last_item_size); i++)
    {
        bit_buffer_push_bit(self, bit_buffer_get_bit(other, i));
    }
}

bool bit_buffer_starts_with(bit_buffer* self, bit_buffer* other)
{
    uint64_t min_size = min(2, bit_buffer_get_size(self), bit_buffer_get_size(other));
    for (uint64_t i = 0; i < min_size; i++)
    {
        if (bit_buffer_get_bit(self, i) != bit_buffer_get_bit(other, i))
        {
            return false;
        }
    }
    return true;
}

void bit_buffer_extend_from_memory(bit_buffer* self, const uint8_t* data, uint32_t bits_count)
{
    for (uint64_t i = 0; i < bits_count * 8; i++)
    {
        bit_buffer_push_bit(self, bit_array_get_bit(data, i));
    }
}
