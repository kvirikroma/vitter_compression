%include "dynamic_array_include.asm"
%include "deque_include_internal.asm"

extern malloc
extern free

global deque_init
global deque_delete
global deque_delete_each
global deque_push_right
global deque_push_left
global deque_pop_right
global deque_pop_left
global deque_to_array
global deque_extend_from_array
global deque_merge


segment .text
    create_deque_node:
        ;initializes and returns ptr to deque node
        push rbp
        mov rbp, rsp

        mov rdi, DEQUE_NODE_SIZE
        call malloc
        mov ecx, 3
        mov r8, rax
        mov rdi, rax
        xor rax, rax
        rep stosq
        mov rax, r8

        leave
        ret

    deque_init:
        ;param rdi - deque ptr
        ;initializes and returns ptr to empty deque structure
        mov qword [rdi+deque.len], 0
        mov qword [rdi+deque.first], 0
        mov qword [rdi+deque.last], 0
        ret

    deque_delete:
        ;param rdi - address of deque
        ;deletes deque and all its nodes
        push rbp
        mov rbp, rsp

        mov rcx, [rdi+deque.len]
        cmp rcx, 0
        je end_deleting
        deleting_nodes:
            push rdi
            call deque_pop_right
            pop rdi
            loop deleting_nodes
        end_deleting:
        xor eax, eax

        leave
        ret
    
    deque_delete_each:
        ;param rdi - address of deque
        ;frees memory using each item as pointer
        ;clears the deque
        push rbp
        mov rbp, rsp

        mov rcx, [rdi+deque.len]
        cmp rcx, 0
        je end_deleting_each
        deleting_each_node:
            push rdi
            call deque_pop_right
            mov rdi, rax
            call free
            pop rdi
            loop deleting_each_node
        end_deleting_each:
        mov [rdi+deque.len], qword 0
        xor rax, rax

        leave
        ret

    deque_push_right:
        ;param rdi - address of deque
        ;param rsi - value to append
        ;appends value to the end of deque
        ;returns count of items in deque
        push rbp
        mov rbp, rsp

        push rdi
        push rsi
        call create_deque_node
        pop qword [rax+deque_node.data]
        pop rdi
        mov r8, [rdi+deque.last]
        cmp r8, 0
        je right_empty
            mov [r8+deque_node.next], rax
            mov [rax+deque_node.prev], r8
            mov [rdi+deque.last], rax
            jmp end_pushing_right
        right_empty:
            mov [rdi+deque.last], rax
            mov [rdi+deque.first], rax
        end_pushing_right:
        mov rax, [rdi+deque.len]
        inc rax
        mov [rdi+deque.len], rax

        leave
        ret

    deque_push_left:
        ;param rdi - address of deque
        ;param rsi - value to append
        ;appends value to the start of deque
        ;returns count of items in deque
        push rbp
        mov rbp, rsp

        push rdi
        push rsi
        call create_deque_node
        pop qword [rax+deque_node.data]
        pop rdi
        mov r8, [rdi+deque.first]
        cmp r8, 0
        je left_empty
            mov [r8+deque_node.prev], rax
            mov [rax+deque_node.next], r8
            mov [rdi+deque.first], rax
            jmp end_pushing_left
        left_empty:
            mov [rdi+deque.last], rax
            mov [rdi+deque.first], rax
        end_pushing_left:
        mov rax, [rdi+deque.len]
        inc rax
        mov [rdi+deque.len], rax

        leave
        ret

    deque_pop_right:
        ;param rdi - address of deque
        ;pops and returns value from the end of deque
        push rbp
        mov rbp, rsp

        mov rax, [rdi+deque.last]
        cmp rax, 0
        je end_right_pop
            cmp rax, [rdi+deque.first]
            jne not_one_right
                mov qword [rdi+deque.first], 0
                push qword [rax+deque_node.data]
                mov qword [rdi+deque.last], 0
                jmp end_not_one_right
            not_one_right:
                mov r8, [rax+deque_node.prev]
                mov qword [r8+deque_node.next], 0
                push qword [rax+deque_node.data]
                mov [rdi+deque.last], r8
            end_not_one_right:
            push rdi
            mov rdi, rax
            call free
            pop rdi
            mov rax, [rdi+deque.len]
            dec rax
            mov [rdi+deque.len], rax
            pop rax
        end_right_pop:

        leave
        ret

    deque_pop_left:
        ;param rdi - address of deque
        ;pops and returns value from the start of deque
        push rbp
        mov rbp, rsp

        mov rax, [rdi+deque.first]
        cmp rax, 0
        je end_left_pop
            cmp rax, [rdi+deque.last]
            jne not_one_left
                mov qword [rdi+deque.last], 0
                push qword [rax+deque_node.data]
                mov qword [rdi+deque.first], 0
                jmp end_not_one_left
            not_one_left:
                mov r8, [rax+deque_node.next]
                mov qword [r8+deque_node.prev], 0
                push qword [rax+deque_node.data]
                mov [rdi+deque.first], r8
            end_not_one_left:
            push rdi
            mov rdi, rax
            call free
            pop rdi
            mov rax, [rdi+deque.len]
            dec rax
            mov [rdi+deque.len], rax
            pop rax
        end_left_pop:

        leave
        ret
    
    deque_to_array:
        ;param rdi - address of deque
        ;returns pointer to newly created array
        push rbp
        mov rbp, rsp

        push rdi
        call array_init
        push rax
        mov rdi, [rbp-8]
        mov rdx, [rdi+deque.first]

        appending:
            cmp rdx, 0
            je end_appending
            mov rdi, [rbp-16]
            mov rsi, [rdx+deque_node.data]
            push rdx
            call array_append_value
            pop rdx
            mov [rbp-16], rax
            mov rdx, [rdx+deque_node.next]
            jmp appending
        end_appending:
        mov rax, [rbp-16]

        leave
        ret

    deque_extend_from_array:
        ;param rdi - address of deque
        ;param rsi - address of array
        ;returns count of items in deque or 0 if array is empty
        push rbp
        mov rbp, rsp
        
        push rdi
        push rsi
        mov rdi, rsi
        call array_get_size
        cmp rax, 0
        jng extend_from_empty
            mov rcx, rax
            push rax
            extending:
                push rcx
                mov rsi, rcx
                sub rsi, [rbp-24]
                neg rsi
                mov rdi, [rbp-16]
                call array_get_by_index
                mov rsi, [rax]
                mov rdi, [rbp-8]
                call deque_push_right
                pop rcx
                loop extending
        extend_from_empty:

        leave
        ret

    deque_merge:
        ;param rdi - address of deque
        ;param rsi - address of other deque (will be deleted)
        ;deque in rsi will be merged to the end of deque in rdi
        push rbp
        mov rbp, rsp

        mov rax, [rdi+deque.len]
        add rax, [rsi+deque.len]
        mov [rdi+deque.len], rax

        mov rax, [rdi+deque.last]
        mov rdx, [rsi+deque.last]
        mov rcx, [rsi+deque.first]
        
        cmp rax, 0
        jne first_not_empty
            mov [rdi+deque.first], rcx
        first_not_empty:
        cmp rdx, 0
        je second_empty
            mov [rdi+deque.last], rdx
            mov [rcx+deque_node.prev], rax
            cmp rax, 0
            je first_empty
                mov [rax+deque_node.next], rcx
            first_empty:
            push rdi
            mov rdi, rsi
            call free
            pop rax
        second_empty:
        
        leave
        ret
