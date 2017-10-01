/****************************************************************************************
 *                                                                                      *
 *  ----------------------------------------------------------------------------------  *
 *  |    0    |    1    |    2    |    3    |    4     |    5    |    6    |    7    |  *
 *  ----------------------------------------------------------------------------------  *
 *  |   0x0   |   0x4   |   0x8   |   0xc   |   0x10   |   0x14  |   0x18  |   0x1c  |  *
 *  ----------------------------------------------------------------------------------  *
 *  | fc_mxcsr|fc_x87_cw|        R12        |         R13        |        R14        |  *
 *  ----------------------------------------------------------------------------------  *
 *  ----------------------------------------------------------------------------------  *
 *  |    8    |    9    |   10    |   11    |    12    |    13   |    14   |    15   |  *
 *  ----------------------------------------------------------------------------------  *
 *  |   0x20  |   0x24  |   0x28  |  0x2c   |   0x30   |   0x34  |   0x38  |   0x3c  |  *
 *  ----------------------------------------------------------------------------------  *
 *  |        R15        |        RBX        |         RBP        |        RIP        |  *
 *  ----------------------------------------------------------------------------------  *
 *                                                                                      *
 ****************************************************************************************/

.text
.globl _jump_fcontext
.align 8
_jump_fcontext:
	leaq -0x38(%rsp), %rsp // Reserve space on the stack

	stmxcsr (%rsp)   // save MMX control- and status-word
	fnstcw 0x4(%rsp) // save x87 control-word

	movq %r12, 0x8(%rsp)  // save R12
	movq %r13, 0x10(%rsp)  // save R13
	movq %r14, 0x18(%rsp)  // save R14
	movq %r15, 0x20(%rsp)  // save R15
	movq %rbx, 0x28(%rsp)  // save RBX
	movq %rbp, 0x30(%rsp)  // save RBP

	// RDI (first argument) contains the address to where we want to store our context.
	// Go ahead and store rsp (pointing our context) in the address given by RDI (first arg passed to jump_fcontext)
	movq %rsp, (%rdi)

	// RSI (second argument) contains the address of the context we want to switch to.
	// Load the pointer to the given context into the stack pointer
	movq %rsi, %rsp

	movq 0x38(%rsp), %r8 // restore return-address (address of trampoline function)

	ldmxcsr(%rsp)  // restore MMX control- and status-word
	fldcw 0x4(%rsp) // restore x87 control-word

	movq 0x8(%rsp), %r12  // restore R12
	movq 0x10(%rsp), %r13  // restore R13
	movq 0x18(%rsp), %r14  // restore R14
	movq 0x20(%rsp), %r15  // restore R15
	movq 0x28(%rsp), %rbx  // restore RBX
	movq 0x30(%rsp), %rbp  // restore RBP

	leaq 0x40(%rsp), %rsp // prepare stack

	// Set RDX (third argument) as the first argument passed to the context function.
	movq %rdx, %rdi

	// indirect jump to context
	jmp *%r8
