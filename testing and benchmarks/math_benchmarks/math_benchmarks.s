	.section	__TEXT,__text,regular,pure_instructions
	.macosx_version_min 10, 10
	.globl	_rdtsc
	.align	4, 0x90
_rdtsc:                                 ## @rdtsc
	.cfi_startproc
## BB#0:
	pushq	%rbp
Ltmp0:
	.cfi_def_cfa_offset 16
Ltmp1:
	.cfi_offset %rbp, -16
	movq	%rsp, %rbp
Ltmp2:
	.cfi_def_cfa_register %rbp
	## InlineAsm Start
	rdtsc
	## InlineAsm End
                                        ## kill: EDX<def> EDX<kill> RDX<def>
	shlq	$32, %rdx
	movl	%eax, %eax
	orq	%rdx, %rax
	popq	%rbp
	retq
	.cfi_endproc

	.section	__TEXT,__literal8,8byte_literals
	.align	3
LCPI1_0:
	.quad	4517329193108106637     ## double 9.9999999999999995E-7
	.section	__TEXT,__text,regular,pure_instructions
	.globl	_my_wtime
	.align	4, 0x90
_my_wtime:                              ## @my_wtime
	.cfi_startproc
## BB#0:
	pushq	%rbp
Ltmp3:
	.cfi_def_cfa_offset 16
Ltmp4:
	.cfi_offset %rbp, -16
	movq	%rsp, %rbp
Ltmp5:
	.cfi_def_cfa_register %rbp
	subq	$16, %rsp
	leaq	-16(%rbp), %rdi
	xorl	%esi, %esi
	callq	_gettimeofday
	vcvtsi2sdq	-16(%rbp), %xmm0, %xmm0
	vcvtsi2sdl	-8(%rbp), %xmm0, %xmm1
	vmulsd	LCPI1_0(%rip), %xmm1, %xmm1
	vaddsd	%xmm0, %xmm1, %xmm0
	addq	$16, %rsp
	popq	%rbp
	retq
	.cfi_endproc

	.globl	_test_vec4_mul_mm128_
	.align	4, 0x90
_test_vec4_mul_mm128_:                  ## @test_vec4_mul_mm128_
	.cfi_startproc
## BB#0:
	pushq	%rbp
Ltmp6:
	.cfi_def_cfa_offset 16
Ltmp7:
	.cfi_offset %rbp, -16
	movq	%rsp, %rbp
Ltmp8:
	.cfi_def_cfa_register %rbp
	vmovaps	(%rdi), %xmm1
	vmovaps	16(%rdi), %xmm0
	addq	$32, %rdi
	movl	$671088637, %eax        ## imm = 0x27FFFFFD
	.align	4, 0x90
LBB2_1:                                 ## =>This Inner Loop Header: Depth=1
	vmovaps	%xmm0, %xmm2
	vmulps	%xmm2, %xmm1, %xmm0
	vmovaps	%xmm0, (%rdi)
	addq	$16, %rdi
	vmovaps	%xmm2, %xmm1
	decq	%rax
	jne	LBB2_1
## BB#2:
	popq	%rbp
	retq
	.cfi_endproc

	.globl	_test_vec4_mul_mm128
	.align	4, 0x90
_test_vec4_mul_mm128:                   ## @test_vec4_mul_mm128
	.cfi_startproc
## BB#0:
	pushq	%rbp
Ltmp9:
	.cfi_def_cfa_offset 16
Ltmp10:
	.cfi_offset %rbp, -16
	movq	%rsp, %rbp
Ltmp11:
	.cfi_def_cfa_register %rbp
	vmovups	16(%rdi), %xmm0
	xorl	%eax, %eax
	.align	4, 0x90
LBB3_1:                                 ## =>This Inner Loop Header: Depth=1
	vmulps	(%rdi,%rax,4), %xmm0, %xmm0
	vmovups	%xmm0, 32(%rdi,%rax,4)
	vmulps	16(%rdi,%rax,4), %xmm0, %xmm0
	vmovups	%xmm0, 48(%rdi,%rax,4)
	leaq	8(%rax), %rax
	cmpl	$-1610612745, %eax      ## imm = 0xFFFFFFFF9FFFFFF7
	jb	LBB3_1
