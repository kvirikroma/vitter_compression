%ifndef DYNAMIC_ARRAY_INCLUDE_INTERNAL
%define DYNAMIC_ARRAY_INCLUDE_INTERNAL


INIT_ALIGNMENT equ 16
INIT_LENGTH    equ 2 * INIT_ALIGNMENT
ITEM_SIZE      equ 8
LOG2_ITEM_SIZE equ 3

struc array
    ;structure of dynamic array
    .length resq 1    ;count of qword values
    .allocated resq 1 ;maximum qword values
    .data resq 0      ;offset of data (it is of course not zero, but some varying number)
endstruc


%endif
