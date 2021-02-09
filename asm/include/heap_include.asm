%ifndef HEAP_INCLUDE
%define HEAP_INCLUDE

%include "heap_include_internal.asm"

extern heap_get_top ;returns top value of heap
extern heap_push    ;appends value to the heap
extern heap_pop     ;pops top value from the heap
global heap_init    ;initializes the heap instance
global heap_delete  ;frees a memory used for the heap
;look function definition to see some details

;This heap is just basically a set of functions
;that wraps around the dynamic array structure.

;Some functions take parameter with function address.
;Requirements to that function in parameter (for min-heap):
;1. Takes first item in rdi and second in rsi
;2. Returns 0 if they are equal.
;3. Returns -1 if parameter in rdi is less than one in rsi.
;4. Returns 1 if parameter in rdi is greater than one in rsi.
;5. You can change theese values to the opposite ones to get a max-heap

%endif
