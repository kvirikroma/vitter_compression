#include <stdlib.h>
#include <string.h>

#include "include/adaptive_tree.h"
#include "include/utils.h"


static uint64_t weight_hash_function(uint64_t weight)
{
    return weight;
}

static bool weight_comparison_function(uint64_t weight, uint64_t other_weight)
{
    return weight == other_weight;
}

static uint64_t leaf_hash_function(uint64_t leaf_value)
{
    return (uint8_t)leaf_value;
}

static bool leaf_comparison_function(uint64_t leaf_value, uint64_t other_leaf_value)
{
    return (uint8_t)leaf_value == (uint8_t)other_leaf_value;
}

static void remove_node_in_traversal(adaptive_node* node, bit_buffer* path, void* params)
{
    adaptive_tree* tree = (adaptive_tree*)params;
    if (adaptive_node_get_type(node) == NODE_TYPE_LEAF)
    {
        map_remove_item(&tree->leaves_map, (void*)(uint64_t)node->value);
    }
    adaptive_node_delete(node);
}

void adaptive_tree_init(adaptive_tree* self, uint8_t weight)
{
    self->nyt_node = adaptive_node_init(true, 0, NULL, NULL, NULL, weight);
    self->root = self->nyt_node;
    map_init(&self->weights_map, (uint64_t(*)(void*))weight_hash_function, (bool(*)(void*, void*))weight_comparison_function);
    map_init(&self->leaves_map, (uint64_t(*)(void*))leaf_hash_function, (bool(*)(void*, void*))leaf_comparison_function);
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
            adaptive_tree_traversal(self, node_handler, params, current_path, (adaptive_node*)current_node->left);
        }
        if (current_node->right)
        {
            bit_buffer_add_bit(current_path, 1);  // path for the next node
            adaptive_tree_traversal(self, node_handler, params, current_path, (adaptive_node*)current_node->right);
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
    map_delete(&self->leaves_map);
    map_delete(&self->weights_map);
    self->nyt_node = 0;
    self->root = 0;
}

static void check_node_children(adaptive_node* node)
{
    if (node->left && node->right)
    {
        if (((adaptive_node*)node->left)->weight > ((adaptive_node*)node->right)->weight)
        {
            adaptive_node_exchange((adaptive_node*)node->left, (adaptive_node*)node->right);
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
}

void adaptive_tree_update(adaptive_tree* self, uint8_t value, bit_buffer* buffer)
{
    if (map_is_present(&self->leaves_map, (void*)(uint64_t)value))
    {
    }
    else
    {
    }
}
