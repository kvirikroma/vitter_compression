#include <stdlib.h>
#include <string.h>
#include <stdarg.h>

#include "include/adaptive_tree.h"
#include "include/utils.h"


#define NODES_MAP_SIZE (256 * sizeof(adaptive_node*))


static int64_t max(uint8_t items_count, ...)
{
    va_list args;
    va_start(args, items_count);
    int64_t result = INT64_MIN;
    for (uint8_t item = 0; item < items_count; item++)
    {
        int64_t current_arg = va_arg(args, int64_t);
        if (result < current_arg)
        {
            result = current_arg;
        }
    }
    va_end(args);
    return result;
}

static void remove_node_in_traversal(adaptive_node* node, bit_buffer* path, void* params)
{
    adaptive_tree* tree = (adaptive_tree*)params;
    if (adaptive_node_get_type(node) == NODE_TYPE_LEAF)
    {
        tree->nodes_map[node->value] = NULL;
    }
    adaptive_node_delete(node);
}

void adaptive_tree_init(adaptive_tree* self, uint8_t weight)
{
    self->nyt_node = adaptive_node_init(true, 0, NULL, NULL, NULL, weight);
    self->root = self->nyt_node;
    self->nodes_map = malloc(NODES_MAP_SIZE);
    memset(self->nodes_map, 0, NODES_MAP_SIZE);
}

void adaptive_tree_traversal(
    adaptive_tree* self, void(*node_handler)(adaptive_node*, bit_buffer*, void*), void* params, bit_buffer* current_path, adaptive_node* current_node
){
    if (!current_path)
    {
        current_path = check_pointer_after_malloc(malloc(sizeof(bit_buffer)));
        bit_buffer_init(current_path);
    }
    if (!current_node)
    {
        current_node = self->root;
    }
    if (adaptive_node_get_type(current_node) == NODE_TYPE_INTERNAL)
    {
        if (current_node->left)
        {
            bit_buffer_add_bit(current_path, 0);  // path for the next node
            adaptive_tree_traversal(self, node_handler, params, current_path, current_node->left);
        }
        if (current_node->right)
        {
            bit_buffer_add_bit(current_path, 1);  // path for the next node
            adaptive_tree_traversal(self, node_handler, params, current_path, current_node->right);
        }
    }
    if (adaptive_node_get_type(current_node) == NODE_TYPE_LEAF)
    {
        node_handler(current_node, current_path, params);
    }
    bit_buffer_pop_bit(current_path);
}

void adaptive_tree_delete(adaptive_tree* self)
{
    adaptive_tree_traversal(self, remove_node_in_traversal, self, NULL, NULL);
    free(self->nodes_map);
    self->nodes_map = 0;
    self->nyt_node = 0;
    self->root = 0;
}

static void check_node_children(adaptive_node* node)
{
    if (node->left && node->right)
    {
        if (((adaptive_node*)node->left)->weight > ((adaptive_node*)node->right)->weight)
        {
            adaptive_node_exchange(node->left, node->right);
        }
    }
}

static void recalculate_node_weight(adaptive_node* node)
{
    if (adaptive_node_get_type(node) != NODE_TYPE_LEAF)
    {
        node->weight = (node->left? ((adaptive_node*)node->left)->weight : 0) + (node->right? ((adaptive_node*)node->right)->weight : 0);
    }
}

static void update_weights(adaptive_tree* tree, adaptive_node* node)
{
    recalculate_node_weight(node);
    uint8_t max_leaf_weight = adaptive_node_get_type(node)==NODE_TYPE_LEAF? node->weight : 0;
    adaptive_node* current_node = node;
    uint8_t max_node_weight = node->weight;
    while (current_node != tree)
    {
        max_node_weight = max(3,
            node->weight,
            node->left? ((adaptive_node*)current_node->left)->weight : 0,
            node->right? ((adaptive_node*)current_node->right)->weight : 0
        );
        max_leaf_weight = max(4,
            max_leaf_weight,
            adaptive_node_get_type(current_node)==NODE_TYPE_LEAF? node->weight : 0,
            node->left && adaptive_node_get_type(current_node->left)==NODE_TYPE_LEAF? ((adaptive_node*)node->left)->weight : 0,
            node->right && adaptive_node_get_type(current_node->right)==NODE_TYPE_LEAF? ((adaptive_node*)node->right)->weight : 0
        );
        if (current_node->left && adaptive_node_get_type(current_node->left)==NODE_TYPE_LEAF && )
        {}
    }
}

void adaptive_tree_update(adaptive_tree* self, uint8_t value, bit_buffer* buffer)
{
    if (self->nodes_map[value])
    {
    }
    else
    {
    }
}