## BB#2:
	popq	%rbp
	retq
	.cfi_endproc

	.globl	_test_vec4_mul_scalar
	.align	4, 0x90
_test_vec4_mul_scalar:                  ## @test_vec4_mul_scalar
	.cfi_startproc
## BB#0:
	pushq	%rbp
Ltmp12:
	.cfi_def_cfa_offset 16
Ltmp13:
	.cfi_offset %rbp, -16
	movq	%rsp, %rbp
Ltmp14:
	.cfi_def_cfa_register %rbp
	vmovups	16(%rdi), %xmm0
	xorl	%eax, %eax
	.align	4, 0x90
LBB4_1:                                 ## =>This Inner Loop Header: Depth=1
	vmulps	(%rdi,%rax,4), %xmm0, %xmm0
	vmovups	%xmm0, 32(%rdi,%rax,4)
	vmulps	16(%rdi,%rax,4), %xmm0, %xmm0
	vmovups	%xmm0, 48(%rdi,%rax,4)
	leaq	8(%rax), %rax
	cmpl	$-1610612745, %eax      ## imm = 0xFFFFFFFF9FFFFFF7
	jb	LBB4_1
## BB#2:
	popq	%rbp
	retq
	.cfi_endproc

	.section	__TEXT,__literal4,4byte_literals
	.align	2
LCPI5_0:
	.long	973279855               ## float 5.00000024E-4
	.section	__TEXT,__literal8,8byte_literals
	.align	3
LCPI5_1:
	.quad	4517329193108106637     ## double 9.9999999999999995E-7
LCPI5_2:
	.quad	-4706042843746669171    ## double -9.9999999999999995E-7
	.section	__TEXT,__text,regular,pure_instructions
	.globl	_main
	.align	4, 0x90
_main:                                  ## @main
	.cfi_startproc
## BB#0:
	pushq	%rbp
Ltmp15:
	.cfi_def_cfa_offset 16
Ltmp16:
	.cfi_offset %rbp, -16
	movq	%rsp, %rbp
Ltmp17:
	.cfi_def_cfa_register %rbp
	pushq	%r15
	pushq	%r14
	pushq	%r13
	pushq	%r12
	pushq	%rbx
	subq	$2147483647, %rsp       ## imm = 0x7FFFFFFF
	subq	$2147483647, %rsp       ## imm = 0x7FFFFFFF
	subq	$2147483647, %rsp       ## imm = 0x7FFFFFFF
	subq	$2147483647, %rsp       ## imm = 0x7FFFFFFF
	subq	$2147483647, %rsp       ## imm = 0x7FFFFFFF
	subq	$29, %rsp
Ltmp18:
	.cfi_offset %rbx, -56
Ltmp19:
	.cfi_offset %r12, -48
Ltmp20:
	.cfi_offset %r13, -40
Ltmp21:
	.cfi_offset %r14, -32
Ltmp22:
	.cfi_offset %r15, -24
	movq	___stack_chk_guard@GOTPCREL(%rip), %r15
	movq	(%r15), %r15
	movq	%r15, -48(%rbp)
	leaq	2147483616(%rbp), %r13
	movabsq	$10737418224, %rsi      ## imm = 0x27FFFFFF0
	movq	%r13, %rdi
	callq	___bzero
	## InlineAsm Start
	rdtsc
	## InlineAsm End
	movl	%edx, %r14d
	movl	%eax, %r12d
	xorl	%ebx, %ebx
	leaq	2147483600(%rbp), %rdi
	xorl	%esi, %esi
	callq	_gettimeofday
	movq	2147483600(%rbp), %rax
	movl	2147483608(%rbp), %ecx
	vmovss	LCPI5_0(%rip), %xmm0
	.align	4, 0x90
