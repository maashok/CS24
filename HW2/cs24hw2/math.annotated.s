/* This file contains IA32 assembly-language implementations of three
 * basic, very common math operations.
 *
 * Instead of using branching operations (conditional control), it uses conditional data transfer or no such operations at all. This is preferred by the compiler since it calculates what   * to do ahead of time and do multiple instructions at the same time when only transferring data conditionally. However, branching operations can only be done when the instruction gets to  * the branching statement and then, only after that logic is executed, it can decide what to do next
 */

    .text

/*====================================================================
 * The function checks whether x or y is less (signed comparison).
 * int f1(int x, int y)
 */
.globl f1
f1:
	pushl	%ebp
	movl	%esp, %ebp
	movl	8(%ebp), %edx  // Move the first argument(put last on stack) into edx
	movl	12(%ebp), %eax // Move the second argument into eax
	cmpl	%edx, %eax     // Compare whether y is greater than x
	cmovg	%edx, %eax     // If y is greater than x, then move x into eax 
			       // (If x is greater than y, then y would still be in eax)
	popl	%ebp
	ret


/*====================================================================
 * This function takes the absolute value of the input value
 * int f2(int x)
 */
.globl f2
f2:
	pushl	%ebp
	movl	%esp, %ebp
	movl	8(%ebp), %eax  // Move the argument, x, into eax
	movl	%eax, %edx     // And into edx
	sarl	$31, %edx      // Right shift x by 31 bits and store in edx. If x >= 0, x >> 31 = 0. If x < 0, x >> 31 = -1
	xorl	%edx, %eax     // (x >> 31) ^ x stored in eax. If x >= 0, this is just x itself or ~x + 1, which is how we get the absolute value.
	popl	%ebp
	ret


/*====================================================================
 * Checks if the input is positive, negative, or zero. If positive, returns 1. If negative returns * -1. If zero, returns 0.
 * int f3(int x)
 */
.globl f3
f3:
	pushl	%ebp
	movl	%esp, %ebp
	movl	8(%ebp), %edx // Moves input, x, into edx
	movl	%edx, %eax    // and in eax
	sarl	$31, %eax     // Stores x >> 31 into eax. If x >= 0, eax = 0. If x < 0, eax = -1
	testl	%edx, %edx    // Performs x & x. This will set the flags for x itself. If x > 0 then ZF = 0, SF = 0, and OF = 0. If x < 0, ZF = 0, SF = 1, OF = 0. 				 			  // If x = 0, ZF = 1, SF = 0, OF = 0.
	movl	$1, %edx      // %edx = 1
	cmovg	%edx, %eax    // If ZF = 0 and SF = OF, then move 1 into %eax. This will only happen if x > 0. Otherwise, at this point eax = -1 if x < 0 or 0 if x = 0.
	popl	%ebp
	ret

