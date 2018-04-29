.global reverse_list

# void reverse_list(LinkedList *list)

# This function reverses the list which
# it is passed as a pointer to it.
# 8(%ebp) = pointer to list
# No return value

# Stack Setup

# 8(%ebp) contains address of linked list, x
# M[x] contains the pointer to the head, y
# M[x + 4] contains the pointer to the tail, z

# M[y] contains the value in the head
# M[y + 4] contains the pointer to the next node
# M[z] contains the value in the tail
# M[z + 4] contains the value in the next node (NULL)
       
reverse_list:
       pushl   %ebp                    # Save caller ebp
       movl    %esp, %ebp              # Set up stack frame pointer

       pushl   %ebx                    # Save callee save registers
       pushl   %esi

       movl    8(%ebp), %eax           # Save pointer to list in %eax
       movl    (%eax), %ecx            # Save pointer to head in %ecx
       movl    4(%eax), %edx           # Save pointer to tail in %edx

       movl    $0x0, %ebx              
       
       cmpl    %ecx, %ebx              # See if head is NULL or if tail is
       je      Return                  # NULL. If either is, return

       cmpl    %edx, %ebx       
       je      Return
       
       movl    %ecx, %eax              # Set up for reversing, putting the
       movl    4(%eax), %ebx           # pointer to the first element in %eax
                                       # and the second in %ebx
       movl    $0x0, 4(%ecx)           # Set the next of the original head
                                       # to NULL since it will be the tail

Loop:                                  # Consider 2 adjacent elements
       movl    4(%ebx), %esi           # Save the element after the second
       movl    %eax, 4(%ebx)           # Make the first element come after
                                       # the second

       movl    %ebx, %eax              # Then, slide down the list to the 
       movl    %esi, %ebx              # current second and the one after that

       test    %ebx, %edx              # If the new group doesn't include the
       jne     Loop                    # last element iterate again
       
Done:
       movl    8(%ebp), %eax              

       movl    %edx, (%eax)            # Swap the head and the tail
       movl    %ecx, 4(%eax)       
Return:       

       popl    %esi                    # Restore callee save registers
       popl    %ebx
       
       movl    %ebp, %esp              # Clean up the stack before returning
       popl    %ebp
       ret