LBB5_1:                                 ## =>This Inner Loop Header: Depth=1
	movl	%ebx, %edx
	vcvtsi2ssq	%rdx, %xmm0, %xmm1
	vmulss	%xmm0, %xmm1, %xmm1
	vpshufd	$0, %xmm1, %xmm1        ## xmm1 = xmm1[0,0,0,0]
	vmovdqa	%xmm1, (%r13)
	incq	%rbx
	addq	$16, %r13
	cmpq	$671088639, %rbx        ## imm = 0x27FFFFFF
	jne	LBB5_1
## BB#2:
	shlq	$32, %r14
	orq	%r12, %r14
	vxorps	%xmm0, %xmm0, %xmm0
	vcvtsi2sdq	%rax, %xmm0, %xmm0
	vmovsd	%xmm0, 2147483592(%rbp) ## 8-byte Spill
	vxorps	%xmm0, %xmm0, %xmm0
	vcvtsi2sdl	%ecx, %xmm0, %xmm0
	vmovsd	%xmm0, 2147483584(%rbp) ## 8-byte Spill
	## InlineAsm Start
	rdtsc
	## InlineAsm End
                                        ## kill: EDX<def> EDX<kill> RDX<def>
	shlq	$32, %rdx
	movl	%eax, %eax
	orq	%rdx, %rax
	movq	_profilers(%rip), %rcx
	subq	%r14, %rcx
	addq	%rax, %rcx
	movq	%rcx, _profilers(%rip)
	addl	$-1610612737, _profilers+8(%rip) ## imm = 0xFFFFFFFF9FFFFFFF
	xorl	%r14d, %r14d
	leaq	2147483600(%rbp), %rdi
	xorl	%esi, %esi
	callq	_gettimeofday
	vcvtsi2sdq	2147483600(%rbp), %xmm0, %xmm0
	vcvtsi2sdl	2147483608(%rbp), %xmm0, %xmm1
	vmulsd	LCPI5_1(%rip), %xmm1, %xmm1
	vmovsd	2147483584(%rbp), %xmm2 ## 8-byte Reload
	vmulsd	LCPI5_2(%rip), %xmm2, %xmm2
	vsubsd	2147483592(%rbp), %xmm2, %xmm2 ## 8-byte Folded Reload
	vaddsd	%xmm0, %xmm2, %xmm0
	vaddsd	%xmm1, %xmm0, %xmm0
	vaddsd	_profilers+16(%rip), %xmm0, %xmm0
	vmovsd	%xmm0, _profilers+16(%rip)
	movq	_profilers(%rip), %rbx
	movl	_profilers+8(%rip), %ecx
	testq	%rcx, %rcx
	je	LBB5_4
## BB#3:
	xorl	%edx, %edx
	movq	%rbx, %rax
	divq	%rcx
	movq	%rax, %r14
LBB5_4:
	leaq	L_.str(%rip), %rdi
	leaq	L_.str1(%rip), %rsi
	movb	$1, %al
	movq	%rbx, %rdx
                                        ## kill: ECX<def> ECX<kill> RCX<kill>
	movq	%r14, %r8
	callq	_printf
	## InlineAsm Start
	rdtsc
	## InlineAsm End
	movl	%edx, %ebx
	movl	%eax, %r14d
	leaq	2147483600(%rbp), %rdi
	xorl	%esi, %esi
	callq	_gettimeofday
	movq	2147483600(%rbp), %rax
	movl	2147483608(%rbp), %ecx
	vmovaps	2147483616(%rbp), %xmm1
	vmovaps	2147483632(%rbp), %xmm0
	leaq	-2147483648(%rbp), %rdx
	movl	$671088637, %esi        ## imm = 0x27FFFFFD
	.align	4, 0x90
LBB5_5:                                 ## =>This Inner Loop Header: Depth=1
	vmovaps	%xmm0, %xmm2
	vmulps	%xmm1, %xmm2, %xmm0
	vmovaps	%xmm0, (%rdx)
	addq	$16, %rdx
	vmovaps	%xmm2, %xmm1
	decq	%rsi
	jne	LBB5_5
