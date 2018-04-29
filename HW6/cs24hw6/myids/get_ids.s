.globl getID


# void getID(int *uid, int *gid);

getID:
        # 8(%ebp) = pointer to integer to receive user ID
        # 12(%ebp) = pointer to integer to receive group ID

        pushl   %ebp              # Save caller ebp and move to current
        movl    %esp, %ebp        # stackframe

        movl    $24, %eax         # Move the operation getuid to %eax
        int     $0x80             # Make the system call
        movl    8(%ebp), %edx     # Move the pointer to where the uid should
                                  # go to %edx
        movl    %eax, (%edx)      # Move the uid to the specified address

        movl    $47, %eax         # Move the operation getgid to %eax
        int     $0x80             # Make the system call
        movl    12(%ebp), %ecx    # Move the pointer to where the gid should
                                  # go to %ecx
        movl    %eax, (%ecx)      # Move the gid to the specified address

        pop     %ebp              # Restore the base pointer and return
        ret
