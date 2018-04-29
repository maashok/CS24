#
# Keep a pointer to the main scheduler context.
# This variable should be initialized to %esp;
# which is done in the __sthread_start routine.
#
     .data
     .align 4
scheduler_context:      .long   0

#
# __sthread_schedule is the main entry point for the thread
# scheduler.  It has three parts:
#
#    1. Save the context of the current thread on the stack.
#       The context includes only the integer registers
#       and EFLAGS.
#    2. Call __sthread_scheduler (the C scheduler function),
#       passing the context as an argument.  The scheduler
#       stack *must* be restored by setting %esp to the
#       scheduler_context before __sthread_scheduler is
#       called.
#    3. __sthread_scheduler will return the context of
#       a new thread.  Restore the context, and return
#       to the thread.
#
     .text
     .align 4
     .globl __sthread_schedule
__sthread_schedule:

     # Save the context - all integer registers and the flags
     pusha
     pushfl

     # Call the high-level scheduler with the current
     # context as an argument
     movl    %esp, %eax
     movl    scheduler_context, %esp
     pushl   %eax

     call    __sthread_scheduler

     # The scheduler will return a context to start.
     # Restore the context to resume the thread

__sthread_restore:

     # Move the context to start as the stack pointer
     movl    %eax, %esp
     # Move the integer registers and flags from this
     # context into the registers, to be used
     popfl
     popa

     ret

#
# Initialize a process context, given:
#    1. the stack for the process   8(%ebp)
#    2. the function to start      12(%ebp)
#    3. its argument               16(%ebp)
# The context should be consistent with the context
# saved in the __sthread_schedule routine.
#
# A pointer to the newly initialized context is returned to the caller.
# (This is the stack-pointer after the context has been set up.)
#
# This function is described in more detail in sthread.c.
#
#
     .globl __sthread_initialize_context
__sthread_initialize_context:

     # Set up the frame pointer and save caller ebp
     pushl  %ebp
     movl   %esp, %ebp

     # Move the stack of the thread to the stack pointer
     movl    8(%ebp), %esp
     # Push the argument to the function on the stack
     pushl   16(%ebp)
     # Push the function to return to after completing
     # the thread function
     pushl   $__sthread_finish
     # Push the thread function pointer as the return
     # value from this function
     pushl   12(%ebp)

     # Push placeholders for all the registers to be saved
     # in the context
     pushl   $0
     pushl   $0
     pushl   $0
     pushl   $0
     pushl   $0
     pushl   $0
     pushl   $0
     pushl   $0
     pushfl

     # Save the stack pointer as the return value and
     # remove local variables from stack
     movl    %esp, %eax
     movl    %ebp, %esp
     pop     %ebp
     
     ret

#
# The start routine initializes the scheduler_context variable,
# and calls the __sthread_scheduler with a NULL context.
# The scheduler will return a context to resume.
#
     .globl __sthread_start
__sthread_start:
     # Remember the context
     movl    %esp, scheduler_context

     # Call the scheduler with no context
     pushl   $0
     call    __sthread_scheduler

     # Restore the context returned by the scheduler
     jmp     __sthread_restore
