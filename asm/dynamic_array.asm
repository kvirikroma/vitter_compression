%include "dynamic_array_include_internal.asm"

extern realloc
extern free
extern posix_memalign
extern exit

extern check_pointer_after_malloc

global array_init
global array_delete
global array_delete_each
global array_append_value
global array_pop_value
global array_get_by_index
global array_shrink_to_fit
global array_extend
global array_extend_from_mem
global array_clear
global array_to_usual


segment .rodata
    alloc_error db "An error occurred while allocating memory! Exiting", 10, 0
    alloc_error_len equ $-alloc_error

segment .text
    array_init:
        ;generates array with 'length' and 'allocated' in first 2 items
        ;'length' does not include first 2 elements but the 'allocated' does
        ;'length' is needed for counting items in the array
        ;(look for "struc array" in this project to understand better)
        push rbp
        mov rbp, rsp

        sub rsp, 8
        mov rdi, rsp
        mov edx, INIT_LENGTH*ITEM_SIZE
        mov esi, INIT_ALIGNMENT
        call posix_memalign
        cmp eax, 0
        je mem_ok
            push rax
            mov rax, 1
            mov rdi, 0
            mov rsi, alloc_error
            mov rdx, alloc_error_len
            syscall
            pop rdi
            call exit
        mem_ok:
        pop rax
        mov qword [rax+array.length], 0
        mov qword [rax+array.allocated], INIT_LENGTH

        leave
        ret
    
    array_delete:
        ;param rdi - address of array
        push rbp
        mov rbp, rsp

        call free
        xor rax, rax

        leave
        ret
    
    array_delete_each:
        ;param rdi - address of array
        push rbp
        mov rbp, rsp

        push rdi
        mov rcx, [rdi+array.length]
        cmp rcx, 0
        jz not_deleting
        
        deleting:
            mov rdi, rcx
            dec rdi
            shl rdi, LOG2_ITEM_SIZE
            add rdi, array.data
            add rdi, [rsp]
            mov rdi, [rdi]
            push rcx
            call free
            pop rcx
        loop deleting
        not_deleting:

        pop rax
        mov qword [rax+array.length], 0
        
        leave
        ret
    
    array_check_space:
        ;param rdi - address of array
        ;returns new address if reallocates (or old one if does not)
        push rbp
        mov rbp, rsp

        mov r8, [rdi+array.length]
        mov r9, [rdi+array.allocated]
        mov rsi, r9
        sub r9, 2
        sub r9, r8
        shl r9, 4
        cmp r9, rsi
        
        jg enough_space
            shl rsi, 1
            mov [rdi+array.allocated], rsi
            shl rsi, LOG2_ITEM_SIZE
            call realloc
            
            mov rdi, rax
            call check_pointer_after_malloc

            jmp exit_check_space
        enough_space:
            mov rax, rdi
        exit_check_space:

        leave
        ret
    
    array_append_value:
        ;returns new address if reallocates
        ;param rdi - address of array
        ;param rsi - value
        push rbp
        mov rbp, rsp

        push rsi
        call array_check_space
        pop rsi
        mov r8, rax
        add r8, array.data
        mov r9, [rax+array.length]
        mov r10, r9
        inc r10
        mov [rax+array.length], r10
        shl r9, LOG2_ITEM_SIZE
        add r8, r9
        mov [r8], rsi

        leave
        ret
    
    array_get_by_index:
        ;param rdi - address of array
        ;param rsi - index
        ;returns address of item or nullptr if it doesn't exist
        mov rax, [rdi+array.length]
        dec rax
        cmp rax, rsi
        jl return_nullptr_by_index
        cmp rsi, 0
        jl return_nullptr_by_index
            mov rax, rsi
            shl rax, LOG2_ITEM_SIZE
            add rax, rdi
            add rax, array.data
            ret
        return_nullptr_by_index:
        xor eax, eax
        ret
    
    array_pop_value:
        ;param rdi - address of array
        ;deletes last item and returns its value
        push rbp
        mov rbp, rsp

        mov rdx, [rdi+array.length]
        cmp rdx, 0
        jng nothing_to_pop
            dec rdx
            push rdx
            push rdi
            mov rsi, rdx
            call array_get_by_index
            pop rdi
            pop rdx
            mov [rdi+array.length], rdx
            mov rax, [rax]
            jmp end_pop
        nothing_to_pop:
            xor eax, eax
        end_pop:

        leave
        ret
    
    array_shrink_to_fit:
        ;param rdi - address of array
        ;returns address of reallocated array
        push rbp
        mov rbp, rsp

        mov rsi, [rdi+array.length]
        add rsi, 2
        mov [rdi+array.allocated], rsi
        shl rsi, LOG2_ITEM_SIZE
        call realloc
        
        leave
        ret
    
    array_extend:
        ;param rdi - address of array
        ;param rsi - address of other array
        ;returns address of array
        push rbp
        mov rbp, rsp

        mov rdx, [rsi+array.length]
        add rsi, array.data
        call array_extend_from_mem
       
        leave
        ret

    array_extend_from_mem:
        ;param rdi - address of array
        ;param rsi - address from where to copy
        ;param rdx - count of qwords to copy
        ;returns address of array
        push rbp
        mov rbp, rsp

        cmp rdx, 0
        jng not_extending

        mov rcx, rdx
        push rsi
        push rdi
        push rdx
        mov rsi, rdx
        add rsi, [rdi+array.allocated]

        shr rsi, 1
        jnc not_incrementing
            inc rsi
        not_incrementing:
        shl rsi, 1

        mov rax, rdi
        mov rcx, [rdi+array.allocated]
        sub rcx, 2
        sub rcx, [rdi+array.length]
        cmp rcx, rdx
        jnl not_reallocating
            mov [rdi+array.allocated], rsi
            shl rsi, LOG2_ITEM_SIZE
            call realloc
        not_reallocating:

        pop rcx
        add rsp, 8
        pop rsi
        push rax
        mov rdi, rax

        mov r8, [rdi+array.length]
        mov r9, r8
        add r8, rcx
        mov [rdi+array.length], r8
        shl r9, LOG2_ITEM_SIZE

        add rdi, r9
        add rdi, array.data
        rep movsq
        pop rax
        jmp exit_extend

        not_extending:
            mov rax, rdi
        exit_extend:
        leave
        ret

    array_clear:
        ;param rdi - address of array
        mov qword [rdi+array.length], 0
        ret
    
    array_to_usual:
        ;param rdi - address of array
        ;casts the dynamic array to usual array (it's the same as your parameter)
        ;leaves some free space in the end (with some trash - be careful!)
        ;returns address of usual array
        push rbp
        mov rbp, rsp

        push rdi
        mov rsi, rdi
        add rsi, array.data
        mov rcx, [rdi+array.length]
        mov rax, rcx
        shl rax, LOG2_ITEM_SIZE
        mov qword [rax+rsi], 0
        inc rcx
        rep movsq
        pop rax

        leave
        ret
