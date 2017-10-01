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

// RAX - The accumulator register
// RBX - The base register
// RCX - The counter
// RDX - The data register
// RSP - Stack pointer
// RBP - Points to the base of the stack frame
// RSI - Source index for string operations
// RDI - Destination index for string operations
// RIP - Instruction Pointer

// The stack grows "downwards" from high memory address to low, so set the start at the highest address.
// We don't actually care about the size in mac osx, but we do on other systems.
// void *make_fcontext(void * stack_pointer, size_t size, void(*func)(void *));

// Microsoft x64 calling convention
// It uses registers
// 		RCX, RDX, R8, R9
// for the first four integer or pointer arguments (in that order), and
// 		XMM0, XMM1, XMM2, XMM3
// are used for floating point arguments.

// Calling convensions for System V AMD64 AB:
// The first six integer or pointer arguments are passed in registers:
// 		RDI, RSI, RDX, RCX, R8, and R9
// while
// 		XMM0, XMM1, XMM2, XMM3, XMM4, XMM5, XMM6 and XMM7
// are used for certain floating point arguments.

.text
.globl _make_fcontext
.align 8
_make_fcontext:
	// first argument (RDI) of make_fcontext() is the top of the user allocated context-stack
	movq %rdi, %rax

	// shift address in RAX to lower 16 byte boundary, why??
	andq $-16, %rax // %rax &= (-16)   -16 == 0xfffffff0

	// reserve space for context-data on context-stack
	// on context-function entry: (RSP -0x8) % 16 == 0
	leaq -0x40(%rax), %rax

	// Third argument to make_fcontext() is the address to the function to run
	// Stored in RBX
	movq %rdx, 0x28(%rax)

	// save MMX control- and status-word
	stmxcsr (%rax)
	// save x87 control-word
	fnstcw 0x4(%rax)

	// compute abs address of label trampoline
	leaq trampoline(%rip), %rcx

	// save address of trampoline as return-address for context-function
	// will be entered after calling jump_fcontext() first time
	movq %rcx, 0x38(%rax)

	// compute abs address of label finish
	leaq finish(%rip), %rcx

	// save address of finish as return-address for context-function
	// will be entered after context-function returns
	movq %rcx, 0x30(%rax)

	ret // return pointer to context-data

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
