	.section .vectors,"ax",@progbits
	.global	__vectors
	.func	__vectors
__vectors:
	rjmp main
	.endfunc