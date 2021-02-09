%ifndef HEAP_INCLUDE_INTERNAL
%define HEAP_INCLUDE_INTERNAL

struc heap
    .data        resq 1
    .comparator  resq 1
endstruc

HEAP_SIZE equ 8

%endif
