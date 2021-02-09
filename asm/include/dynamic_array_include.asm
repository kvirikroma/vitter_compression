%ifndef DYNAMIC_ARRAY_INCLUDE
%define DYNAMIC_ARRAY_INCLUDE

%include "dynamic_array_include_internal.asm"


extern array_init              ;*initializes dynamic array
extern array_delete            ;deletes dynamic array
extern array_delete_each       ;frees memory using each item as pointer
extern array_append_value      ;*appends value to the end
extern array_pop_value         ;returns value from the end and decreases length
extern array_get_by_index      ;returns address of item or nullptr if it doesn't exist
extern array_shrink_to_fit     ;*reallocates minimal memory size to contain data
extern array_extend            ;*extends array by the items of other array
extern array_extend_from_mem   ;*extends array by the qwords from mem
extern array_clear             ;zeroes array length
extern array_to_usual          ;*casts the dynamic array to usual array in-place
;functions that marked with "*" return a pointer to allocated memory
;look function definition to see some details


%endif
