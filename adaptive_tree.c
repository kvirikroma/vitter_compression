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

static uint64_t node_hash_function(void* node)
{
    return (uint64_t)node;
}

static bool node_comparison_function(adaptive_node* node, adaptive_node* other_node)
{
    return node == other_node;
}

static void remove_hash_tables(hash_table* table, void* params)
{
    hash_table_delete(table);
}

static void find_highest_node_in_another_subtree(adaptive_node* node, adaptive_node** highest_so_far)
{
    if (*highest_so_far == NULL)
    {
        *highest_so_far = node;
    }
    bit_buffer node_path_storage;
    bit_buffer highest_node_path_storage;
    bit_buffer_init(&node_path_storage);
    bit_buffer_init(&highest_node_path_storage);
    adaptive_node_get_path(node, &node_path_storage);
    adaptive_node_get_path(*highest_so_far, &highest_node_path_storage);
    if (bit_buffer_get_size(&node_path_storage) < bit_buffer_get_size(&highest_node_path_storage))
    {
        if (! bit_buffer_starts_with(&node_path_storage, &highest_node_path_storage))
        {
            *highest_so_far = node;
        }
    }
    bit_buffer_delete(&node_path_storage);
    bit_buffer_delete(&highest_node_path_storage);
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

void adaptive_tree_init(adaptive_tree* self)
{
    self->root = self->nyt_node = adaptive_node_init(true, 0, NULL, NULL, NULL, 0);
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
    map_iterate(&self->weights_map, (void(*)(void*, void*))remove_hash_tables, NULL, true);
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

static hash_table* create_weight_block(adaptive_tree* self, uint64_t weight)
{
    hash_table* new_table = check_pointer_after_malloc(malloc(sizeof(hash_table)));
    hash_table_init(new_table, BLOCK_DEFAULT_ACCURACY, (uint64_t(*)(void*))node_hash_function, (bool(*)(void*, void*))node_comparison_function);
    map_set_item(&self->weights_map, (void*)weight, new_table);
    return new_table;
}

static void delete_weight_block(adaptive_tree* self, hash_table* block)
{
    hash_table_delete(block);
    map_remove_item(&self->weights_map, block);
    free(block);
}

static void increase_weights(adaptive_tree* self, adaptive_node* node)
{
    while (node)
    {
        adaptive_node* highest_node = NULL;
        hash_table_iterate(map_get_item(&self->weights_map, (void*)node->weight), (void(*)(void*, void*))find_highest_node_in_another_subtree, &highest_node);
        if (highest_node != node)
        {
            adaptive_node_exchange(highest_node, node);
        }
        hash_table* old_weights_block = map_get_item(&self->weights_map, (void*)node->weight);
        hash_table_remove_item(old_weights_block, node);
        if (old_weights_block->items_count == 0)
        {
            delete_weight_block(self, old_weights_block);
        }
        node->weight++;
        hash_table* new_weight_block;
        if (map_is_present(&self->weights_map, (void*)node->weight))
        {
            new_weight_block = map_get_item(&self->weights_map, (void*)node->weight);
        }
        else
        {
            new_weight_block = create_weight_block(self, node->weight);
        }
        hash_table_insert_item(new_weight_block, node);
        if (adaptive_node_get_type(node) == NODE_TYPE_INTERNAL)
        {
            check_node_children(node);
        }
        node = (adaptive_node*)node->parent;
    }
}

void adaptive_tree_update(adaptive_tree* self, uint8_t value)
{
    adaptive_node* node_to_update;
    if (map_is_present(&self->leaves_map, (void*)(uint64_t)value))
    {
        node_to_update = map_get_item(&self->leaves_map, (void*)(uint64_t)value);
    }
    else
    {
        node_to_update = adaptive_node_init(false, 0, NULL, NULL, NULL, value);
        adaptive_node* new_internal_node = (adaptive_node*)adaptive_node_init(false, 0, (adaptive_node*)self->nyt_node->parent, self->nyt_node, node_to_update, 0);
        node_to_update->parent = (struct adaptive_node*)new_internal_node;
        ((adaptive_node*)self->nyt_node->parent)->left = (struct adaptive_node*)new_internal_node;
        self->nyt_node->parent = (struct adaptive_node*)new_internal_node;
        map_set_item(&self->leaves_map, (void*)(uint64_t)value, node_to_update);
        if (!map_is_present(&self->weights_map, 0))
        {
            create_weight_block(self, 0);
        }
        hash_table* zero_block = map_get_item(&self->weights_map, 0);
        hash_table_insert_item(zero_block, (void*)node_to_update);
        hash_table_insert_item(zero_block, (void*)new_internal_node);
    }
    increase_weights(self, node_to_update);
}

uint8_t adaptive_tree_get_value(adaptive_tree* self, bit_buffer* path)
{
    adaptive_node* node = self->root;
    uint32_t buffer_walker = 0;
    while ((adaptive_node_get_type(node) != NODE_TYPE_LEAF) && (buffer_walker < bit_buffer_get_size(path)))
    {
        if (bit_buffer_get_bit(path, buffer_walker))
        {
            node = (adaptive_node*)node->right;
        }
        else
        {
            node = (adaptive_node*)node->left;
        }
        buffer_walker++;
    }
    return node->value;
}
