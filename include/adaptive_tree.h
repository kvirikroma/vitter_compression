#ifndef ADAPTIVE_TREE_H
#define ADAPTIVE_TREE_H

#include <stdio.h>

#include "adaptive_node.h"
#include "deque.h"
#include "map.h"

#define BLOCK_DEFAULT_ACCURACY 50


typedef struct
{
    adaptive_node* nyt_node;
    adaptive_node* root;
    map leaves_map;  // map<uint8_t: adaptive_node*>
    map weights_map;  // map<uint64_t: hash_table<adaptive_node*>>
    FILE* logs_file;
}
adaptive_tree;


void adaptive_tree_init(adaptive_tree*);

void adaptive_tree_delete(adaptive_tree*);

void adaptive_tree_traversal(adaptive_tree*, void(*)(adaptive_node*, bit_buffer*, void*), void*, bit_buffer*, adaptive_node*);

void adaptive_tree_update(adaptive_tree*, uint8_t);

uint8_t adaptive_tree_get_value(adaptive_tree* self, bit_buffer* path);


#endif
