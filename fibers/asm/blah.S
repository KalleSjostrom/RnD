.text
.globl _blah
.align 8
_blah:
	// first argument of make_fcontext() is the top of the user allocated context-stack
	movq %rdi, %rax
	andq  $-16, %rax

	/* reserve space for context-data on context-stack */
	/* on context-function entry: (RSP -0x8) % 16 == 0 */
	leaq  -0x40(%rax), %rax

	/* third arg of make_fcontext() == address of context-function */
	/* stored in RBX */
	movq  %rdx, 0x28(%rax)

	/* save MMX control- and status-word */
	stmxcsr  (%rax)
	/* save x87 control-word */
	fnstcw   0x4(%rax)

	/* compute abs address of label trampoline */
	leaq  trampoline(%rip), %rcx
	/* save address of trampoline as return-address for context-function */
	/* will be entered after calling jump_fcontext() first time */
	movq  %rcx, 0x38(%rax)

	/* compute abs address of label finish */
	leaq  finish(%rip), %rcx
	/* save address of finish as return-address for context-function */
	/* will be entered after context-function returns */
	movq  %rcx, 0x30(%rax)

	ret /* return pointer to context-data */

trampoline:
	/* store return address on stack */
	/* fix stack alignment */
	push %rbp
	/* jump to context-function */
	jmp *%rbx

finish:
	/* exit code is zero */
	xorq  %rdi, %rdi
	/* exit application */
	call  __exit
	hlt
