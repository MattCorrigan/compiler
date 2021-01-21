	.globl main
.format:
	.ascii "%d\12\0"
.L0:
	.ascii "hello\12\0"
hello:
	pushq %rbp
	movq %rsp, %rbp
	subq $128, %rsp
	leaq .L0(%rip), %rax
	pushq %rcx
	pushq %rdx
	movq %rax, %rcx
	call printf
	popq %rdx
	popq %rcx
	movq $0, %rax
	addq $128, %rsp
	popq %rbp
	ret

main:
	pushq %rbp
	movq %rsp, %rbp
	subq $128, %rsp
	call __main
	movq $0, %rax
	movq $5, %rbx
	movq %rax, -8(%rbp)
	cmp %rbx, -8(%rbp)
	jge f2
	f1:
	call hello
	addq $1, -8(%rbp)
	cmp %rbx, -8(%rbp)
	jl f1
	f2:
	movq $0, %rax
	addq $128, %rsp
	popq %rbp
	ret

