	.file	"test.c"
	.text
	.globl	function
	.def	function;	.scl	2;	.type	32;	.endef
	.seh_proc	function
function:
	pushq	%rbp
	.seh_pushreg	%rbp
	pushq	%rbx
	.seh_pushreg	%rbx
	subq	$40, %rsp
	.seh_stackalloc	40
	leaq	128(%rsp), %rbp
	.seh_setframe	%rbp, 128
	.seh_endprologue
	movl	%ecx, -64(%rbp)
	cmpl	$1, -64(%rbp)
	jg	.L2
	movl	$1, %eax
	jmp	.L3
.L2:
	movl	-64(%rbp), %eax
	subl	$1, %eax
	movl	%eax, %ecx
	call	function
	movl	%eax, %ebx
	movl	-64(%rbp), %eax
	subl	$2, %eax
	movl	%eax, %ecx
	call	function
	addl	%ebx, %eax
.L3:
	addq	$40, %rsp
	popq	%rbx
	popq	%rbp
	ret
	.seh_endproc
	.def	__main;	.scl	2;	.type	32;	.endef
	.section .rdata,"dr"
.LC0:
	.ascii "%d\12\0"
	.text
	.globl	main
	.def	main;	.scl	2;	.type	32;	.endef
	.seh_proc	main
main:
	pushq	%rbp
	.seh_pushreg	%rbp
	movq	%rsp, %rbp
	.seh_setframe	%rbp, 0
	subq	$64, %rsp
	.seh_stackalloc	64
	.seh_endprologue
	call	__main
	call	clock
	movq	%rax, -16(%rbp)
	movl	$0, -4(%rbp)
	jmp	.L5
.L6:
	movl	$40, %ecx
	call	function
	addl	$1, -4(%rbp)
.L5:
	cmpl	$9, -4(%rbp)
	jle	.L6
	call	clock
	movq	%rax, -24(%rbp)
	movq	-24(%rbp), %rax
	subq	-16(%rbp), %rax
	movq	%rax, %rdx
	leaq	.LC0(%rip), %rcx
	call	printf
	movl	$0, %eax
	addq	$64, %rsp
	popq	%rbp
	ret
	.seh_endproc
	.ident	"GCC: (GNU) 7.3.0"
	.def	clock;	.scl	2;	.type	32;	.endef
	.def	printf;	.scl	2;	.type	32;	.endef
