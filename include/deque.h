#include <stdint.h>

#include "dynamic_array.h"


typedef struct
{
    void* data;
    struct deque_node* next;
    struct deque_node* prev;
 }
 deque_node;

typedef struct
{
    deque_node* first;
    deque_node* last;
    uint64_t len;
}
deque;


// initializes empty deque structure
extern void deque_init(deque*);

// clears the deque
extern void deque_clear(deque*);

// frees memory using each item as pointer and clears the deque
extern void deque_delete_each(deque*);

// appends value to the end of deque
extern void deque_push_right(deque*, uint64_t);

// appends value to the start of deque
extern void deque_push_left(deque*, uint64_t);

// deletes and returns value from the end of deque
extern uint64_t deque_pop_right(deque*);

// deletes and returns value from the start of deque
extern uint64_t deque_pop_left(deque*);

// creates dynamic array basing on deque contents in O(n)
extern dynamic_array* deque_to_array(deque*);

// extends deque from array
extern void deque_extend_from_array(deque*, dynamic_array*);

// merges two deques to one in O(1), deletes second of them
extern void deque_merge(deque*, deque*);
