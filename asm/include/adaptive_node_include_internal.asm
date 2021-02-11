%ifndef ADAPTIVE_NODE_INCLUDE_INTERNAL
%define ADAPTIVE_NODE_INCLUDE_INTERNAL


struc adaptive_node
    .parent resq 1
    .left   resq 1
    .right  resq 1
    .weight resq 1
    .value  resb 1
    .is_nyt resb 1
endstruc

ADAPTIVE_NODE_SIZE equ 34

NODE_TYPE_LEAF equ 0
NODE_TYPE_INTERNAL equ 1
NODE_TYPE_NYT equ 2


%endif
