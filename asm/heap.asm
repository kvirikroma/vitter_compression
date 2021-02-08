extern array_append_value
extern array_pop_value
extern array_get_by_index
extern array_get_size

global heap_get_top ;returns top value of heap
global heap_append  ;appends value to the heap
global heap_pop     ;pops top value from the heap
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

segment .text
    heap_get_top:
        ;param rdi - address of heap
        ;returns top value of heap
        xor esi, esi
        call array_get_by_index
        mov rax, [rax]
        ret

    heap_append:
        ;param rdi - address of heap
        ;param rsi - address of function that compares values
        ;param rdx - value to append
        ;appends value to heap
        push rbp
        mov rbp, rsp

        push rdi ;[rbp-8] - heap address
        push rsi ;[rbp-16] - function
        mov rsi, rdx
        call array_append_value
        mov rdi, [rbp-8]
        call array_get_size
        dec rax
        mov rdx, rax
        pop rsi
        pop rdi
        call flow_up

        leave
        ret
    
    heap_pop:
        ;param rdi - address of heap
        ;param rsi - address of function that compares values
        ;deletes and returns top value of heap
        push rbp
        mov rbp, rsp

        push rdi ;[rbp-8] - heap address
        push rsi ;[rbp-16] - function
        call array_get_size
        cmp rax, 0
        jne not_zero
            xor eax, eax
            dec rax
            leave
            ret
        not_zero:
        cmp rax, 1
        jne not_one
            mov rdi, [rbp-8]
            call array_pop_value
            leave
            ret
        not_one:
        mov rdi, [rbp-8]
        call array_pop_value
        push rax
        mov rdi, [rbp-8]
        xor esi, esi
        call array_get_by_index
        pop r10
        push qword [rax]
        mov [rax], r10
        mov rdi, [rbp-8]
        xor edx, edx
        mov rsi, [rbp-16]
        call flow_down

        pop rax
        leave
        ret
    
    flow_up:
        ;param rdi - address of heap
        ;param rsi - address of function that compares values
        ;param rdx - index of item to flow
        push rbp
        mov rbp, rsp

        push rdi ;[rbp-8] - heap address
        push rsi ;[rbp-16] - function
        push rdx ;[rbp-24] - index of current
        mov rsi, rdx
        call get_parent
        cmp rax, -1
            je fu_end
        push rax ;[rbp-32] - parent of current
        mov rsi, rax
        mov rdi, [rbp-8]
        call array_get_by_index
        push rax
        mov rsi, [rbp-24]
        mov rdi, [rbp-8]
        call array_get_by_index
        mov rsi, [rax]
        mov rdi, [rbp-40]
        mov rdi, [rdi]
        call [rbp-16]
        cmp rax, 1
        jne fu_not_greater
            mov rsi, [rbp-24]
            mov rdi, [rbp-8]
            call array_get_by_index
            mov r8, [rax]
            pop r10
            mov r9, [r10]
            mov [r10], r8
            mov [rax], r9
        fu_not_greater:
        pop rdx
        add rsp, 8
        pop rsi
        pop rdi
        call flow_up

        fu_end:
        leave
        ret
    
    flow_down:
        ;param rdi - address of heap
        ;param rsi - address of function that compares values
        ;param rdx - index of item to flow
        push rbp
        mov rbp, rsp

        push rdi ;[rbp-8] - heap address
        push rsi ;[rbp-16] - function
        push rdx ;[rbp-24] - index of current
        mov rsi, rdx
        call get_right
        push rax ;[rbp-32] - right of current
        mov rdi, [rbp-8]
        mov rsi, [rbp-24]
        call get_left
        push rax ;[rbp-40] - left of current
        cmp rax, -1
        je fd_not_both
        mov rax, [rbp-32]
        cmp rax, -1
        je fd_not_both
            mov rdi, [rbp-8]
            mov rsi, [rbp-40]
            call array_get_by_index
            push qword [rax]
            mov rdi, [rbp-8]
            mov rsi, [rbp-32]
            call array_get_by_index
            mov rdi, [rax]
            pop rsi
            call [rbp-16]
            cmp rax, 1
            jne fd_not_greater
                mov qword [rbp-32], -1
                jmp fd_not_both
            fd_not_greater:
                mov qword [rbp-40], -1
        fd_not_both:
        mov rax, [rbp-40]
        cmp rax, -1
        je fd_not_left
            call fd_cmp_internal
            mov rsi, [rbp-40]
            call fd_xchg_internal
            pop rdx
            add rsp, 16
            pop rsi
            pop rdi
            call flow_down
            jmp fd_not_right
        fd_not_left:
        mov rax, [rbp-32]
        cmp rax, -1
        je fd_not_right
            call fd_cmp_internal
            mov rsi, [rbp-32]
            call fd_xchg_internal
            add rsp, 8
            pop rdx
            add rsp, 8
            pop rsi
            pop rdi
            call flow_down
        fd_not_right:
        
        jmp fd_end
        fd_xchg_internal:
            mov rdi, [rbp-8]
            call array_get_by_index
            push rax
            mov rsi, [rbp-24]
            mov rdi, [rbp-8]
            call array_get_by_index
            mov r8, [rax]
            pop r10
            mov r9, [r10]
            mov [r10], r8
            mov [rax], r9
            ret
        fd_cmp_internal:
            mov rsi, rax
            mov rdi, [rbp-8]
            call array_get_by_index
            push qword [rax]
            mov rsi, [rbp-24]
            mov rdi, [rbp-8]
            call array_get_by_index
            mov rdi, [rax]
            pop rsi
            call [rbp-16]
            cmp rax, 1
            je fd_cmp_end
                add rsp, 8
                jmp fd_not_right
            fd_cmp_end:
            ret

        fd_end:
        leave
        ret
    
    get_parent:
        ;param rdi - address of heap
        ;param rsi - index of element
        ;returns index of parent item of one in rsi or -1 if it does not exist
        push rbp
        mov rbp, rsp

        push rsi
        call array_get_size
        xor r10d, r10d
        cmp rax, r10
        jg parent_normal
        parent_not_nornal:
            xor eax, eax
            dec rax
            leave
            ret
        parent_normal:
        pop rax
        cmp rax, r10
        jng parent_not_nornal
        dec rax
        shr rax, 1

        leave
        ret
    
    get_left:
        ;param rdi - address of heap
        ;param rsi - index of element
        ;returns index of item in left of one in rsi or -1 if it does not exist
        mov edx, 1
        call left_right_common
        ret
    
    get_right:
        ;param rdi - address of heap
        ;param rsi - index of element
        ;returns index of item in right of one in rsi or -1 if it does not exist
        mov edx, 2
        call left_right_common
        ret
    
    left_right_common:
        ;param rdi - address of heap
        ;param rsi - index of element
        ;param rdx - will be added to rsi
        push rbp
        mov rbp, rsp

        shl rsi, 1
        add rsi, rdx
        push rsi
        call array_get_size
        pop rsi
        dec rax
        cmp rsi, rax
        jg lrc_error
        cmp rsi, 0
        jl lrc_error
        jmp lrc_normal
        lrc_error:
            xor eax, eax
            dec rax
            leave
            ret
        lrc_normal:
        mov rax, rsi

        leave
        ret