#include <stdlib.h>

#include <include/hash_table.h>


void hash_table_init(hash_table* self, uint8_t accuracy, uint64_t(*hash_function)(void*))
{
    self->data = calloc(INIT_DATA_SIZE, sizeof(deque));  // all deques are already zeroed by calloc and so initialized
    if (accuracy < 10)
    {
        self->accuracy = 10;
    }
    else if (accuracy > 245)
    {
        self->accuracy = 245;
    }
    else
    {
        self->accuracy = accuracy;
    }
    self->hash_function = hash_function;
    self->current_size = INIT_DATA_SIZE;
    self->items_count = 0;
}

void hash_table_delete(hash_table* self);

static uint8_t calculate_actual_accuracy(hash_table* self);
static void move_items(hash_table* old, hash_table* new);
static void check_for_space(hash_table* self);

void hash_table_insert_item(hash_table* self, void* item);

bool hash_table_is_present(hash_table* self, void* item);

void hash_table_remove_item(hash_table* self, void* item);
