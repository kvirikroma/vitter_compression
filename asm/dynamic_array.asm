extern realloc
extern free
extern posix_memalign
extern exit

extern check_pointer_after_malloc

global array_init              ;*initializes dynamic array
global array_delete            ;deletes dynamic array
global array_delete_each       ;frees memory using each item as pointer
global array_append_value      ;*appends value to the end
global array_pop_value         ;returns value from the end and decreases length
global array_get_by_index      ;returns address of item or nullptr if it doesn't exist
global array_shrink_to_fit     ;*reallocates minimal memory size to contain data
global array_get_size          ;returns size of array in O(1) time
global array_extend            ;*extends array by the items of other array
global array_extend_from_mem   ;*extends array by the qwords from mem
global array_clear             ;zeroes array length
global array_to_usual          ;*casts the dynamic array to usual array in-place
;functions that marked with "*" return a pointer to allocated memory
;look function definition to see some details


INIT_ALIGNMENT equ 16
INIT_LENGTH    equ 2 * INIT_ALIGNMENT
ITEM_SIZE      equ 8
LOG2_ITEM_SIZE equ 3

struc dynamic
    ;structure of dynamic array
    .length resq 1    ;count of qword values
    .allocated resq 1 ;maximum qword values
    .data resq 0      ;offset of data (it is of course not zero, but some varying number)
endstruc


segment .rodata
    alloc_error db "An error occurred while allocating memory! Exiting", 10, 0
    alloc_error_len equ $-alloc_error

segment .text
    array_init:
        ;generates array with 'length' and 'allocated' in first 2 items
        ;'length' does not include first 2 elements but the 'allocated' does
        ;'length' is needed for counting items in the array
        ;(look for "struc dynamic" in this file to understand better)
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
        mov qword [rax+dynamic.length], 0
        mov qword [rax+dynamic.allocated], INIT_LENGTH

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
        mov rcx, [rdi+dynamic.length]
        cmp rcx, 0
        jz not_deleting
        
        deleting:
            mov rdi, rcx
            dec rdi
            shl rdi, LOG2_ITEM_SIZE
            add rdi, dynamic.data
            add rdi, [rsp]
            mov rdi, [rdi]
            push rcx
            call free
            pop rcx
        loop deleting
        not_deleting:

        pop rax
        mov qword [rax+dynamic.length], 0
        
        leave
        ret
    
    array_check_space:
        ;param rdi - address of array
        ;returns new address if reallocates (or old one if does not)
        push rbp
        mov rbp, rsp

        mov r8, [rdi+dynamic.length]
        mov r9, [rdi+dynamic.allocated]
        mov rsi, r9
        sub r9, 2
        sub r9, r8
        shl r9, 4
        cmp r9, rsi
        
        jg enough_space
            shl rsi, 1
            mov [rdi+dynamic.allocated], rsi
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
        add r8, dynamic.data
        mov r9, [rax+dynamic.length]
        mov r10, r9
        inc r10
        mov [rax+dynamic.length], r10
        shl r9, LOG2_ITEM_SIZE
        add r8, r9
        mov [r8], rsi

        leave
        ret
    
    array_get_by_index:
        ;param rdi - address of array
        ;param rsi - index
        ;returns address of item or nullptr if it doesn't exist
        mov rax, [rdi+dynamic.length]
        dec rax
        cmp rax, rsi
        jl return_nullptr_by_index
        cmp rsi, 0
        jl return_nullptr_by_index
            mov rax, rsi
            shl rax, LOG2_ITEM_SIZE
            add rax, rdi
            add rax, dynamic.data
            ret
        return_nullptr_by_index:
        xor eax, eax
        ret
    
    array_pop_value:
        ;param rdi - address of array
        ;deletes last item and returns its value
        push rbp
        mov rbp, rsp

        mov rdx, [rdi+dynamic.length]
        cmp rdx, 0
        jng nothing_to_pop
            dec rdx
            push rdx
            push rdi
            mov rsi, rdx
            call array_get_by_index
            pop rdi
            pop rdx
            mov [rdi+dynamic.length], rdx
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

        mov rsi, [rdi+dynamic.length]
        add rsi, 2
        mov [rdi+dynamic.allocated], rsi
        shl rsi, LOG2_ITEM_SIZE
        call realloc
        
        leave
        ret

    array_get_size:
        ;param rdi - address of array
        ;returns array size
        mov rax, [rdi+dynamic.length]
        ret
    
    array_extend:
        ;param rdi - address of array
        ;param rsi - address of other array
        ;returns address of array
        push rbp
        mov rbp, rsp

        mov rdx, [rsi+dynamic.length]
        add rsi, dynamic.data
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
        add rsi, [rdi+dynamic.allocated]

        shr rsi, 1
        jnc not_incrementing
            inc rsi
        not_incrementing:
        shl rsi, 1

        mov rax, rdi
        mov rcx, [rdi+dynamic.allocated]
        sub rcx, 2
        sub rcx, [rdi+dynamic.length]
        cmp rcx, rdx
        jnl not_reallocating
            mov [rdi+dynamic.allocated], rsi
            shl rsi, LOG2_ITEM_SIZE
            call realloc
        not_reallocating:

        pop rcx
        add rsp, 8
        pop rsi
        push rax
        mov rdi, rax

        mov r8, [rdi+dynamic.length]
        mov r9, r8
        add r8, rcx
        mov [rdi+dynamic.length], r8
        shl r9, LOG2_ITEM_SIZE

        add rdi, r9
        add rdi, dynamic.data
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
        mov qword [rdi+dynamic.length], 0
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
        add rsi, dynamic.data
        mov rcx, [rdi+dynamic.length]
        mov rax, rcx
        shl rax, LOG2_ITEM_SIZE
        mov qword [rax+rsi], 0
        inc rcx
        rep movsq
        pop rax

        leave
        ret
