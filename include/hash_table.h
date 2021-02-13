#include <stdint.h>
#include <stdbool.h>

#include "deque.h"


typedef struct
{
    deque* data;
    uint8_t accuracy;  // bigger accuracy will take more memory but work faster
    uint64_t current_size;
    uint64_t items_count;
    uint64_t (*hash_function)(void*);
}
hash_table;

#define INIT_DATA_SIZE 16


// Initialize the hash table
void hash_table_init(hash_table*, uint8_t, uint64_t(*)(void*));

// Delete the hash table
void hash_table_delete(hash_table*);

// Insert an item to hash table
void hash_table_insert_item(hash_table*, void*);

// Check if the item is present in the hash table
bool hash_table_is_present(hash_table*, void*);

// Remove an item from the hash table
void hash_table_remove_item(hash_table*, void*);
