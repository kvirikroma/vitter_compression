#ifndef HASH_TABLE_H
#define HASH_TABLE_H

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
    bool (*comparison_function)(void*, void*);  // it is needed to check equality of an items
}
hash_table;

#define INIT_DATA_SIZE      16
#define MIN_ACCURACY        8
#define MAX_ACCURACY        90
#define SHRINKING_ACCURACY  98
#define SHRINKING_MIN_ITEMS 8


// Initialize the hash table
void hash_table_init(hash_table*, uint8_t, uint64_t(*)(void*), bool(*)(void*, void*));

// Delete the hash table
void hash_table_delete(hash_table*);

// Insert an item to hash table
void hash_table_insert_item(hash_table*, void*);

// Check if the item is present in the hash table
bool hash_table_is_present(hash_table*, void*);

// Get item by itself or equivalent one
void* hash_table_get_item(hash_table* self, void* item);

// Remove an item from the hash table; returns item if it was successfully found and deleted
void* hash_table_remove_item(hash_table*, void*);

// Iterate through hash table items and execute a function for each of them with the item and a custom parameter
void hash_table_iterate(hash_table*, void(*)(void*, void*), void*);


#endif
