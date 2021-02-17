#ifndef ADAPTIVE_NODE_H
#define ADAPTIVE_NODE_H

#include <stdint.h>
#include <stdbool.h>

#include "bit_buffer.h"


typedef struct adaptive_node
{
    struct adaptive_node* parent;
    struct adaptive_node* left;
    struct adaptive_node* right;
    uint64_t weight;
    uint8_t value;
    bool is_nyt;
}
adaptive_node;

typedef enum
{
    NODE_TYPE_LEAF = 0,
    NODE_TYPE_INTERNAL = 1,
    NODE_TYPE_NYT = 2,
}
adaptive_node_type;


extern adaptive_node* adaptive_node_init(bool is_nyt, uint64_t weight, adaptive_node* parent, adaptive_node* left, adaptive_node* right, uint8_t value);

extern void adaptive_node_delete(adaptive_node*);

extern adaptive_node_type adaptive_node_get_type(const adaptive_node*);

extern bit_buffer* adaptive_node_get_path(const adaptive_node*, bit_buffer*);


#endif
