#ifndef MAP_H
#define MAP_H

#include "hash_table.h"

#define MAP_DEFAULT_ACCURACY 65


typedef struct 
{
    hash_table table;
    uint64_t(*hash_function)(void*);
    bool (*comparison_function)(void*, void*);
}
map;


void map_init(map*, uint64_t(*)(void*), bool(*)(void*, void*));

void map_delete(map*);

void map_set_item(map*, void*, void*);

void* map_get_item(map*, void*);

bool map_is_present(map*, void*);

void* map_remove_item(map*, void*);

void map_get_keys(map*, void**);

void map_get_values(map*, void**);

void map_iterate(map*, void(*)(void*, void*), void*, bool);


#endif