#ifndef ADAPTIVE_TREE_H
#define ADAPTIVE_TREE_H

#include "adaptive_node.h"


typedef struct
{
    adaptive_node* nyt_node;
    adaptive_node* root;
}
adaptive_tree;

void adaptive_tree_init(adaptive_tree*, uint16_t);

void adaptive_tree_delete(adaptive_tree*);

void adaptive_tree_traversal(adaptive_tree*, void(*)(adaptive_node*, void*), void*, bit_buffer*);

void adaptive_tree_push(adaptive_tree*, uint16_t);


#endif
