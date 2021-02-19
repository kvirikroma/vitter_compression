#include <stdlib.h>
#include <string.h>
#include <stdio.h>

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

static uint64_t node_hash_function(const adaptive_node* node)
{
    return (uint64_t)node;
}

static bool node_comparison_function(const adaptive_node* node, const adaptive_node* other_node)
{
    return node == other_node;
}

static void remove_hash_tables(hash_table* table, void* params)
{
    hash_table_delete(table);
}

// Return true if the path is righter then the other one
static bool is_path_righter(bit_buffer* path, bit_buffer* other_path)
{
    volatile uint32_t min_path_size = min(2, bit_buffer_get_size(path), bit_buffer_get_size(other_path));
    for (uint32_t i = 0; i < min_path_size; i++)
    {
        volatile uint8_t path_bit = (uint8_t)bit_buffer_get_bit(path, i);
        volatile uint8_t other_path_bit = (uint8_t)bit_buffer_get_bit(other_path, i);
        if (path_bit < other_path_bit)
        {
            return false;
        }
        if (path_bit > other_path_bit)
        {
            return true;
        }
    }
    return false;
}

typedef struct fhnias_params
{
    adaptive_node* node;
    adaptive_node* highest_so_far;
    bit_buffer node_path_storage;
    bit_buffer current_node_path_storage;
    bit_buffer highest_node_path_storage;
}
fhnias_params;

static void find_highest_node_in_another_subtree(adaptive_node* current_node, fhnias_params* params)
{
    if (params->highest_so_far == NULL)
    {
        params->highest_so_far = current_node;
    }
    bit_buffer_clear(&params->current_node_path_storage);
    bit_buffer_clear(&params->highest_node_path_storage);
    adaptive_node_get_path(current_node, &params->current_node_path_storage);
    adaptive_node_get_path(params->highest_so_far, &params->highest_node_path_storage);
    uint32_t current_node_path_size = bit_buffer_get_size(&params->current_node_path_storage);
    uint32_t highest_node_path_size = bit_buffer_get_size(&params->highest_node_path_storage);
    if (
            (current_node_path_size < highest_node_path_size) ||
            ((current_node_path_size == highest_node_path_size) && is_path_righter(&params->current_node_path_storage, &params->highest_node_path_storage))
    )
    {
        if (! bit_buffer_starts_with(&params->node_path_storage, &params->current_node_path_storage))
        {
            params->highest_so_far = current_node;
        }
    }
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
    map_init(
        &self->weights_map,
        (uint64_t(*)(const void*))weight_hash_function,
        (bool(*)(const void*, const void*))weight_comparison_function
    );
    map_init(
        &self->leaves_map,
        (uint64_t(*)(const void*))leaf_hash_function,
        (bool(*)(const void*, const void*))leaf_comparison_function
    );
    self->logs_file = NULL; //fopen("/dev/tty", "w");
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
            bit_buffer_push_bit(current_path, 0);  // path for the next node
            adaptive_tree_traversal(self, node_handler, params, current_path, (adaptive_node*)current_node->left);
        }
        if (current_node->right)
        {
            bit_buffer_push_bit(current_path, 1);  // path for the next node
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
    if (self->logs_file)
    {
        fclose(self->logs_file);
    }
    self->logs_file = 0;
    map_iterate_values(&self->weights_map, (void(*)(void*, void*))remove_hash_tables, NULL);
    adaptive_tree_traversal(self, remove_node_in_traversal, self, NULL, NULL);
    map_delete(&self->leaves_map);
    map_delete(&self->weights_map);
    self->nyt_node = 0;
    self->root = 0;
}

static void exchange_nodes(adaptive_node* node, adaptive_node* other_node)
{
    if ((node->parent == other_node) || (other_node->parent == node))
    {
        return;
    }
    adaptive_node* old_node_parent_left = node->parent->left;
    adaptive_node* old_other_parent_left = other_node->parent->left;
    if (node->parent)
    {
        if (old_node_parent_left == node)
        {
            node->parent->left = other_node;
        }
        else
        {
            node->parent->right = other_node;
        }
    }
    if (other_node->parent)
    {
        if (old_other_parent_left == other_node)
        {
            other_node->parent->left = node;
        }
        else
        {
            other_node->parent->right = node;
        }
    }
    adaptive_node* tmp = other_node->parent;
    other_node->parent = node->parent;
    node->parent = tmp;
}

 void check_node_children(adaptive_node* node)
{
    if (node->left && node->right)
    {
        uint64_t left_weight = ((adaptive_node*)node->left)->weight;
        uint64_t right_weight = ((adaptive_node*)node->right)->weight;
        if (
                (left_weight > right_weight) || (
                    (left_weight == right_weight) && (
                        (adaptive_node_get_type((adaptive_node*)node->right) != NODE_TYPE_INTERNAL) &&
                        (adaptive_node_get_type((adaptive_node*)node->left) == NODE_TYPE_INTERNAL)
                    )
                )
        )
        {
            exchange_nodes((adaptive_node*)node->left, (adaptive_node*)node->right);
        }
    }
}

static hash_table* create_weight_block(adaptive_tree* self, uint64_t weight)
{
    hash_table* new_table = check_pointer_after_malloc(malloc(sizeof(hash_table)));
    hash_table_init(
        new_table, BLOCK_DEFAULT_ACCURACY,
        (uint64_t(*)(const void*))node_hash_function,
        (bool(*)(const void*, const void*))node_comparison_function
    );
    map_set_item(&self->weights_map, (const void*)weight, new_table);
    return new_table;
}

