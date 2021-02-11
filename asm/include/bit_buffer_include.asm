%ifndef BIT_BUFFER_INCLUDE
%define BIT_BUFFER_INCLUDE

struc bit_buffer
    data           resq 1
    last_item      resq 1
    last_item_size resq 1
endstruc

BIT_BUFFER_SIZE equ 24

extern bit_buffer_init
extern bit_buffer_delete
extern bit_buffer_add_bit
extern bit_buffer_get_bit
extern bit_buffer_get_size
extern bit_buffer_reverse
extern bit_buffer_get_full_part
extern bit_buffer_extend

%endif
