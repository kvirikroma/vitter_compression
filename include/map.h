#ifndef MAP_H
#define MAP_H

#include "hash_table.h"

#define MAP_DEFAULT_ACCURACY 65


typedef struct 
{
    hash_table table;
    uint64_t(*hash_function)(const void*);
    bool (*comparison_function)(const void*, const void*);
}
map;


void map_init(map*, uint64_t(*)(const void*), bool(*)(const void*, const void*));

void map_delete(map*);

void map_set_item(map*, const void*, void*);

void* map_get_item(map*, const void*);

bool map_is_present(map*, const void*);

void* map_remove_item(map*, const void*);

void map_get_keys(map*, const void**);

void map_get_values(map*, void**);

void map_iterate_keys(map*, void(*)(const void*, void*), void*);

void map_iterate_values(map*, void(*)(void*, void*), void*);


#endif