## BB#6:                                ## %test_vec4_mul_mm128_.exit
	shlq	$32, %rbx
	orq	%r14, %rbx
	vxorps	%xmm0, %xmm0, %xmm0
	vcvtsi2sdq	%rax, %xmm0, %xmm0
	vmovsd	%xmm0, 2147483592(%rbp) ## 8-byte Spill
	vxorps	%xmm0, %xmm0, %xmm0
	vcvtsi2sdl	%ecx, %xmm0, %xmm0
	vmovsd	%xmm0, 2147483584(%rbp) ## 8-byte Spill
	## InlineAsm Start
	rdtsc
	## InlineAsm End
                                        ## kill: EDX<def> EDX<kill> RDX<def>
	shlq	$32, %rdx
	movl	%eax, %eax
	orq	%rdx, %rax
	movq	_profilers+24(%rip), %rcx
	subq	%rbx, %rcx
	addq	%rax, %rcx
	movq	%rcx, _profilers+24(%rip)
	addl	$1342177277, _profilers+32(%rip) ## imm = 0x4FFFFFFD
	xorl	%r14d, %r14d
	leaq	2147483600(%rbp), %rdi
	xorl	%esi, %esi
	callq	_gettimeofday
	vcvtsi2sdq	2147483600(%rbp), %xmm0, %xmm0
	vcvtsi2sdl	2147483608(%rbp), %xmm0, %xmm1
	vmulsd	LCPI5_1(%rip), %xmm1, %xmm1
	vmovsd	2147483584(%rbp), %xmm2 ## 8-byte Reload
	vmulsd	LCPI5_2(%rip), %xmm2, %xmm2
	vsubsd	2147483592(%rbp), %xmm2, %xmm2 ## 8-byte Folded Reload
	vaddsd	%xmm0, %xmm2, %xmm0
	vaddsd	%xmm1, %xmm0, %xmm0
	vaddsd	_profilers+40(%rip), %xmm0, %xmm0
	vmovsd	%xmm0, _profilers+40(%rip)
	movq	_profilers+24(%rip), %rbx
	movl	_profilers+32(%rip), %ecx
	testq	%rcx, %rcx
	je	LBB5_8
## BB#7:
	xorl	%edx, %edx
	movq	%rbx, %rax
	divq	%rcx
	movq	%rax, %r14
LBB5_8:
	leaq	L_.str(%rip), %rdi
	leaq	L_.str2(%rip), %rsi
	movb	$1, %al
	movq	%rbx, %rdx
                                        ## kill: ECX<def> ECX<kill> RCX<kill>
	movq	%r14, %r8
	callq	_printf
	cmpq	-48(%rbp), %r15
	jne	LBB5_10
## BB#9:
	xorl	%eax, %eax
	addq	$2147483647, %rsp       ## imm = 0x7FFFFFFF
	addq	$2147483647, %rsp       ## imm = 0x7FFFFFFF
	addq	$2147483647, %rsp       ## imm = 0x7FFFFFFF
	addq	$2147483647, %rsp       ## imm = 0x7FFFFFFF
	addq	$2147483647, %rsp       ## imm = 0x7FFFFFFF
	addq	$29, %rsp
	popq	%rbx
	popq	%r12
	popq	%r13
	popq	%r14
	popq	%r15
	popq	%rbp
	retq
LBB5_10:
	callq	___stack_chk_fail
	.cfi_endproc

	.globl	_profilers              ## @profilers
.zerofill __DATA,__common,_profilers,24576,4
	.section	__TEXT,__cstring,cstring_literals
L_.str:                                 ## @.str
	.asciz	"[%-30s]\tcycles: %12llu,\t\thits: %10u,\t\tcycles/hit: %12llu,\t\ttime: %10f\n"

L_.str1:                                ## @.str1
	.asciz	"random_generation"

L_.str2:                                ## @.str2
	.asciz	"vec4_mul_mm128_"


.subsections_via_symbols
