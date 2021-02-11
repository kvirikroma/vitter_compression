#include <stdlib.h>

#include "include/adaptive_tree.h"
#include "include/utils.h"


static void remove_node_in_traversal(adaptive_node* node, void* params)
{
    adaptive_node_delete(node);
}

void adaptive_tree_init(adaptive_tree* self, uint16_t weight)
{
    self->nyt_node = adaptive_node_init(true, 0, NULL, NULL, NULL, weight);
    self->root = self->nyt_node;
}

void adaptive_tree_traversal(adaptive_tree* self, void(*node_handler)(adaptive_node*, void*), void* params, bit_buffer* current_path)
{
    if (!current_path)
    {
        current_path = check_pointer_after_malloc(malloc(sizeof(bit_buffer)));
        bit_buffer_init(current_path);
    }
}

void adaptive_tree_delete(adaptive_tree* self)
{
    adaptive_tree_traversal(self, remove_node_in_traversal, NULL, NULL);
    self->nyt_node = 0;
    self->root = 0;
}

void adaptive_tree_push(adaptive_tree* self, uint16_t value);