static void delete_weight_block(adaptive_tree* self, uint64_t weight)
{
    hash_table* table_to_delete = map_get_item(&self->weights_map, (const void*)weight);
    if (table_to_delete)
    {
        hash_table_delete(table_to_delete);
        free(table_to_delete);
    }
    map_remove_item(&self->weights_map, (const void*)weight);
}

static void increase_weights(adaptive_tree* self, adaptive_node* node)
{
    fhnias_params iteration_params;
    bit_buffer_init(&iteration_params.node_path_storage);
    bit_buffer_init(&iteration_params.current_node_path_storage);
    bit_buffer_init(&iteration_params.highest_node_path_storage);
    while (node)
    {
        bit_buffer_clear(&iteration_params.node_path_storage);
        adaptive_node_get_path(node, &iteration_params.node_path_storage);
        iteration_params.node = node;
        iteration_params.highest_so_far = node;
        hash_table_iterate(
            map_get_item(&self->weights_map, (const void*)node->weight),
            (void(*)(void*, void*))find_highest_node_in_another_subtree,
            &iteration_params
        );
        if (iteration_params.highest_so_far != node)
        {
            exchange_nodes(iteration_params.highest_so_far, node);
            check_node_children(node->parent);
            check_node_children(iteration_params.highest_so_far->parent);
        }
        hash_table* old_weights_block = map_get_item(&self->weights_map, (const void*)node->weight);
        hash_table_remove_item(old_weights_block, node);
        if (old_weights_block->items_count == 0)
        {
            delete_weight_block(self, node->weight);
        }
        node->weight++;
        hash_table* new_weight_block;
        if (map_is_present(&self->weights_map, (const void*)node->weight))
        {
            new_weight_block = map_get_item(&self->weights_map, (const void*)node->weight);
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
    bit_buffer_delete(&iteration_params.node_path_storage);
    bit_buffer_delete(&iteration_params.current_node_path_storage);
    bit_buffer_delete(&iteration_params.highest_node_path_storage);
}

static void print_node(const adaptive_node* node, FILE* logs_file)
{
    switch (adaptive_node_get_type(node))
    {
        case NODE_TYPE_LEAF:
        {
            fprintf(logs_file, "[%c|%lu] ", (uint32_t)node->value, node->weight);
            break;
        }
        case NODE_TYPE_INTERNAL:
        {
            fprintf(logs_file, "(%lu) ", node->weight);
            break;
        }
        case NODE_TYPE_NYT:
        {
            fprintf(logs_file, "[NYT] ");
            break;
        }
    }
}

static void print_node_verbose(const adaptive_node* node, FILE* logs_file)
{
    const char* node_type;
    switch (adaptive_node_get_type(node))
    {
        case NODE_TYPE_NYT:
        {
            node_type = "NODE_TYPE_NYT";
            break;
        }
        case NODE_TYPE_INTERNAL:
        {
            node_type = "NODE_TYPE_INTERNAL";
            break;
        }
        case NODE_TYPE_LEAF:
        {
            node_type = "NODE_TYPE_LEAF";
            break;
        }
        default:
        {
            break;
        }
    }
    fprintf(
        logs_file, "Node(address=%p, type=%s, value=%u, weight=%lu, parent=%p, left=%p, right=%p)",
        node, node_type, node->value, node->weight, node->parent, node->left, node->right
    );
}

void print_tree(const adaptive_tree* self)
{
    deque node_container;
    deque all_nodes;
    deque_init(&all_nodes);
    bit_buffer path_container;
    uint32_t max_depth = 0;
    bit_buffer_init(&path_container);
    deque_init(&node_container);

    deque_push_right(&node_container, (uint64_t)self->root);
    deque_push_right(&all_nodes, (uint64_t)self->root);

    while (node_container.len)
    {
        adaptive_node* node = (adaptive_node*)deque_pop_left(&node_container);
        adaptive_node_get_path(node, &path_container);
        uint32_t node_depth = bit_buffer_get_size(&path_container);
        if (max_depth < node_depth)
        {
            max_depth = node_depth;
            fprintf(self->logs_file, "\n");
        }
        bit_buffer_clear(&path_container);
        print_node(node, self->logs_file);
        if (node->left)
        {
            deque_push_right(&node_container, (uint64_t)node->left);
            deque_push_right(&all_nodes, (uint64_t)node->left);
        }
        if (node->right)
        {
            deque_push_right(&node_container, (uint64_t)node->right);
            deque_push_right(&all_nodes, (uint64_t)node->right);
        }
    }
    
    bit_buffer_delete(&path_container);
    fprintf(self->logs_file, "\n\nAll nodes:\n");
    while (all_nodes.len)
    {
        print_node_verbose((adaptive_node*)deque_pop_left(&all_nodes), self->logs_file);
        if (all_nodes.len >= 1)
        {
            fprintf(self->logs_file, ",\n");
        }
    }
    fprintf(self->logs_file, "\n\n\n");
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
        adaptive_node* new_internal_node = (adaptive_node*)adaptive_node_init(
            false, 0, (adaptive_node*)self->nyt_node->parent, self->nyt_node, node_to_update, 0
        );
        node_to_update->parent = (struct adaptive_node*)new_internal_node;
        if (self->nyt_node->parent)
        {
            ((adaptive_node*)self->nyt_node->parent)->left = (struct adaptive_node*)new_internal_node;
        }
        else
        {
            self->root = new_internal_node;
        }
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
    if (self->logs_file)
    {
        fprintf(self->logs_file, "Adding a '%c' symbol\n", value);
        print_tree(self);
    }
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
