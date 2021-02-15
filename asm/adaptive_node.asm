%include "adaptive_node_include_internal.asm"
%include "bit_buffer_include.asm"

extern malloc
extern free

extern check_pointer_after_malloc

global adaptive_node_init
global adaptive_node_delete
global adaptive_node_exchange
global adaptive_node_get_type
global adaptive_node_get_path


segment .text
    adaptive_node_init:
        ;param rdi - is_nyt
        ;param rsi - weight
        ;others (rdx-r9) may be null if 1st is true
        ;param rdx - parent
        ;param rcx - left
        ;param r8  - right
        ;param r9  - value
        ;returns node pointer
        push rbp
        mov rbp, rsp

        push rdi  ; [rbp-8] - is_nyt
        push rsi  ; [rbp-16] - weight
        cmp rdi, 0
        je allocating
            push rdx  ; [rbp-24] - parent
            push rcx  ; [rbp-32] - left
            push r8   ; [rbp-40] - right
            push r9   ; [rbp-48] - value
        allocating:
        mov rdi, ADAPTIVE_NODE_SIZE
        call malloc
        mov rdi, rax
        call check_pointer_after_malloc
        mov r10, [rbp-16]
        mov [rax+adaptive_node.weight], r10
        mov rdi, [rbp-8]
        cmp rdi, 0
        jne setting_values
            mov byte [rax+adaptive_node.is_nyt], 1
            mov byte [rax+adaptive_node.value], 0
            mov qword [rax+adaptive_node.parent], 0
            mov qword [rax+adaptive_node.left], 0
            mov qword [rax+adaptive_node.right], 0
            jmp ani_end
        setting_values:
            mov byte [rax+adaptive_node.is_nyt], 0
            pop r9
            pop r8
            pop rcx
            pop rdx
            mov [rax+adaptive_node.value], r9b
            mov qword [rax+adaptive_node.parent], rdx
            mov qword [rax+adaptive_node.left], rcx
            mov qword [rax+adaptive_node.right], r8
        ani_end:

        leave
        ret

    adaptive_node_delete:
        ;param rdi - node
        call free
        ret

    adaptive_node_exchange:
        ;param rdi - node
        ;param rsi - other node
        mov rax, [rdi+adaptive_node.parent]
        mov rcx, [rsi+adaptive_node.parent]
        mov [rdi+adaptive_node.parent], rcx
        mov [rsi+adaptive_node.parent], rax
        ret

    adaptive_node_get_type:
        ;param rdi - node
        ;returns its type:
        ;NODE_TYPE_LEAF == 0
        ;NODE_TYPE_INTERNAL == 1
        ;NODE_TYPE_NYT == 2
        cmp byte [rdi+adaptive_node.is_nyt], 1
        jne not_nyt
            mov eax, NODE_TYPE_NYT
            ret
        not_nyt:
            mov rax, [rdi+adaptive_node.left]
            or rax, [rdi+adaptive_node.right]
            jz angt_ret_leaf
                mov eax, NODE_TYPE_INTERNAL
                ret
            angt_ret_leaf:
                mov eax, NODE_TYPE_LEAF
                ret

    adaptive_node_get_path:
        ;param rdi - node
        ;param rsi - path to complete (only for the recursion), should be a nullprt (or empty one) on 1st call
        ;returns bit_buffer with path from root to the node
        push rbp
        mov rbp, rsp
        
        push rdi  ; [rbp-8]  - node
        push rsi  ; [rbp-16] - path
        cmp qword [rbp-16], 0
        jne path_storage_created
            mov rdi, BIT_BUFFER_SIZE
            call malloc
            mov rdi, rax
            call check_pointer_after_malloc
            mov rdi, rax
            mov [rbp-16], rax
            call bit_buffer_init
        path_storage_created:
        mov rdi, [rbp-8]
        cmp qword [rdi+adaptive_node.parent], 0
        jne angp_not_root_yet
            mov rdi, [rbp-16]
            call bit_buffer_reverse
            jmp angp_end
        angp_not_root_yet:
        mov rax, [rdi+adaptive_node.parent]
        cmp [rax+adaptive_node.left], rdi
        je current_is_left
            mov rdi, [rbp-16]
            mov rsi, 1
            call bit_buffer_push_bit
            jmp continue_recursion
        current_is_left:
            mov rdi, [rbp-16]
            mov rsi, 0
            call bit_buffer_push_bit
        continue_recursion:
            mov rdi, [rbp-8]
            mov rsi, [rbp-16]
            call adaptive_node_get_path

        angp_end:
        mov rax, [rbp-16]
        leave
        ret

