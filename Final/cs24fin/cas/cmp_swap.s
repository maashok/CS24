.global compare_and_swap


# Compares the value at address target to old and
# if they are the same, stores new into that address
# and returns 1. Otherwise, doesn't modify target and returns 0

# int compare_and_swap(uint32_t *target, uint32_t old, uint32_t new);

# Arguments:
# 8(%ebp) = address of target
# 12(%ebp) = old
# 16(%ebp) = new

# Return value = 0 or 1 in %eax

compare_and_swap:
      pushl       %ebp             # Save the caller ebp and move to 
      movl        %esp, %ebp       # current stack frame

      movl        8(%ebp), %ecx    # Move the address of target into %ecx
      movl        12(%ebp), %eax   # Save the old and new values in registers
      movl        16(%ebp), %edx

      lock
      cmpxchgl    %edx, (%ecx)     # If target equals old value, set zero flag
                                   # and loads new value into value at target
                                   # pointer
      movl        $0, %eax
      movl        $1, %ecx
      cmove       %ecx, %eax       # If zero flag was set, return value is 1,
                                   # otherwise it is zero

      pop         %ebp             # Restore base pointer
      ret

