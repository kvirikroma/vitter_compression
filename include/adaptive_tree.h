#ifndef ADAPTIVE_TREE_H
#define ADAPTIVE_TREE_H

#include "adaptive_node.h"
#include "deque.h"
#include "map.h"


typedef struct
{
    adaptive_node* nyt_node;
    adaptive_node* root;
    map leaves_map;  // index is the value of node
    map weights_map;  // array<deque<adaptive_node*>>
}
adaptive_tree;


void adaptive_tree_init(adaptive_tree*, uint8_t);

void adaptive_tree_delete(adaptive_tree*);

void adaptive_tree_traversal(adaptive_tree*, void(*)(adaptive_node*, bit_buffer*, void*), void*, bit_buffer*, adaptive_node*);

void adaptive_tree_update(adaptive_tree*, uint8_t, bit_buffer*);


#endif
