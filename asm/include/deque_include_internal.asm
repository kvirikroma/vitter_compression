%ifndef DEQUE_INCLUDE_INTERNAL
%define DEQUE_INCLUDE_INTERNAL


struc deque_node
    ;node of list-like deque
    ;each field is zero by default
    .data resq 1
    .next resq 1
    .prev resq 1
endstruc

struc deque
    ;list-like deque
    ;each field is zero by default
    .first resq 1
    .last  resq 1
    .len   resq 1
endstruc

DEQUE_NODE_SIZE equ 24
DEQUE_SIZE      equ 24


%endif
