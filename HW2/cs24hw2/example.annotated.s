	.file	"example.c"
	.section	.text.unlikely,"ax",@progbits
.LCOLDB0:
	.text
.LHOTB0:
	.p2align 4,,15
	.globl	ex
	.type	ex, @function
ex:
.LFB0:
	.cfi_startproc
	movl	8(%esp), %eax /* Moves b into %eax for future use */
	subl	12(%esp), %eax /* Subtracts c from b (which was stored in %eax) */
	imull	4(%esp), %eax /*Multiplies a times b-c (which was stored in %eax) */
	addl	16(%esp), %eax /* Adds d to a*(b-c) (which was stored in %eax) */
	ret
	.cfi_endproc
.LFE0:
	.size	ex, .-ex
	.section	.text.unlikely
.LCOLDE0:
	.text
.LHOTE0:
	.ident	"GCC: (GNU) 4.9.2 20150304 (prerelease)"
	.section	.note.GNU-stack,"",@progbits
