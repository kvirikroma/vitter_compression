%ifndef DEQUE_INCLUDE
%define DEQUE_INCLUDE

%include "deque_include_internal.asm"


extern deque_init              ;initializes empty deque structure
extern deque_clear             ;clears the deque
extern deque_delete_each       ;frees memory using each item as pointer and clears the deque
extern deque_push_right        ;appends value to the end of deque
extern deque_push_left         ;appends value to the start of deque
extern deque_pop_right         ;deletes and returns value from the end of deque
extern deque_pop_left          ;deletes and returns value from the start of deque
extern deque_to_array          ;creates dynamic array basing on deque contents in O(n)
extern deque_extend_from_array ;extends deque from array
extern deque_merge             ;merges two deques to one in O(1), deletes second of them


%endif
