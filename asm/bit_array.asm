global bit_array_get_bit
global bit_array_set_bit


segment .text
    bit_array_get_bit:
        ; get value of bit by number
        ; param rdi - address of field
        ; param rsi - number of bit to return
        mov rax, rsi
        shr rax, 3
        mov cl, 7  ; it's also 111b
        and sil, cl
        mov al, [rdi + rax]
        mov cl, sil
        shr al, cl
        and eax, 1
        ret

    bit_array_set_bit:  ;WORKS IN DIFFERENT ORDER
        ; set value of bit by number
        ; param rdi - address of field
        ; param rsi - number of bit to set
        ; param rdx - value (non-0 values will be accepted as 1)
        mov rax, rsi
        shr rax, 3
        and sil, 111b
        mov r8b, 80h
        mov cl, sil
        shr r8b, cl

        cmp rdx, 0
        je bfsb_zero
            or [rdi + rax], r8b
            jmp bfsb_end
        bfsb_zero:
            not r8b
            and [rdi + rax], r8b
        bfsb_end:

        ret
