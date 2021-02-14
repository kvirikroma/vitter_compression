#include <stdlib.h>

#include "include/utils.h"
#include "include/hash_table.h"


void hash_table_init(hash_table* self, uint8_t accuracy, uint64_t(*hash_function)(void*), bool(*comparison_function)(void*, void*))
{
    self->data = check_pointer_after_malloc(calloc(INIT_DATA_SIZE, sizeof(deque)));  // all deques are already zeroed by calloc and so initialized
    if (accuracy < MIN_ACCURACY)
    {
        self->accuracy = MIN_ACCURACY;
    }
    else if (accuracy > MAX_ACCURACY)
    {
        self->accuracy = MAX_ACCURACY;
    }
    else
    {
        self->accuracy = accuracy;
    }
    self->hash_function = hash_function;
    self->comparison_function = comparison_function;
    self->current_size = INIT_DATA_SIZE;
    self->items_count = 0;
}

static void free_data_storage(deque* storage, size_t size)
{
    for (uint64_t i = 0; i < size; i++)
    {
        deque_clear(&storage[i]);
    }
    free(storage);
}

void hash_table_delete(hash_table* self)
{
    free_data_storage(self->data, self->current_size);
    self->data = NULL;
    self->hash_function = NULL;
    self->comparison_function = NULL;
}

static uint8_t calculate_actual_accuracy(hash_table* self)
{
    double result = ((double)self->current_size - (double)self->items_count) / (double)self->current_size;
    return (uint8_t)result;
}

static void move_items(hash_table* self, deque* src_data, size_t size)
{
    for (uint64_t i = 0; i < size; i++)
    {
        while (src_data[i].len)
        {
            hash_table_insert_item(self, (void*)deque_pop_left(&src_data[i]));
        }
    }
}

static void check_for_space(hash_table* self)
{
    uint8_t actual_accuracy = calculate_actual_accuracy(self);
    bool actions_required = false;
    size_t old_storage_size = self->current_size;
    if (actual_accuracy < self->accuracy)
    {
        self->current_size <<= 1;  // *= 2
        actions_required = true;
    }
    else if (actual_accuracy >= SHRINKING_ACCURACY && self->items_count >= SHRINKING_MIN_ITEMS)
    {
        self->current_size >>= 1;  // *= 2
        actions_required = true;
    }
    if (actions_required)
    {
        deque* new_data_storage = check_pointer_after_malloc(calloc(self->current_size, sizeof(deque)));  // all deques are already zeroed by calloc and so initialized
        deque* old_data_storage = self->data;
        self->data = new_data_storage;
        move_items(self, old_data_storage, old_storage_size);
        free_data_storage(old_data_storage, old_storage_size);
    }
}

static deque_node* find_node_by_value(deque* dest, void* value, bool(*comparison_function)(void*, void*))
{
    deque_node* current_node = dest->first;
    while (current_node)
    {
        if (comparison_function(current_node->data, value))
        {
            return current_node;
        }
        current_node = (deque_node*)current_node->next;
    }
    return NULL;
}

static deque* get_deque_of_item(hash_table* self, void* item)
{
    uint64_t place = self->hash_function(item) % self->current_size;
    return &self->data[place];
}

void hash_table_insert_item(hash_table* self, void* item)
{
    deque* deque_to_edit = get_deque_of_item(self, item);
    if (!find_node_by_value(deque_to_edit, item, self->comparison_function))
    {
        deque_push_right(deque_to_edit, (uint64_t)item);
        self->items_count++;
        check_for_space(self);
    }
}

bool hash_table_is_present(hash_table* self, void* item)
{
    deque_node* node = find_node_by_value(get_deque_of_item(self, item), item, self->comparison_function);
    return (bool)node;
}

void* hash_table_get_item(hash_table* self, void* item)
{
    deque_node* node = find_node_by_value(get_deque_of_item(self, item), item, self->comparison_function);
    if (!node)
    {
        return NULL;
    }
    return node->data;
}

void* hash_table_remove_item(hash_table* self, void* item)
{
    deque* deque_to_edit = get_deque_of_item(self, item);
    deque_node* node = find_node_by_value(deque_to_edit, item, self->comparison_function);
    if (node)
    {
        if (deque_to_edit->first == node)
        {
            deque_pop_left(deque_to_edit);
        }
        else if (deque_to_edit->last == node)
        {
            deque_pop_right(deque_to_edit);
        }
        else
        {
            deque_to_edit->len--;
            ((deque_node*)node->prev)->next = node->next;
            ((deque_node*)node->next)->prev = node->prev;
        }
        self->items_count--;
        check_for_space(self);
        return node->data;
    }
    return NULL;
}

void hash_table_iterate(hash_table* self, void(*item_receiver)(void* item, void* params), void* params)
{
    for (uint64_t i = 0; i < self->current_size; i++)
    {
        deque_node* current_node = self->data[i].first;
        while (current_node)
        {
            item_receiver((void*)current_node->data, params);
            current_node = (deque_node*)current_node->next;
        }
    }
}
