.globl gcd			/* Ignore everything beginning with a "." */

gcd:
	# arg1 = 8(%ebp)
	# arg2 = 12(8%ebp)
	pushl %ebp		/* Push old frame pointer */
	movl %esp, %ebp		/* The new frame pointer is at current TOS */
	cmpl $0, 12(%ebp)	/* Compare arg2 and zero */
	ja gcd_cont		/*If arg2 > 0 continue this call */
	movl 8(%ebp), %eax	/* Otherwise, the result is arg1 */
	jmp gcd_return		/* Jump to return the result */

gcd_cont:
	movl 8(%ebp), %eax	/* Move arg1 into register eax */
	cltd			/* Set up arg1 into eax and edx for division */
	idivl 12(%ebp)		/* Divide arg1 by arg2, quotient in eax, remainder in edx */
	pushl %edx		/* Push the remainder onto stack (Next arg2) */
	pushl 12(%ebp)		/* Push the arg2 onto stack (Next arg1) */
	call gcd		/* Recursively call gcd */

gcd_return:
	movl %ebp, %esp		/* Pop local stack */
	popl %ebp		/* Pop old frame base from stack */
	ret			/* Return to caller */
