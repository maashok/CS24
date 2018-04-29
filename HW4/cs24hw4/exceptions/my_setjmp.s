.global my_setjmp
.global my_longjmp

my_setjmp:
    # 8(%ebp) = address of jmp_buf where the registers and other important
    # states will be stored

    # Return value in %eax is 0
    
    push    %ebp             # Save caller ebp and move to current stackframe
    movl    %esp, %ebp    
    movl    8(%ebp), %edx    # Save the address of the jump buf in %edx
    movl    %esp, (%edx)     # Move the stack pointer to jump buffer
    movl    (%ebp), %eax     # Move caller ebp to jump buffer through %eax
    movl    %eax, 4(%edx)
    movl    %ebx, 8(%edx)    # Move %ebx to jump buffer
    movl    %esi, 12(%edx)   # Move %esi to jump buffer
    movl    %edi, 16(%edx)   # Move %edi to jump buffer
    movl    4(%ebp), %eax    # Move return address to %edx then jump buffer
    movl    %eax, 20(%edx)
    movl    $0, %eax         # Return 0 always for setjmp

    pop     %ebp

    ret

my_longjmp:
        # 8(%ebp) = address of jmp_buf where the registers and other important
    # states have been stored from set_jmp
    # 12(%ebp) = value to return

    # Return value in %eax is 1 if value to return is 0, or
    # the value to return itself

    push    %ebp             # Save caller ebp and move to current stackframe
    movl    %esp, %ebp
    movl    8(%ebp), %edx    # Save the address of the jump buf in %edx
    movl    (%edx), %esp     # Move the stack pointer back to old position
    movl    8(%edx), %ebx    # Change %ebx back to original value
    movl    12(%edx), %esi   # Change %esi back to original value
    movl    16(%edx), %edi   # Change %edi back to original value
    movl    20(%edx), %eax   # Change return address back to setjmp's 
    movl    %eax, 4(%esp)    # return address, moving it through %eax
    movl    12(%ebp), %eax   # Move ret to %eax to return it
    cmpl    $0, %eax         # Check if it is equal to 0
    jne     return    
    movl    $1, %eax         # If it is, change the return value to 1
    
return:    
    movl    4(%edx), %ecx    # Finally, change the caller ebp to the
    movl    %ecx, (%esp)     # original one from setjmp

    popl    %ebp             # Move the old caller ebp into the %ebp register
    
    ret

