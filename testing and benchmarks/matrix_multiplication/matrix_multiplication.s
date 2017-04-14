	.section	__TEXT,__text,regular,pure_instructions
	.macosx_version_min 10, 11
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

	.globl	_mm_vector_128
	.align	4, 0x90
_mm_vector_128:                         ## @mm_vector_128
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
	vmovaps	(%rdi), %xmm3
	vmovaps	16(%rdi), %xmm2
	vmovaps	32(%rdi), %xmm1
	vmovaps	48(%rdi), %xmm0
	vbroadcastss	(%rsi), %xmm4
	vmulps	%xmm3, %xmm4, %xmm4
	vbroadcastss	4(%rsi), %xmm5
	vmulps	%xmm2, %xmm5, %xmm5
	vaddps	%xmm4, %xmm5, %xmm4
	vbroadcastss	8(%rsi), %xmm5
	vmulps	%xmm1, %xmm5, %xmm5
	vbroadcastss	12(%rsi), %xmm6
	vmulps	%xmm0, %xmm6, %xmm6
	vaddps	%xmm6, %xmm5, %xmm5
	vaddps	%xmm5, %xmm4, %xmm8
	vbroadcastss	16(%rsi), %xmm5
	vmulps	%xmm3, %xmm5, %xmm5
	vbroadcastss	20(%rsi), %xmm6
	vmulps	%xmm2, %xmm6, %xmm6
	vaddps	%xmm5, %xmm6, %xmm5
	vbroadcastss	24(%rsi), %xmm6
	vmulps	%xmm1, %xmm6, %xmm6
	vbroadcastss	28(%rsi), %xmm7
	vmulps	%xmm0, %xmm7, %xmm7
	vaddps	%xmm7, %xmm6, %xmm6
	vaddps	%xmm6, %xmm5, %xmm5
	vbroadcastss	32(%rsi), %xmm6
	vmulps	%xmm3, %xmm6, %xmm6
	vbroadcastss	36(%rsi), %xmm7
	vmulps	%xmm2, %xmm7, %xmm7
	vaddps	%xmm6, %xmm7, %xmm6
	vbroadcastss	40(%rsi), %xmm7
	vmulps	%xmm1, %xmm7, %xmm7
	vbroadcastss	44(%rsi), %xmm4
	vmulps	%xmm0, %xmm4, %xmm4
	vaddps	%xmm4, %xmm7, %xmm4
	vaddps	%xmm4, %xmm6, %xmm4
	vbroadcastss	48(%rsi), %xmm6
	vmulps	%xmm3, %xmm6, %xmm3
	vbroadcastss	52(%rsi), %xmm6
	vmulps	%xmm2, %xmm6, %xmm2
	vaddps	%xmm3, %xmm2, %xmm2
	vbroadcastss	56(%rsi), %xmm3
	vmulps	%xmm1, %xmm3, %xmm1
	vbroadcastss	60(%rsi), %xmm3
	vmulps	%xmm0, %xmm3, %xmm0
	vaddps	%xmm0, %xmm1, %xmm0
	vaddps	%xmm0, %xmm2, %xmm0
	vmovaps	%xmm8, (%rdx)
	vmovaps	%xmm5, 16(%rdx)
	vmovaps	%xmm4, 32(%rdx)
	vmovaps	%xmm0, 48(%rdx)
	popq	%rbp
	retq
	.cfi_endproc

	.globl	_mm_vector_256
	.align	4, 0x90
_mm_vector_256:                         ## @mm_vector_256
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
	vmovups	(%rdi), %ymm1
	vmovups	32(%rdi), %ymm0
	vbroadcastss	20(%rsi), %xmm2
	vbroadcastss	(%rsi), %xmm3
	vinsertf128	$1, %xmm3, %ymm2, %ymm2
	vmulps	%ymm1, %ymm2, %ymm2
	vbroadcastss	28(%rsi), %xmm3
	vbroadcastss	8(%rsi), %xmm4
	vinsertf128	$1, %xmm4, %ymm3, %ymm3
	vaddps	%ymm2, %ymm3, %ymm2
	vbroadcastss	4(%rsi), %ymm3
	vbroadcastss	16(%rsi), %ymm4
	vinsertf128	$1, %xmm4, %ymm3, %ymm3
	vmulps	%ymm1, %ymm3, %ymm3
	vbroadcastss	24(%rsi), %ymm5
	vinsertf128	$1, %xmm5, %ymm4, %ymm4
	vmulps	%ymm0, %ymm4, %ymm4
	vaddps	%ymm3, %ymm4, %ymm3
	vperm2f128	$1, %ymm0, %ymm3, %ymm3 ## ymm3 = ymm3[2,3,0,1]
	vaddps	%ymm3, %ymm2, %ymm2
	vmovups	%ymm2, (%rdx)
	vbroadcastss	52(%rsi), %xmm2
	vbroadcastss	32(%rsi), %xmm3
	vinsertf128	$1, %xmm3, %ymm2, %ymm2
	vmulps	%ymm1, %ymm2, %ymm2
	vbroadcastss	60(%rsi), %xmm3
	vbroadcastss	40(%rsi), %xmm4
	vinsertf128	$1, %xmm4, %ymm3, %ymm3
	vaddps	%ymm2, %ymm3, %ymm2
	vbroadcastss	36(%rsi), %ymm3
	vbroadcastss	48(%rsi), %ymm4
	vinsertf128	$1, %xmm4, %ymm3, %ymm3
	vmulps	%ymm1, %ymm3, %ymm1
	vbroadcastss	56(%rsi), %ymm3
	vinsertf128	$1, %xmm3, %ymm4, %ymm3
	vmulps	%ymm0, %ymm3, %ymm0
	vaddps	%ymm1, %ymm0, %ymm0
	vperm2f128	$1, %ymm0, %ymm0, %ymm0 ## ymm0 = ymm0[2,3,0,1]
	vaddps	%ymm0, %ymm2, %ymm0
	vmovups	%ymm0, 32(%rdx)
	popq	%rbp
	vzeroupper
	retq
	.cfi_endproc

	.globl	_mm_scalar_unrolled
	.align	4, 0x90
_mm_scalar_unrolled:                    ## @mm_scalar_unrolled
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
	vmovss	(%rsi), %xmm0           ## xmm0 = mem[0],zero,zero,zero
	vmovss	4(%rsi), %xmm1          ## xmm1 = mem[0],zero,zero,zero
	vmulss	(%rdi), %xmm0, %xmm0
	vmulss	16(%rdi), %xmm1, %xmm1
	vaddss	%xmm0, %xmm1, %xmm0
	vmovss	8(%rsi), %xmm1          ## xmm1 = mem[0],zero,zero,zero
	vmulss	32(%rdi), %xmm1, %xmm1
	vmovss	12(%rsi), %xmm2         ## xmm2 = mem[0],zero,zero,zero
	vmulss	48(%rdi), %xmm2, %xmm2
	vaddss	%xmm2, %xmm1, %xmm1
	vaddss	%xmm1, %xmm0, %xmm0
	vmovss	%xmm0, (%rdx)
	vmovss	(%rsi), %xmm0           ## xmm0 = mem[0],zero,zero,zero
	vmovss	4(%rsi), %xmm1          ## xmm1 = mem[0],zero,zero,zero
	vmulss	4(%rdi), %xmm0, %xmm0
	vmulss	20(%rdi), %xmm1, %xmm1
	vaddss	%xmm0, %xmm1, %xmm0
	vmovss	8(%rsi), %xmm1          ## xmm1 = mem[0],zero,zero,zero
	vmulss	36(%rdi), %xmm1, %xmm1
	vmovss	12(%rsi), %xmm2         ## xmm2 = mem[0],zero,zero,zero
	vmulss	52(%rdi), %xmm2, %xmm2
	vaddss	%xmm2, %xmm1, %xmm1
	vaddss	%xmm1, %xmm0, %xmm0
	vmovss	%xmm0, 4(%rdx)
	vmovss	(%rsi), %xmm0           ## xmm0 = mem[0],zero,zero,zero
	vmovss	4(%rsi), %xmm1          ## xmm1 = mem[0],zero,zero,zero
	vmulss	8(%rdi), %xmm0, %xmm0
	vmulss	24(%rdi), %xmm1, %xmm1
	vaddss	%xmm0, %xmm1, %xmm0
	vmovss	8(%rsi), %xmm1          ## xmm1 = mem[0],zero,zero,zero
	vmulss	40(%rdi), %xmm1, %xmm1
	vmovss	12(%rsi), %xmm2         ## xmm2 = mem[0],zero,zero,zero
	vmulss	56(%rdi), %xmm2, %xmm2
	vaddss	%xmm2, %xmm1, %xmm1
	vaddss	%xmm1, %xmm0, %xmm0
	vmovss	%xmm0, 8(%rdx)
	vmovss	(%rsi), %xmm0           ## xmm0 = mem[0],zero,zero,zero
	vmovss	4(%rsi), %xmm1          ## xmm1 = mem[0],zero,zero,zero
	vmulss	12(%rdi), %xmm0, %xmm0
	vmulss	28(%rdi), %xmm1, %xmm1
	vaddss	%xmm0, %xmm1, %xmm0
	vmovss	8(%rsi), %xmm1          ## xmm1 = mem[0],zero,zero,zero
	vmulss	44(%rdi), %xmm1, %xmm1
	vmovss	12(%rsi), %xmm2         ## xmm2 = mem[0],zero,zero,zero
	vmulss	60(%rdi), %xmm2, %xmm2
	vaddss	%xmm2, %xmm1, %xmm1
	vaddss	%xmm1, %xmm0, %xmm0
	vmovss	%xmm0, 12(%rdx)
	vmovss	16(%rsi), %xmm0         ## xmm0 = mem[0],zero,zero,zero
	vmovss	20(%rsi), %xmm1         ## xmm1 = mem[0],zero,zero,zero
	vmulss	(%rdi), %xmm0, %xmm0
	vmulss	16(%rdi), %xmm1, %xmm1
	vaddss	%xmm0, %xmm1, %xmm0
	vmovss	24(%rsi), %xmm1         ## xmm1 = mem[0],zero,zero,zero
	vmulss	32(%rdi), %xmm1, %xmm1
	vmovss	28(%rsi), %xmm2         ## xmm2 = mem[0],zero,zero,zero
	vmulss	48(%rdi), %xmm2, %xmm2
	vaddss	%xmm2, %xmm1, %xmm1
	vaddss	%xmm1, %xmm0, %xmm0
	vmovss	%xmm0, 16(%rdx)
	vmovss	16(%rsi), %xmm0         ## xmm0 = mem[0],zero,zero,zero
	vmovss	20(%rsi), %xmm1         ## xmm1 = mem[0],zero,zero,zero
	vmulss	4(%rdi), %xmm0, %xmm0
	vmulss	20(%rdi), %xmm1, %xmm1
	vaddss	%xmm0, %xmm1, %xmm0
	vmovss	24(%rsi), %xmm1         ## xmm1 = mem[0],zero,zero,zero
	vmulss	36(%rdi), %xmm1, %xmm1
	vmovss	28(%rsi), %xmm2         ## xmm2 = mem[0],zero,zero,zero
	vmulss	52(%rdi), %xmm2, %xmm2
	vaddss	%xmm2, %xmm1, %xmm1
	vaddss	%xmm1, %xmm0, %xmm0
	vmovss	%xmm0, 20(%rdx)
	vmovss	16(%rsi), %xmm0         ## xmm0 = mem[0],zero,zero,zero
	vmovss	20(%rsi), %xmm1         ## xmm1 = mem[0],zero,zero,zero
	vmulss	8(%rdi), %xmm0, %xmm0
	vmulss	24(%rdi), %xmm1, %xmm1
	vaddss	%xmm0, %xmm1, %xmm0
	vmovss	24(%rsi), %xmm1         ## xmm1 = mem[0],zero,zero,zero
	vmulss	40(%rdi), %xmm1, %xmm1
	vmovss	28(%rsi), %xmm2         ## xmm2 = mem[0],zero,zero,zero
	vmulss	56(%rdi), %xmm2, %xmm2
	vaddss	%xmm2, %xmm1, %xmm1
	vaddss	%xmm1, %xmm0, %xmm0
	vmovss	%xmm0, 24(%rdx)
	vmovss	16(%rsi), %xmm0         ## xmm0 = mem[0],zero,zero,zero
	vmovss	20(%rsi), %xmm1         ## xmm1 = mem[0],zero,zero,zero
	vmulss	12(%rdi), %xmm0, %xmm0
	vmulss	28(%rdi), %xmm1, %xmm1
	vaddss	%xmm0, %xmm1, %xmm0
	vmovss	24(%rsi), %xmm1         ## xmm1 = mem[0],zero,zero,zero
	vmulss	44(%rdi), %xmm1, %xmm1
	vmovss	28(%rsi), %xmm2         ## xmm2 = mem[0],zero,zero,zero
	vmulss	60(%rdi), %xmm2, %xmm2
	vaddss	%xmm2, %xmm1, %xmm1
	vaddss	%xmm1, %xmm0, %xmm0
	vmovss	%xmm0, 28(%rdx)
	vmovss	32(%rsi), %xmm0         ## xmm0 = mem[0],zero,zero,zero
	vmovss	36(%rsi), %xmm1         ## xmm1 = mem[0],zero,zero,zero
	vmulss	(%rdi), %xmm0, %xmm0
	vmulss	16(%rdi), %xmm1, %xmm1
	vaddss	%xmm0, %xmm1, %xmm0
	vmovss	40(%rsi), %xmm1         ## xmm1 = mem[0],zero,zero,zero
	vmulss	32(%rdi), %xmm1, %xmm1
	vmovss	44(%rsi), %xmm2         ## xmm2 = mem[0],zero,zero,zero
	vmulss	48(%rdi), %xmm2, %xmm2
	vaddss	%xmm2, %xmm1, %xmm1
	vaddss	%xmm1, %xmm0, %xmm0
	vmovss	%xmm0, 32(%rdx)
	vmovss	32(%rsi), %xmm0         ## xmm0 = mem[0],zero,zero,zero
	vmovss	36(%rsi), %xmm1         ## xmm1 = mem[0],zero,zero,zero
	vmulss	4(%rdi), %xmm0, %xmm0
	vmulss	20(%rdi), %xmm1, %xmm1
	vaddss	%xmm0, %xmm1, %xmm0
	vmovss	40(%rsi), %xmm1         ## xmm1 = mem[0],zero,zero,zero
	vmulss	36(%rdi), %xmm1, %xmm1
	vmovss	44(%rsi), %xmm2         ## xmm2 = mem[0],zero,zero,zero
	vmulss	52(%rdi), %xmm2, %xmm2
	vaddss	%xmm2, %xmm1, %xmm1
	vaddss	%xmm1, %xmm0, %xmm0
	vmovss	%xmm0, 36(%rdx)
	vmovss	32(%rsi), %xmm0         ## xmm0 = mem[0],zero,zero,zero
	vmovss	36(%rsi), %xmm1         ## xmm1 = mem[0],zero,zero,zero
	vmulss	8(%rdi), %xmm0, %xmm0
	vmulss	24(%rdi), %xmm1, %xmm1
	vaddss	%xmm0, %xmm1, %xmm0
	vmovss	40(%rsi), %xmm1         ## xmm1 = mem[0],zero,zero,zero
	vmulss	40(%rdi), %xmm1, %xmm1
	vmovss	44(%rsi), %xmm2         ## xmm2 = mem[0],zero,zero,zero
	vmulss	56(%rdi), %xmm2, %xmm2
	vaddss	%xmm2, %xmm1, %xmm1
	vaddss	%xmm1, %xmm0, %xmm0
	vmovss	%xmm0, 40(%rdx)
	vmovss	32(%rsi), %xmm0         ## xmm0 = mem[0],zero,zero,zero
	vmovss	36(%rsi), %xmm1         ## xmm1 = mem[0],zero,zero,zero
	vmulss	12(%rdi), %xmm0, %xmm0
	vmulss	28(%rdi), %xmm1, %xmm1
	vaddss	%xmm0, %xmm1, %xmm0
	vmovss	40(%rsi), %xmm1         ## xmm1 = mem[0],zero,zero,zero
	vmulss	44(%rdi), %xmm1, %xmm1
	vmovss	44(%rsi), %xmm2         ## xmm2 = mem[0],zero,zero,zero
	vmulss	60(%rdi), %xmm2, %xmm2
	vaddss	%xmm2, %xmm1, %xmm1
	vaddss	%xmm1, %xmm0, %xmm0
	vmovss	%xmm0, 44(%rdx)
	vmovss	48(%rsi), %xmm0         ## xmm0 = mem[0],zero,zero,zero
	vmovss	52(%rsi), %xmm1         ## xmm1 = mem[0],zero,zero,zero
	vmulss	(%rdi), %xmm0, %xmm0
	vmulss	16(%rdi), %xmm1, %xmm1
	vaddss	%xmm0, %xmm1, %xmm0
	vmovss	56(%rsi), %xmm1         ## xmm1 = mem[0],zero,zero,zero
	vmulss	32(%rdi), %xmm1, %xmm1
	vmovss	60(%rsi), %xmm2         ## xmm2 = mem[0],zero,zero,zero
	vmulss	48(%rdi), %xmm2, %xmm2
	vaddss	%xmm2, %xmm1, %xmm1
	vaddss	%xmm1, %xmm0, %xmm0
	vmovss	%xmm0, 48(%rdx)
	vmovss	48(%rsi), %xmm0         ## xmm0 = mem[0],zero,zero,zero
	vmovss	52(%rsi), %xmm1         ## xmm1 = mem[0],zero,zero,zero
	vmulss	4(%rdi), %xmm0, %xmm0
	vmulss	20(%rdi), %xmm1, %xmm1
	vaddss	%xmm0, %xmm1, %xmm0
	vmovss	56(%rsi), %xmm1         ## xmm1 = mem[0],zero,zero,zero
	vmulss	36(%rdi), %xmm1, %xmm1
	vmovss	60(%rsi), %xmm2         ## xmm2 = mem[0],zero,zero,zero
	vmulss	52(%rdi), %xmm2, %xmm2
	vaddss	%xmm2, %xmm1, %xmm1
	vaddss	%xmm1, %xmm0, %xmm0
	vmovss	%xmm0, 52(%rdx)
	vmovss	48(%rsi), %xmm0         ## xmm0 = mem[0],zero,zero,zero
	vmovss	52(%rsi), %xmm1         ## xmm1 = mem[0],zero,zero,zero
	vmulss	8(%rdi), %xmm0, %xmm0
	vmulss	24(%rdi), %xmm1, %xmm1
	vaddss	%xmm0, %xmm1, %xmm0
	vmovss	56(%rsi), %xmm1         ## xmm1 = mem[0],zero,zero,zero
	vmulss	40(%rdi), %xmm1, %xmm1
	vmovss	60(%rsi), %xmm2         ## xmm2 = mem[0],zero,zero,zero
	vmulss	56(%rdi), %xmm2, %xmm2
	vaddss	%xmm2, %xmm1, %xmm1
	vaddss	%xmm1, %xmm0, %xmm0
	vmovss	%xmm0, 56(%rdx)
	vmovss	48(%rsi), %xmm0         ## xmm0 = mem[0],zero,zero,zero
	vmovss	52(%rsi), %xmm1         ## xmm1 = mem[0],zero,zero,zero
	vmulss	12(%rdi), %xmm0, %xmm0
	vmulss	28(%rdi), %xmm1, %xmm1
	vaddss	%xmm0, %xmm1, %xmm0
	vmovss	56(%rsi), %xmm1         ## xmm1 = mem[0],zero,zero,zero
	vmulss	44(%rdi), %xmm1, %xmm1
	vmovss	60(%rsi), %xmm2         ## xmm2 = mem[0],zero,zero,zero
	vmulss	60(%rdi), %xmm2, %xmm2
	vaddss	%xmm2, %xmm1, %xmm1
	vaddss	%xmm1, %xmm0, %xmm0
	vmovss	%xmm0, 60(%rdx)
	popq	%rbp
	retq
	.cfi_endproc

	.globl	_mm_scalar_loop
	.align	4, 0x90
_mm_scalar_loop:                        ## @mm_scalar_loop
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
	xorl	%eax, %eax
	.align	4, 0x90
LBB5_1:                                 ## %.preheader
                                        ## =>This Inner Loop Header: Depth=1
	vmovss	(%rsi,%rax), %xmm0      ## xmm0 = mem[0],zero,zero,zero
	vmovss	4(%rsi,%rax), %xmm1     ## xmm1 = mem[0],zero,zero,zero
	vmulss	(%rdi), %xmm0, %xmm0
	vmulss	16(%rdi), %xmm1, %xmm1
	vaddss	%xmm0, %xmm1, %xmm0
	vmovss	8(%rsi,%rax), %xmm1     ## xmm1 = mem[0],zero,zero,zero
	vmulss	32(%rdi), %xmm1, %xmm1
	vmovss	12(%rsi,%rax), %xmm2    ## xmm2 = mem[0],zero,zero,zero
	vmulss	48(%rdi), %xmm2, %xmm2
	vaddss	%xmm2, %xmm1, %xmm1
	vaddss	%xmm1, %xmm0, %xmm0
	vmovss	%xmm0, (%rdx,%rax)
	vmovss	(%rsi,%rax), %xmm0      ## xmm0 = mem[0],zero,zero,zero
	vmovss	4(%rsi,%rax), %xmm1     ## xmm1 = mem[0],zero,zero,zero
	vmulss	4(%rdi), %xmm0, %xmm0
	vmulss	20(%rdi), %xmm1, %xmm1
	vaddss	%xmm0, %xmm1, %xmm0
	vmovss	8(%rsi,%rax), %xmm1     ## xmm1 = mem[0],zero,zero,zero
	vmulss	36(%rdi), %xmm1, %xmm1
	vmovss	12(%rsi,%rax), %xmm2    ## xmm2 = mem[0],zero,zero,zero
	vmulss	52(%rdi), %xmm2, %xmm2
	vaddss	%xmm2, %xmm1, %xmm1
	vaddss	%xmm1, %xmm0, %xmm0
	vmovss	%xmm0, 4(%rdx,%rax)
	vmovss	(%rsi,%rax), %xmm0      ## xmm0 = mem[0],zero,zero,zero
	vmovss	4(%rsi,%rax), %xmm1     ## xmm1 = mem[0],zero,zero,zero
	vmulss	8(%rdi), %xmm0, %xmm0
	vmulss	24(%rdi), %xmm1, %xmm1
	vaddss	%xmm0, %xmm1, %xmm0
	vmovss	8(%rsi,%rax), %xmm1     ## xmm1 = mem[0],zero,zero,zero
	vmulss	40(%rdi), %xmm1, %xmm1
	vmovss	12(%rsi,%rax), %xmm2    ## xmm2 = mem[0],zero,zero,zero
	vmulss	56(%rdi), %xmm2, %xmm2
	vaddss	%xmm2, %xmm1, %xmm1
	vaddss	%xmm1, %xmm0, %xmm0
	vmovss	%xmm0, 8(%rdx,%rax)
	vmovss	(%rsi,%rax), %xmm0      ## xmm0 = mem[0],zero,zero,zero
	vmovss	4(%rsi,%rax), %xmm1     ## xmm1 = mem[0],zero,zero,zero
	vmulss	12(%rdi), %xmm0, %xmm0
	vmulss	28(%rdi), %xmm1, %xmm1
	vaddss	%xmm0, %xmm1, %xmm0
	vmovss	8(%rsi,%rax), %xmm1     ## xmm1 = mem[0],zero,zero,zero
	vmulss	44(%rdi), %xmm1, %xmm1
	vmovss	12(%rsi,%rax), %xmm2    ## xmm2 = mem[0],zero,zero,zero
	vmulss	60(%rdi), %xmm2, %xmm2
	vaddss	%xmm2, %xmm1, %xmm1
	vaddss	%xmm1, %xmm0, %xmm0
	vmovss	%xmm0, 12(%rdx,%rax)
	addq	$16, %rax
	cmpq	$64, %rax
	jne	LBB5_1
## BB#2:
	popq	%rbp
	retq
	.cfi_endproc

	.globl	_test_mm_vector_256
	.align	4, 0x90
_test_mm_vector_256:                    ## @test_mm_vector_256
	.cfi_startproc
## BB#0:
	pushq	%rbp
Ltmp18:
	.cfi_def_cfa_offset 16
Ltmp19:
	.cfi_offset %rbp, -16
	movq	%rsp, %rbp
Ltmp20:
	.cfi_def_cfa_register %rbp
	xorl	%eax, %eax
	.align	4, 0x90
LBB6_1:                                 ## =>This Inner Loop Header: Depth=1
	vmovups	(%rdi,%rax,4), %ymm1
	vmovups	32(%rdi,%rax,4), %ymm0
	vbroadcastss	64(%rdi,%rax,4), %xmm2
	vbroadcastss	84(%rdi,%rax,4), %xmm3
	vinsertf128	$1, %xmm2, %ymm3, %ymm2
	vmulps	%ymm1, %ymm2, %ymm2
	vbroadcastss	72(%rdi,%rax,4), %xmm3
	vbroadcastss	92(%rdi,%rax,4), %xmm4
	vinsertf128	$1, %xmm3, %ymm4, %ymm3
	vaddps	%ymm2, %ymm3, %ymm2
	vbroadcastss	68(%rdi,%rax,4), %ymm3
	vbroadcastss	80(%rdi,%rax,4), %ymm4
	vinsertf128	$1, %xmm4, %ymm3, %ymm3
	vmulps	%ymm1, %ymm3, %ymm3
	vbroadcastss	88(%rdi,%rax,4), %ymm5
	vinsertf128	$1, %xmm5, %ymm4, %ymm4
	vmulps	%ymm0, %ymm4, %ymm4
	vaddps	%ymm3, %ymm4, %ymm3
	vperm2f128	$1, %ymm0, %ymm3, %ymm3 ## ymm3 = ymm3[2,3,0,1]
	vaddps	%ymm3, %ymm2, %ymm2
	vmovups	%ymm2, 128(%rdi,%rax,4)
	vbroadcastss	96(%rdi,%rax,4), %xmm2
	vbroadcastss	116(%rdi,%rax,4), %xmm3
	vinsertf128	$1, %xmm2, %ymm3, %ymm2
	vmulps	%ymm1, %ymm2, %ymm2
	vbroadcastss	104(%rdi,%rax,4), %xmm3
	vbroadcastss	124(%rdi,%rax,4), %xmm4
	vinsertf128	$1, %xmm3, %ymm4, %ymm3
	vaddps	%ymm2, %ymm3, %ymm2
	vbroadcastss	100(%rdi,%rax,4), %ymm3
	vbroadcastss	112(%rdi,%rax,4), %ymm4
	vinsertf128	$1, %xmm4, %ymm3, %ymm3
	vmulps	%ymm1, %ymm3, %ymm1
	vbroadcastss	120(%rdi,%rax,4), %ymm3
	vinsertf128	$1, %xmm3, %ymm4, %ymm3
	vmulps	%ymm0, %ymm3, %ymm0
	vaddps	%ymm1, %ymm0, %ymm0
	vperm2f128	$1, %ymm0, %ymm0, %ymm0 ## ymm0 = ymm0[2,3,0,1]
	vaddps	%ymm0, %ymm2, %ymm0
	vmovups	%ymm0, 160(%rdi,%rax,4)
	addq	$16, %rax
	cmpq	$1599999968, %rax       ## imm = 0x5F5E0FE0
	jl	LBB6_1
## BB#2:
	popq	%rbp
	vzeroupper
	retq
	.cfi_endproc

	.globl	_test_mm_vector_128
	.align	4, 0x90
_test_mm_vector_128:                    ## @test_mm_vector_128
	.cfi_startproc
## BB#0:
	pushq	%rbp
Ltmp21:
	.cfi_def_cfa_offset 16
Ltmp22:
	.cfi_offset %rbp, -16
	movq	%rsp, %rbp
Ltmp23:
	.cfi_def_cfa_register %rbp
	xorl	%eax, %eax
	.align	4, 0x90
LBB7_1:                                 ## =>This Inner Loop Header: Depth=1
	vmovaps	(%rdi,%rax,4), %xmm3
	vmovaps	16(%rdi,%rax,4), %xmm2
	vmovaps	32(%rdi,%rax,4), %xmm1
	vmovaps	48(%rdi,%rax,4), %xmm0
	vbroadcastss	64(%rdi,%rax,4), %xmm4
	vmulps	%xmm3, %xmm4, %xmm4
	vbroadcastss	68(%rdi,%rax,4), %xmm5
	vmulps	%xmm2, %xmm5, %xmm5
	vaddps	%xmm4, %xmm5, %xmm4
	vbroadcastss	72(%rdi,%rax,4), %xmm5
	vmulps	%xmm1, %xmm5, %xmm5
	vbroadcastss	76(%rdi,%rax,4), %xmm6
	vmulps	%xmm0, %xmm6, %xmm6
	vaddps	%xmm6, %xmm5, %xmm5
	vaddps	%xmm5, %xmm4, %xmm8
	vbroadcastss	80(%rdi,%rax,4), %xmm5
	vmulps	%xmm3, %xmm5, %xmm5
	vbroadcastss	84(%rdi,%rax,4), %xmm6
	vmulps	%xmm2, %xmm6, %xmm6
	vaddps	%xmm5, %xmm6, %xmm5
	vbroadcastss	88(%rdi,%rax,4), %xmm6
	vmulps	%xmm1, %xmm6, %xmm6
	vbroadcastss	92(%rdi,%rax,4), %xmm7
	vmulps	%xmm0, %xmm7, %xmm7
	vaddps	%xmm7, %xmm6, %xmm6
	vaddps	%xmm6, %xmm5, %xmm5
	vbroadcastss	96(%rdi,%rax,4), %xmm6
	vmulps	%xmm3, %xmm6, %xmm6
	vbroadcastss	100(%rdi,%rax,4), %xmm7
	vmulps	%xmm2, %xmm7, %xmm7
	vaddps	%xmm6, %xmm7, %xmm6
	vbroadcastss	104(%rdi,%rax,4), %xmm7
	vmulps	%xmm1, %xmm7, %xmm7
	vbroadcastss	108(%rdi,%rax,4), %xmm4
	vmulps	%xmm0, %xmm4, %xmm4
	vaddps	%xmm4, %xmm7, %xmm4
	vaddps	%xmm4, %xmm6, %xmm4
	vbroadcastss	112(%rdi,%rax,4), %xmm6
	vmulps	%xmm3, %xmm6, %xmm3
	vbroadcastss	116(%rdi,%rax,4), %xmm6
	vmulps	%xmm2, %xmm6, %xmm2
	vaddps	%xmm3, %xmm2, %xmm2
	vbroadcastss	120(%rdi,%rax,4), %xmm3
	vmulps	%xmm1, %xmm3, %xmm1
	vbroadcastss	124(%rdi,%rax,4), %xmm3
	vmulps	%xmm0, %xmm3, %xmm0
	vaddps	%xmm0, %xmm1, %xmm0
	vaddps	%xmm0, %xmm2, %xmm0
	vmovaps	%xmm8, 128(%rdi,%rax,4)
	vmovaps	%xmm5, 144(%rdi,%rax,4)
	vmovaps	%xmm4, 160(%rdi,%rax,4)
	vmovaps	%xmm0, 176(%rdi,%rax,4)
	addq	$16, %rax
	cmpq	$1599999968, %rax       ## imm = 0x5F5E0FE0
	jl	LBB7_1
## BB#2:
	popq	%rbp
	retq
	.cfi_endproc

	.globl	_test_mm_scalar_unrolled
	.align	4, 0x90
_test_mm_scalar_unrolled:               ## @test_mm_scalar_unrolled
	.cfi_startproc
## BB#0:
	pushq	%rbp
Ltmp24:
	.cfi_def_cfa_offset 16
Ltmp25:
	.cfi_offset %rbp, -16
	movq	%rsp, %rbp
Ltmp26:
	.cfi_def_cfa_register %rbp
	pushq	%r14
	pushq	%rbx
Ltmp27:
	.cfi_offset %rbx, -32
Ltmp28:
	.cfi_offset %r14, -24
	movq	%rdi, %rbx
	xorl	%r14d, %r14d
	.align	4, 0x90
LBB8_1:                                 ## =>This Inner Loop Header: Depth=1
	movq	%rbx, %rdx
	subq	$-128, %rdx
	movq	%rbx, %rdi
	leaq	64(%rbx), %rbx
	movq	%rbx, %rsi
	callq	_mm_scalar_unrolled
	addq	$16, %r14
	cmpq	$1599999968, %r14       ## imm = 0x5F5E0FE0
	jl	LBB8_1
## BB#2:
	popq	%rbx
	popq	%r14
	popq	%rbp
	retq
	.cfi_endproc

	.globl	_test_mm_scalar_loop
	.align	4, 0x90
_test_mm_scalar_loop:                   ## @test_mm_scalar_loop
	.cfi_startproc
## BB#0:
	pushq	%rbp
Ltmp29:
	.cfi_def_cfa_offset 16
Ltmp30:
	.cfi_offset %rbp, -16
	movq	%rsp, %rbp
Ltmp31:
	.cfi_def_cfa_register %rbp
	pushq	%r14
	pushq	%rbx
Ltmp32:
	.cfi_offset %rbx, -32
Ltmp33:
	.cfi_offset %r14, -24
	movq	%rdi, %rbx
	xorl	%r14d, %r14d
	.align	4, 0x90
LBB9_1:                                 ## =>This Inner Loop Header: Depth=1
	movq	%rbx, %rdx
	subq	$-128, %rdx
	movq	%rbx, %rdi
	leaq	64(%rbx), %rbx
	movq	%rbx, %rsi
	callq	_mm_scalar_loop
	addq	$16, %r14
	cmpq	$1599999968, %r14       ## imm = 0x5F5E0FE0
	jl	LBB9_1
## BB#2:
	popq	%rbx
	popq	%r14
	popq	%rbp
	retq
	.cfi_endproc

	.section	__TEXT,__const
	.align	5
LCPI10_0:
	.long	1120534528              ## float 1.010000e+02
	.long	1122500608              ## float 1.160000e+02
	.long	1124270080              ## float 1.310000e+02
	.long	1125253120              ## float 1.460000e+02
	.long	1108869120              ## float 3.800000e+01
	.long	1110179840              ## float 4.300000e+01
	.long	1111490560              ## float 4.800000e+01
	.long	1112801280              ## float 5.300000e+01
LCPI10_1:
	.long	1132691456              ## float 2.630000e+02
	.long	1134067712              ## float 3.050000e+02
	.long	1135443968              ## float 3.470000e+02
	.long	1136820224              ## float 3.890000e+02
	.long	1126432768              ## float 1.640000e+02
	.long	1128529920              ## float 1.960000e+02
	.long	1130627072              ## float 2.280000e+02
	.long	1132593152              ## float 2.600000e+02
	.section	__TEXT,__literal8,8byte_literals
	.align	3
LCPI10_2:
	.quad	4636807660098813952     ## double 101
LCPI10_3:
	.quad	4637863191261478912     ## double 116
LCPI10_4:
	.quad	4638813169307877376     ## double 131
LCPI10_5:
	.quad	4639340934889209856     ## double 146
LCPI10_6:
	.quad	4630544841867001856     ## double 38
LCPI10_7:
	.quad	4631248529308778496     ## double 43
LCPI10_8:
	.quad	4631952216750555136     ## double 48
LCPI10_9:
	.quad	4632655904192331776     ## double 53
LCPI10_10:
	.quad	4643334361121292288     ## double 263
LCPI10_11:
	.quad	4644073232935157760     ## double 305
LCPI10_12:
	.quad	4644812104749023232     ## double 347
LCPI10_13:
	.quad	4645550976562888704     ## double 389
LCPI10_14:
	.quad	4639974253586808832     ## double 164
LCPI10_15:
	.quad	4641100153493651456     ## double 196
LCPI10_16:
	.quad	4642226053400494080     ## double 228
LCPI10_17:
	.quad	4643281584563159040     ## double 260
LCPI10_22:
	.quad	4633781804099174400     ## double 61
LCPI10_23:
	.quad	4634415122796773376     ## double 67
LCPI10_24:
	.quad	4634837335261839360     ## double 73
LCPI10_25:
	.quad	4635259547726905344     ## double 79
LCPI10_26:
	.quad	4640009437958897664     ## double 165
LCPI10_27:
	.quad	4640783494144851968     ## double 187
LCPI10_28:
	.quad	4641557550330806272     ## double 209
LCPI10_29:
	.quad	4642331606516760576     ## double 231
LCPI10_30:
	.quad	4643897311074713600     ## double 295
LCPI10_31:
	.quad	4644636182888579072     ## double 337
LCPI10_32:
	.quad	4645375054702444544     ## double 379
LCPI10_33:
	.quad	4646113926516310016     ## double 421
LCPI10_34:
	.quad	4645726898423332864     ## double 399
LCPI10_35:
	.quad	4646747245213908992     ## double 457
LCPI10_36:
	.quad	4647741203725418496     ## double 515
LCPI10_37:
	.quad	4648251377120706560     ## double 573
	.section	__TEXT,__literal16,16byte_literals
	.align	4
LCPI10_18:
	.long	1114898432              ## float 6.100000e+01
	.long	1116078080              ## float 6.700000e+01
	.long	1116864512              ## float 7.300000e+01
	.long	1117650944              ## float 7.900000e+01
LCPI10_19:
	.long	1126498304              ## float 1.650000e+02
	.long	1127940096              ## float 1.870000e+02
	.long	1129381888              ## float 2.090000e+02
	.long	1130823680              ## float 2.310000e+02
LCPI10_20:
	.long	1133740032              ## float 2.950000e+02
	.long	1135116288              ## float 3.370000e+02
	.long	1136492544              ## float 3.790000e+02
	.long	1137868800              ## float 4.210000e+02
LCPI10_21:
	.long	1137147904              ## float 3.990000e+02
	.long	1139048448              ## float 4.570000e+02
	.long	1140899840              ## float 5.150000e+02
	.long	1141850112              ## float 5.730000e+02
	.section	__TEXT,__text,regular,pure_instructions
	.globl	_unit_test
	.align	4, 0x90
_unit_test:                             ## @unit_test
	.cfi_startproc
## BB#0:                                ## %.preheader22
	pushq	%rbp
Ltmp34:
	.cfi_def_cfa_offset 16
Ltmp35:
	.cfi_offset %rbp, -16
	movq	%rsp, %rbp
Ltmp36:
	.cfi_def_cfa_register %rbp
	pushq	%r15
	pushq	%r14
	pushq	%r13
	pushq	%r12
	pushq	%rbx
	subq	$200, %rsp
Ltmp37:
	.cfi_offset %rbx, -56
Ltmp38:
	.cfi_offset %r12, -48
Ltmp39:
	.cfi_offset %r13, -40
Ltmp40:
	.cfi_offset %r14, -32
Ltmp41:
	.cfi_offset %r15, -24
	movq	___stack_chk_guard@GOTPCREL(%rip), %r13
	movq	(%r13), %r13
	movq	%r13, -48(%rbp)
	vmovups	l_unit_test.b+32(%rip), %ymm0
	vmovups	%ymm0, -80(%rbp)
	vmovups	l_unit_test.b(%rip), %ymm1
	vmovups	%ymm1, -112(%rbp)
	vmovups	%ymm0, -144(%rbp)
	vmovups	%ymm1, -176(%rbp)
	vxorps	%xmm0, %xmm0, %xmm0
	vmovaps	%xmm0, -240(%rbp)
	vmovaps	LCPI10_0(%rip), %ymm0   ## ymm0 = [1.010000e+02,1.160000e+02,1.310000e+02,1.460000e+02,3.800000e+01,4.300000e+01,4.800000e+01,5.300000e+01]
	vmovups	%ymm0, -240(%rbp)
	vmovaps	LCPI10_1(%rip), %ymm0   ## ymm0 = [2.630000e+02,3.050000e+02,3.470000e+02,3.890000e+02,1.640000e+02,1.960000e+02,2.280000e+02,2.600000e+02]
	vmovups	%ymm0, -208(%rbp)
	leaq	L_.str(%rip), %rbx
	vmovsd	LCPI10_2(%rip), %xmm0   ## xmm0 = mem[0],zero
	movb	$1, %al
	movq	%rbx, %rdi
	vzeroupper
	callq	_printf
	vmovsd	LCPI10_3(%rip), %xmm0   ## xmm0 = mem[0],zero
	movb	$1, %al
	movq	%rbx, %rdi
	callq	_printf
	vmovsd	LCPI10_4(%rip), %xmm0   ## xmm0 = mem[0],zero
	movb	$1, %al
	movq	%rbx, %rdi
	callq	_printf
	vmovsd	LCPI10_5(%rip), %xmm0   ## xmm0 = mem[0],zero
	movb	$1, %al
	movq	%rbx, %rdi
	callq	_printf
	movl	$10, %edi
	callq	_putchar
	vmovsd	LCPI10_6(%rip), %xmm0   ## xmm0 = mem[0],zero
	movb	$1, %al
	movq	%rbx, %rdi
	callq	_printf
	vmovsd	LCPI10_7(%rip), %xmm0   ## xmm0 = mem[0],zero
	movb	$1, %al
	movq	%rbx, %rdi
	callq	_printf
	vmovsd	LCPI10_8(%rip), %xmm0   ## xmm0 = mem[0],zero
	movb	$1, %al
	movq	%rbx, %rdi
	callq	_printf
	vmovsd	LCPI10_9(%rip), %xmm0   ## xmm0 = mem[0],zero
	movb	$1, %al
	movq	%rbx, %rdi
	callq	_printf
	movl	$10, %edi
	callq	_putchar
	vmovsd	LCPI10_10(%rip), %xmm0  ## xmm0 = mem[0],zero
	movb	$1, %al
	movq	%rbx, %rdi
	callq	_printf
	vmovsd	LCPI10_11(%rip), %xmm0  ## xmm0 = mem[0],zero
	movb	$1, %al
	movq	%rbx, %rdi
	callq	_printf
	vmovsd	LCPI10_12(%rip), %xmm0  ## xmm0 = mem[0],zero
	movb	$1, %al
	movq	%rbx, %rdi
	callq	_printf
	vmovsd	LCPI10_13(%rip), %xmm0  ## xmm0 = mem[0],zero
	movb	$1, %al
	movq	%rbx, %rdi
	callq	_printf
	movl	$10, %edi
	callq	_putchar
	vmovsd	LCPI10_14(%rip), %xmm0  ## xmm0 = mem[0],zero
	movb	$1, %al
	movq	%rbx, %rdi
	callq	_printf
	vmovsd	LCPI10_15(%rip), %xmm0  ## xmm0 = mem[0],zero
	movb	$1, %al
	movq	%rbx, %rdi
	callq	_printf
	vmovsd	LCPI10_16(%rip), %xmm0  ## xmm0 = mem[0],zero
	movb	$1, %al
	movq	%rbx, %rdi
	callq	_printf
	vmovsd	LCPI10_17(%rip), %xmm0  ## xmm0 = mem[0],zero
	movb	$1, %al
	movq	%rbx, %rdi
	callq	_printf
	movl	$10, %edi
	callq	_putchar
	movl	$10, %edi
	callq	_putchar
	vmovaps	LCPI10_18(%rip), %xmm0  ## xmm0 = [6.100000e+01,6.700000e+01,7.300000e+01,7.900000e+01]
	vmovaps	%xmm0, -240(%rbp)
	vmovaps	LCPI10_19(%rip), %xmm0  ## xmm0 = [1.650000e+02,1.870000e+02,2.090000e+02,2.310000e+02]
	vmovaps	%xmm0, -224(%rbp)
	vmovaps	LCPI10_20(%rip), %xmm0  ## xmm0 = [2.950000e+02,3.370000e+02,3.790000e+02,4.210000e+02]
	vmovaps	%xmm0, -208(%rbp)
	vmovaps	LCPI10_21(%rip), %xmm0  ## xmm0 = [3.990000e+02,4.570000e+02,5.150000e+02,5.730000e+02]
	vmovaps	%xmm0, -192(%rbp)
	vmovsd	LCPI10_22(%rip), %xmm0  ## xmm0 = mem[0],zero
	movb	$1, %al
	movq	%rbx, %rdi
	callq	_printf
	vmovsd	LCPI10_23(%rip), %xmm0  ## xmm0 = mem[0],zero
	movb	$1, %al
	movq	%rbx, %rdi
	callq	_printf
	vmovsd	LCPI10_24(%rip), %xmm0  ## xmm0 = mem[0],zero
	movb	$1, %al
	movq	%rbx, %rdi
	callq	_printf
	vmovsd	LCPI10_25(%rip), %xmm0  ## xmm0 = mem[0],zero
	movb	$1, %al
	movq	%rbx, %rdi
	callq	_printf
	movl	$10, %edi
	callq	_putchar
	vmovsd	LCPI10_26(%rip), %xmm0  ## xmm0 = mem[0],zero
	movb	$1, %al
	movq	%rbx, %rdi
	callq	_printf
	vmovsd	LCPI10_27(%rip), %xmm0  ## xmm0 = mem[0],zero
	movb	$1, %al
	movq	%rbx, %rdi
	callq	_printf
	vmovsd	LCPI10_28(%rip), %xmm0  ## xmm0 = mem[0],zero
	movb	$1, %al
	movq	%rbx, %rdi
	callq	_printf
	vmovsd	LCPI10_29(%rip), %xmm0  ## xmm0 = mem[0],zero
	movb	$1, %al
	movq	%rbx, %rdi
	callq	_printf
	movl	$10, %edi
	callq	_putchar
	vmovsd	LCPI10_30(%rip), %xmm0  ## xmm0 = mem[0],zero
	movb	$1, %al
	movq	%rbx, %rdi
	callq	_printf
	vmovsd	LCPI10_31(%rip), %xmm0  ## xmm0 = mem[0],zero
	movb	$1, %al
	movq	%rbx, %rdi
	callq	_printf
	vmovsd	LCPI10_32(%rip), %xmm0  ## xmm0 = mem[0],zero
	movb	$1, %al
	movq	%rbx, %rdi
	callq	_printf
	vmovsd	LCPI10_33(%rip), %xmm0  ## xmm0 = mem[0],zero
	movb	$1, %al
	movq	%rbx, %rdi
	callq	_printf
	movl	$10, %edi
	callq	_putchar
	vmovsd	LCPI10_34(%rip), %xmm0  ## xmm0 = mem[0],zero
	movb	$1, %al
	movq	%rbx, %rdi
	callq	_printf
	vmovsd	LCPI10_35(%rip), %xmm0  ## xmm0 = mem[0],zero
	movb	$1, %al
	movq	%rbx, %rdi
	callq	_printf
	vmovsd	LCPI10_36(%rip), %xmm0  ## xmm0 = mem[0],zero
	movb	$1, %al
	movq	%rbx, %rdi
	callq	_printf
	vmovsd	LCPI10_37(%rip), %xmm0  ## xmm0 = mem[0],zero
	movb	$1, %al
	movq	%rbx, %rdi
	callq	_printf
	movl	$10, %edi
	callq	_putchar
	movl	$10, %edi
	callq	_putchar
	leaq	-112(%rbp), %r14
	leaq	-176(%rbp), %r15
	leaq	-240(%rbp), %r12
	movq	%r14, %rdi
	movq	%r15, %rsi
	movq	%r12, %rdx
	callq	_mm_scalar_unrolled
	vxorps	%xmm0, %xmm0, %xmm0
	vcvtss2sd	-240(%rbp), %xmm0, %xmm0
	movb	$1, %al
	movq	%rbx, %rdi
	callq	_printf
	vxorps	%xmm0, %xmm0, %xmm0
	vcvtss2sd	-236(%rbp), %xmm0, %xmm0
	movb	$1, %al
	movq	%rbx, %rdi
	callq	_printf
	vxorps	%xmm0, %xmm0, %xmm0
	vcvtss2sd	-232(%rbp), %xmm0, %xmm0
	movb	$1, %al
	movq	%rbx, %rdi
	callq	_printf
	vxorps	%xmm0, %xmm0, %xmm0
	vcvtss2sd	-228(%rbp), %xmm0, %xmm0
	movb	$1, %al
	movq	%rbx, %rdi
	callq	_printf
	movl	$10, %edi
	callq	_putchar
	vxorps	%xmm0, %xmm0, %xmm0
	vcvtss2sd	-224(%rbp), %xmm0, %xmm0
	movb	$1, %al
	movq	%rbx, %rdi
	callq	_printf
	vxorps	%xmm0, %xmm0, %xmm0
	vcvtss2sd	-220(%rbp), %xmm0, %xmm0
	movb	$1, %al
	movq	%rbx, %rdi
	callq	_printf
	vxorps	%xmm0, %xmm0, %xmm0
	vcvtss2sd	-216(%rbp), %xmm0, %xmm0
	movb	$1, %al
	movq	%rbx, %rdi
	callq	_printf
	vxorps	%xmm0, %xmm0, %xmm0
	vcvtss2sd	-212(%rbp), %xmm0, %xmm0
	movb	$1, %al
	movq	%rbx, %rdi
	callq	_printf
	movl	$10, %edi
	callq	_putchar
	vxorps	%xmm0, %xmm0, %xmm0
	vcvtss2sd	-208(%rbp), %xmm0, %xmm0
	movb	$1, %al
	movq	%rbx, %rdi
	callq	_printf
	vxorps	%xmm0, %xmm0, %xmm0
	vcvtss2sd	-204(%rbp), %xmm0, %xmm0
	movb	$1, %al
	movq	%rbx, %rdi
	callq	_printf
	vxorps	%xmm0, %xmm0, %xmm0
	vcvtss2sd	-200(%rbp), %xmm0, %xmm0
	movb	$1, %al
	movq	%rbx, %rdi
	callq	_printf
	vxorps	%xmm0, %xmm0, %xmm0
	vcvtss2sd	-196(%rbp), %xmm0, %xmm0
	movb	$1, %al
	movq	%rbx, %rdi
	callq	_printf
	movl	$10, %edi
	callq	_putchar
	vxorps	%xmm0, %xmm0, %xmm0
	vcvtss2sd	-192(%rbp), %xmm0, %xmm0
	movb	$1, %al
	movq	%rbx, %rdi
	callq	_printf
	vxorps	%xmm0, %xmm0, %xmm0
	vcvtss2sd	-188(%rbp), %xmm0, %xmm0
	movb	$1, %al
	movq	%rbx, %rdi
	callq	_printf
	vxorps	%xmm0, %xmm0, %xmm0
	vcvtss2sd	-184(%rbp), %xmm0, %xmm0
	movb	$1, %al
	movq	%rbx, %rdi
	callq	_printf
	vxorps	%xmm0, %xmm0, %xmm0
	vcvtss2sd	-180(%rbp), %xmm0, %xmm0
	movb	$1, %al
	movq	%rbx, %rdi
	callq	_printf
	movl	$10, %edi
	callq	_putchar
	movl	$10, %edi
	callq	_putchar
	movq	%r14, %rdi
	movq	%r15, %rsi
	movq	%r12, %rdx
	callq	_mm_scalar_loop
	vxorps	%xmm0, %xmm0, %xmm0
	vcvtss2sd	-240(%rbp), %xmm0, %xmm0
	movb	$1, %al
	movq	%rbx, %rdi
	callq	_printf
	vxorps	%xmm0, %xmm0, %xmm0
	vcvtss2sd	-236(%rbp), %xmm0, %xmm0
	movb	$1, %al
	movq	%rbx, %rdi
	callq	_printf
	vxorps	%xmm0, %xmm0, %xmm0
	vcvtss2sd	-232(%rbp), %xmm0, %xmm0
	movb	$1, %al
	movq	%rbx, %rdi
	callq	_printf
	vxorps	%xmm0, %xmm0, %xmm0
	vcvtss2sd	-228(%rbp), %xmm0, %xmm0
	movb	$1, %al
	movq	%rbx, %rdi
	callq	_printf
	movl	$10, %edi
	callq	_putchar
	vxorps	%xmm0, %xmm0, %xmm0
	vcvtss2sd	-224(%rbp), %xmm0, %xmm0
	movb	$1, %al
	movq	%rbx, %rdi
	callq	_printf
	vxorps	%xmm0, %xmm0, %xmm0
	vcvtss2sd	-220(%rbp), %xmm0, %xmm0
	movb	$1, %al
	movq	%rbx, %rdi
	callq	_printf
	vxorps	%xmm0, %xmm0, %xmm0
	vcvtss2sd	-216(%rbp), %xmm0, %xmm0
	movb	$1, %al
	movq	%rbx, %rdi
	callq	_printf
	vxorps	%xmm0, %xmm0, %xmm0
	vcvtss2sd	-212(%rbp), %xmm0, %xmm0
	movb	$1, %al
	movq	%rbx, %rdi
	callq	_printf
	movl	$10, %edi
	callq	_putchar
	vxorps	%xmm0, %xmm0, %xmm0
	vcvtss2sd	-208(%rbp), %xmm0, %xmm0
	movb	$1, %al
	movq	%rbx, %rdi
	callq	_printf
	vxorps	%xmm0, %xmm0, %xmm0
	vcvtss2sd	-204(%rbp), %xmm0, %xmm0
	movb	$1, %al
	movq	%rbx, %rdi
	callq	_printf
	vxorps	%xmm0, %xmm0, %xmm0
	vcvtss2sd	-200(%rbp), %xmm0, %xmm0
	movb	$1, %al
	movq	%rbx, %rdi
	callq	_printf
	vxorps	%xmm0, %xmm0, %xmm0
	vcvtss2sd	-196(%rbp), %xmm0, %xmm0
	movb	$1, %al
	movq	%rbx, %rdi
	callq	_printf
	movl	$10, %edi
	callq	_putchar
	vxorps	%xmm0, %xmm0, %xmm0
	vcvtss2sd	-192(%rbp), %xmm0, %xmm0
	movb	$1, %al
	movq	%rbx, %rdi
	callq	_printf
	vxorps	%xmm0, %xmm0, %xmm0
	vcvtss2sd	-188(%rbp), %xmm0, %xmm0
	movb	$1, %al
	movq	%rbx, %rdi
	callq	_printf
	vxorps	%xmm0, %xmm0, %xmm0
	vcvtss2sd	-184(%rbp), %xmm0, %xmm0
	movb	$1, %al
	movq	%rbx, %rdi
	callq	_printf
	vxorps	%xmm0, %xmm0, %xmm0
	vcvtss2sd	-180(%rbp), %xmm0, %xmm0
	movb	$1, %al
	movq	%rbx, %rdi
	callq	_printf
	movl	$10, %edi
	callq	_putchar
	cmpq	-48(%rbp), %r13
	jne	LBB10_2
## BB#1:                                ## %.preheader22
	addq	$200, %rsp
	popq	%rbx
	popq	%r12
	popq	%r13
	popq	%r14
	popq	%r15
	popq	%rbp
	retq
LBB10_2:                                ## %.preheader22
	callq	___stack_chk_fail
	.cfi_endproc

	.section	__TEXT,__literal16,16byte_literals
	.align	4
LCPI11_0:
	.long	0                       ## 0x0
	.long	1                       ## 0x1
	.long	2                       ## 0x2
	.long	3                       ## 0x3
LCPI11_1:
	.long	4                       ## 0x4
	.long	5                       ## 0x5
	.long	6                       ## 0x6
	.long	7                       ## 0x7
LCPI11_2:
	.long	8                       ## 0x8
	.long	9                       ## 0x9
	.long	10                      ## 0xa
	.long	11                      ## 0xb
LCPI11_3:
	.long	12                      ## 0xc
	.long	13                      ## 0xd
	.long	14                      ## 0xe
	.long	15                      ## 0xf
LCPI11_4:
	.long	16                      ## 0x10
	.long	17                      ## 0x11
	.long	18                      ## 0x12
	.long	19                      ## 0x13
LCPI11_5:
	.long	20                      ## 0x14
	.long	21                      ## 0x15
	.long	22                      ## 0x16
	.long	23                      ## 0x17
LCPI11_6:
	.long	24                      ## 0x18
	.long	25                      ## 0x19
	.long	26                      ## 0x1a
	.long	27                      ## 0x1b
LCPI11_7:
	.long	28                      ## 0x1c
	.long	29                      ## 0x1d
	.long	30                      ## 0x1e
	.long	31                      ## 0x1f
	.section	__TEXT,__literal8,8byte_literals
	.align	3
LCPI11_8:
	.quad	4517329193108106637     ## double 9.9999999999999995E-7
LCPI11_9:
	.quad	-4706042843746669171    ## double -9.9999999999999995E-7
	.section	__TEXT,__text,regular,pure_instructions
	.globl	_main
	.align	4, 0x90
_main:                                  ## @main
	.cfi_startproc
## BB#0:                                ## %min.iters.checked
	pushq	%rbp
Ltmp42:
	.cfi_def_cfa_offset 16
Ltmp43:
	.cfi_offset %rbp, -16
	movq	%rsp, %rbp
Ltmp44:
	.cfi_def_cfa_register %rbp
	pushq	%r15
	pushq	%r14
	pushq	%r13
	pushq	%r12
	pushq	%rbx
	subq	$40, %rsp
Ltmp45:
	.cfi_offset %rbx, -56
Ltmp46:
	.cfi_offset %r12, -48
Ltmp47:
	.cfi_offset %r13, -40
Ltmp48:
	.cfi_offset %r14, -32
Ltmp49:
	.cfi_offset %r15, -24
	leaq	-64(%rbp), %rdi
	movabsq	$6400000000, %rdx       ## imm = 0x17D784000
	movl	$32, %esi
	callq	_posix_memalign
	## InlineAsm Start
	rdtsc
	## InlineAsm End
	movl	%edx, %r15d
	movl	%eax, %r14d
	xorl	%ebx, %ebx
	leaq	-56(%rbp), %rdi
	xorl	%esi, %esi
	callq	_gettimeofday
	movq	-56(%rbp), %rax
	movl	-48(%rbp), %ecx
	movq	-64(%rbp), %rdx
	vmovdqa	LCPI11_0(%rip), %xmm8   ## xmm8 = [0,1,2,3]
	vmovdqa	LCPI11_1(%rip), %xmm10  ## xmm10 = [4,5,6,7]
	vmovdqa	LCPI11_2(%rip), %xmm11  ## xmm11 = [8,9,10,11]
	vmovdqa	LCPI11_3(%rip), %xmm12  ## xmm12 = [12,13,14,15]
	vmovdqa	LCPI11_4(%rip), %xmm4   ## xmm4 = [16,17,18,19]
	vmovdqa	LCPI11_5(%rip), %xmm5   ## xmm5 = [20,21,22,23]
	vmovdqa	LCPI11_6(%rip), %xmm6   ## xmm6 = [24,25,26,27]
	vmovdqa	LCPI11_7(%rip), %xmm7   ## xmm7 = [28,29,30,31]
	.align	4, 0x90
LBB11_1:                                ## %vector.body
                                        ## =>This Inner Loop Header: Depth=1
	vmovd	%ebx, %xmm0
	vpshufd	$0, %xmm0, %xmm0        ## xmm0 = xmm0[0,0,0,0]
	vpaddd	%xmm8, %xmm0, %xmm9
	vpaddd	%xmm10, %xmm0, %xmm1
	vinsertf128	$1, %xmm1, %ymm9, %ymm1
	vpaddd	%xmm11, %xmm0, %xmm9
	vpaddd	%xmm12, %xmm0, %xmm2
	vinsertf128	$1, %xmm2, %ymm9, %ymm2
	vpaddd	%xmm4, %xmm0, %xmm9
	vpaddd	%xmm5, %xmm0, %xmm3
	vinsertf128	$1, %xmm3, %ymm9, %ymm3
	vpaddd	%xmm6, %xmm0, %xmm9
	vpaddd	%xmm7, %xmm0, %xmm0
	vinsertf128	$1, %xmm0, %ymm9, %ymm0
	vcvtdq2ps	%ymm1, %ymm1
	vcvtdq2ps	%ymm2, %ymm2
	vcvtdq2ps	%ymm3, %ymm3
	vcvtdq2ps	%ymm0, %ymm0
	vmovups	%ymm1, (%rdx,%rbx,4)
	vmovups	%ymm2, 32(%rdx,%rbx,4)
	vmovups	%ymm3, 64(%rdx,%rbx,4)
	vmovups	%ymm0, 96(%rdx,%rbx,4)
	addq	$32, %rbx
	cmpq	$1600000000, %rbx       ## imm = 0x5F5E1000
	jne	LBB11_1
## BB#2:                                ## %middle.block
	shlq	$32, %r15
	orq	%r14, %r15
	vxorps	%xmm0, %xmm0, %xmm0
	vcvtsi2sdq	%rax, %xmm0, %xmm0
	vmovsd	%xmm0, -72(%rbp)        ## 8-byte Spill
	vxorps	%xmm0, %xmm0, %xmm0
	vcvtsi2sdl	%ecx, %xmm0, %xmm0
	vmovsd	%xmm0, -80(%rbp)        ## 8-byte Spill
	## InlineAsm Start
	rdtsc
	## InlineAsm End
	shlq	$32, %rdx
	movl	%eax, %eax
	orq	%rdx, %rax
	movq	_profilers+96(%rip), %rcx
	subq	%r15, %rcx
	addq	%rax, %rcx
	movq	%rcx, _profilers+96(%rip)
	addl	$1600000000, _profilers+104(%rip) ## imm = 0x5F5E1000
	xorl	%ebx, %ebx
	leaq	-56(%rbp), %rdi
	xorl	%esi, %esi
	vzeroupper
	callq	_gettimeofday
	vxorps	%xmm0, %xmm0, %xmm0
	vcvtsi2sdq	-56(%rbp), %xmm0, %xmm0
	vcvtsi2sdl	-48(%rbp), %xmm0, %xmm1
	vmulsd	LCPI11_8(%rip), %xmm1, %xmm1
	vmovsd	-80(%rbp), %xmm2        ## 8-byte Reload
                                        ## xmm2 = mem[0],zero
	vmulsd	LCPI11_9(%rip), %xmm2, %xmm2
	vsubsd	-72(%rbp), %xmm2, %xmm2 ## 8-byte Folded Reload
	vaddsd	%xmm1, %xmm0, %xmm0
	vaddsd	%xmm0, %xmm2, %xmm0
	vaddsd	_profilers+112(%rip), %xmm0, %xmm0
	vmovsd	%xmm0, _profilers+112(%rip)
	leaq	L_.str.2(%rip), %rdi
	leaq	L_.str.3(%rip), %rsi
	movb	$1, %al
	callq	_printf
	## InlineAsm Start
	rdtsc
	## InlineAsm End
	movl	%edx, %r14d
	movl	%eax, %r15d
	leaq	-56(%rbp), %rdi
	xorl	%esi, %esi
	callq	_gettimeofday
	movq	-56(%rbp), %rax
	movl	-48(%rbp), %ecx
	movq	-64(%rbp), %rdx
	.align	4, 0x90
LBB11_3:                                ## =>This Inner Loop Header: Depth=1
	vmovaps	(%rdx,%rbx,4), %xmm3
	vmovaps	16(%rdx,%rbx,4), %xmm2
	vmovaps	32(%rdx,%rbx,4), %xmm1
	vmovaps	48(%rdx,%rbx,4), %xmm0
	vbroadcastss	64(%rdx,%rbx,4), %xmm4
	vmulps	%xmm3, %xmm4, %xmm4
	vbroadcastss	68(%rdx,%rbx,4), %xmm5
	vmulps	%xmm2, %xmm5, %xmm5
	vaddps	%xmm4, %xmm5, %xmm4
	vbroadcastss	72(%rdx,%rbx,4), %xmm5
	vmulps	%xmm1, %xmm5, %xmm5
	vbroadcastss	76(%rdx,%rbx,4), %xmm6
	vmulps	%xmm0, %xmm6, %xmm6
	vaddps	%xmm6, %xmm5, %xmm5
	vaddps	%xmm5, %xmm4, %xmm8
	vbroadcastss	80(%rdx,%rbx,4), %xmm5
	vmulps	%xmm3, %xmm5, %xmm5
	vbroadcastss	84(%rdx,%rbx,4), %xmm6
	vmulps	%xmm2, %xmm6, %xmm6
	vaddps	%xmm5, %xmm6, %xmm5
	vbroadcastss	88(%rdx,%rbx,4), %xmm6
	vmulps	%xmm1, %xmm6, %xmm6
	vbroadcastss	92(%rdx,%rbx,4), %xmm7
	vmulps	%xmm0, %xmm7, %xmm7
	vaddps	%xmm7, %xmm6, %xmm6
	vaddps	%xmm6, %xmm5, %xmm5
	vbroadcastss	96(%rdx,%rbx,4), %xmm6
	vmulps	%xmm3, %xmm6, %xmm6
	vbroadcastss	100(%rdx,%rbx,4), %xmm7
	vmulps	%xmm2, %xmm7, %xmm7
	vaddps	%xmm6, %xmm7, %xmm6
	vbroadcastss	104(%rdx,%rbx,4), %xmm7
	vmulps	%xmm1, %xmm7, %xmm7
	vbroadcastss	108(%rdx,%rbx,4), %xmm4
	vmulps	%xmm0, %xmm4, %xmm4
	vaddps	%xmm4, %xmm7, %xmm4
	vaddps	%xmm4, %xmm6, %xmm4
	vbroadcastss	112(%rdx,%rbx,4), %xmm6
	vmulps	%xmm3, %xmm6, %xmm3
	vbroadcastss	116(%rdx,%rbx,4), %xmm6
	vmulps	%xmm2, %xmm6, %xmm2
	vaddps	%xmm3, %xmm2, %xmm2
	vbroadcastss	120(%rdx,%rbx,4), %xmm3
	vmulps	%xmm1, %xmm3, %xmm1
	vbroadcastss	124(%rdx,%rbx,4), %xmm3
	vmulps	%xmm0, %xmm3, %xmm0
	vaddps	%xmm0, %xmm1, %xmm0
	vaddps	%xmm0, %xmm2, %xmm0
	vmovaps	%xmm8, 128(%rdx,%rbx,4)
	vmovaps	%xmm5, 144(%rdx,%rbx,4)
	vmovaps	%xmm4, 160(%rdx,%rbx,4)
	vmovaps	%xmm0, 176(%rdx,%rbx,4)
	addq	$16, %rbx
	cmpq	$1599999968, %rbx       ## imm = 0x5F5E0FE0
	jl	LBB11_3
## BB#4:                                ## %test_mm_vector_128.exit
	shlq	$32, %r14
	orq	%r15, %r14
	vxorps	%xmm0, %xmm0, %xmm0
	vcvtsi2sdq	%rax, %xmm0, %xmm0
	vmovsd	%xmm0, -72(%rbp)        ## 8-byte Spill
	vxorps	%xmm0, %xmm0, %xmm0
	vcvtsi2sdl	%ecx, %xmm0, %xmm0
	vmovsd	%xmm0, -80(%rbp)        ## 8-byte Spill
	## InlineAsm Start
	rdtsc
	## InlineAsm End
	shlq	$32, %rdx
	movl	%eax, %eax
	orq	%rdx, %rax
	movq	_profilers(%rip), %rcx
	subq	%r14, %rcx
	addq	%rax, %rcx
	movq	%rcx, _profilers(%rip)
	addl	$99999998, _profilers+8(%rip) ## imm = 0x5F5E0FE
	xorl	%ebx, %ebx
	leaq	-56(%rbp), %rdi
	xorl	%esi, %esi
	callq	_gettimeofday
	vxorps	%xmm0, %xmm0, %xmm0
	vcvtsi2sdq	-56(%rbp), %xmm0, %xmm0
	vcvtsi2sdl	-48(%rbp), %xmm0, %xmm1
	vmulsd	LCPI11_8(%rip), %xmm1, %xmm1
	vmovsd	-80(%rbp), %xmm2        ## 8-byte Reload
                                        ## xmm2 = mem[0],zero
	vmulsd	LCPI11_9(%rip), %xmm2, %xmm2
	vsubsd	-72(%rbp), %xmm2, %xmm2 ## 8-byte Folded Reload
	vaddsd	%xmm1, %xmm0, %xmm0
	vaddsd	%xmm0, %xmm2, %xmm0
	vaddsd	_profilers+16(%rip), %xmm0, %xmm0
	vmovsd	%xmm0, _profilers+16(%rip)
	leaq	L_.str.2(%rip), %rdi
	leaq	L_.str.4(%rip), %rsi
	movb	$1, %al
	callq	_printf
	## InlineAsm Start
	rdtsc
	## InlineAsm End
	movl	%edx, %r14d
	movl	%eax, %r15d
	leaq	-56(%rbp), %rdi
	xorl	%esi, %esi
	callq	_gettimeofday
	movq	-56(%rbp), %rax
	movl	-48(%rbp), %ecx
	movq	-64(%rbp), %rdx
	.align	4, 0x90
LBB11_5:                                ## =>This Inner Loop Header: Depth=1
	vmovups	(%rdx,%rbx,4), %ymm1
	vmovups	32(%rdx,%rbx,4), %ymm0
	vbroadcastss	64(%rdx,%rbx,4), %xmm2
	vbroadcastss	84(%rdx,%rbx,4), %xmm3
	vinsertf128	$1, %xmm2, %ymm3, %ymm2
	vmulps	%ymm1, %ymm2, %ymm2
	vbroadcastss	72(%rdx,%rbx,4), %xmm3
	vbroadcastss	92(%rdx,%rbx,4), %xmm4
	vinsertf128	$1, %xmm3, %ymm4, %ymm3
	vaddps	%ymm2, %ymm3, %ymm2
	vbroadcastss	68(%rdx,%rbx,4), %ymm3
	vbroadcastss	80(%rdx,%rbx,4), %ymm4
	vinsertf128	$1, %xmm4, %ymm3, %ymm3
	vmulps	%ymm1, %ymm3, %ymm3
	vbroadcastss	88(%rdx,%rbx,4), %ymm5
	vinsertf128	$1, %xmm5, %ymm4, %ymm4
	vmulps	%ymm0, %ymm4, %ymm4
	vaddps	%ymm3, %ymm4, %ymm3
	vperm2f128	$1, %ymm0, %ymm3, %ymm3 ## ymm3 = ymm3[2,3,0,1]
	vaddps	%ymm3, %ymm2, %ymm2
	vmovups	%ymm2, 128(%rdx,%rbx,4)
	vbroadcastss	96(%rdx,%rbx,4), %xmm2
	vbroadcastss	116(%rdx,%rbx,4), %xmm3
	vinsertf128	$1, %xmm2, %ymm3, %ymm2
	vmulps	%ymm1, %ymm2, %ymm2
	vbroadcastss	104(%rdx,%rbx,4), %xmm3
	vbroadcastss	124(%rdx,%rbx,4), %xmm4
	vinsertf128	$1, %xmm3, %ymm4, %ymm3
	vaddps	%ymm2, %ymm3, %ymm2
	vbroadcastss	100(%rdx,%rbx,4), %ymm3
	vbroadcastss	112(%rdx,%rbx,4), %ymm4
	vinsertf128	$1, %xmm4, %ymm3, %ymm3
	vmulps	%ymm1, %ymm3, %ymm1
	vbroadcastss	120(%rdx,%rbx,4), %ymm3
	vinsertf128	$1, %xmm3, %ymm4, %ymm3
	vmulps	%ymm0, %ymm3, %ymm0
	vaddps	%ymm1, %ymm0, %ymm0
	vperm2f128	$1, %ymm0, %ymm0, %ymm0 ## ymm0 = ymm0[2,3,0,1]
	vaddps	%ymm0, %ymm2, %ymm0
	vmovups	%ymm0, 160(%rdx,%rbx,4)
	addq	$16, %rbx
	cmpq	$1599999968, %rbx       ## imm = 0x5F5E0FE0
	jl	LBB11_5
## BB#6:                                ## %test_mm_vector_256.exit
	shlq	$32, %r14
	orq	%r15, %r14
	vxorps	%xmm0, %xmm0, %xmm0
	vcvtsi2sdq	%rax, %xmm0, %xmm0
	vmovsd	%xmm0, -72(%rbp)        ## 8-byte Spill
	vxorps	%xmm0, %xmm0, %xmm0
	vcvtsi2sdl	%ecx, %xmm0, %xmm0
	vmovsd	%xmm0, -80(%rbp)        ## 8-byte Spill
	## InlineAsm Start
	rdtsc
	## InlineAsm End
	shlq	$32, %rdx
	movl	%eax, %eax
	orq	%rdx, %rax
	movq	_profilers+24(%rip), %rcx
	subq	%r14, %rcx
	addq	%rax, %rcx
	movq	%rcx, _profilers+24(%rip)
	addl	$99999998, _profilers+32(%rip) ## imm = 0x5F5E0FE
	xorl	%r14d, %r14d
	leaq	-56(%rbp), %rdi
	xorl	%esi, %esi
	vzeroupper
	callq	_gettimeofday
	vxorps	%xmm0, %xmm0, %xmm0
	vcvtsi2sdq	-56(%rbp), %xmm0, %xmm0
	vcvtsi2sdl	-48(%rbp), %xmm0, %xmm1
	vmulsd	LCPI11_8(%rip), %xmm1, %xmm1
	vmovsd	-80(%rbp), %xmm2        ## 8-byte Reload
                                        ## xmm2 = mem[0],zero
	vmulsd	LCPI11_9(%rip), %xmm2, %xmm2
	vsubsd	-72(%rbp), %xmm2, %xmm2 ## 8-byte Folded Reload
	vaddsd	%xmm1, %xmm0, %xmm0
	vaddsd	%xmm0, %xmm2, %xmm0
	vaddsd	_profilers+40(%rip), %xmm0, %xmm0
	vmovsd	%xmm0, _profilers+40(%rip)
	leaq	L_.str.2(%rip), %rdi
	leaq	L_.str.5(%rip), %rsi
	movb	$1, %al
	callq	_printf
	## InlineAsm Start
	rdtsc
	## InlineAsm End
	movl	%edx, %r15d
	movl	%eax, %eax
	movq	%rax, -72(%rbp)         ## 8-byte Spill
	leaq	-56(%rbp), %rdi
	xorl	%esi, %esi
	callq	_gettimeofday
	movq	-56(%rbp), %r13
	movl	-48(%rbp), %r12d
	movq	-64(%rbp), %rbx
	.align	4, 0x90
LBB11_7:                                ## =>This Inner Loop Header: Depth=1
	movq	%rbx, %rdx
	subq	$-128, %rdx
	movq	%rbx, %rdi
	leaq	64(%rbx), %rbx
	movq	%rbx, %rsi
	callq	_mm_scalar_unrolled
	addq	$16, %r14
	cmpq	$1599999968, %r14       ## imm = 0x5F5E0FE0
	jl	LBB11_7
## BB#8:                                ## %test_mm_scalar_unrolled.exit
	shlq	$32, %r15
	addq	-72(%rbp), %r15         ## 8-byte Folded Reload
	vcvtsi2sdq	%r13, %xmm0, %xmm0
	vmovsd	%xmm0, -72(%rbp)        ## 8-byte Spill
	vxorps	%xmm0, %xmm0, %xmm0
	vcvtsi2sdl	%r12d, %xmm0, %xmm0
	vmovsd	%xmm0, -80(%rbp)        ## 8-byte Spill
	## InlineAsm Start
	rdtsc
	## InlineAsm End
	shlq	$32, %rdx
	movl	%eax, %eax
	orq	%rdx, %rax
	movq	_profilers+48(%rip), %rcx
	subq	%r15, %rcx
	addq	%rax, %rcx
	movq	%rcx, _profilers+48(%rip)
	addl	$99999998, _profilers+56(%rip) ## imm = 0x5F5E0FE
	xorl	%r14d, %r14d
	leaq	-56(%rbp), %rdi
	xorl	%esi, %esi
	callq	_gettimeofday
	vxorps	%xmm0, %xmm0, %xmm0
	vcvtsi2sdq	-56(%rbp), %xmm0, %xmm0
	vcvtsi2sdl	-48(%rbp), %xmm0, %xmm1
	vmulsd	LCPI11_8(%rip), %xmm1, %xmm1
	vmovsd	-80(%rbp), %xmm2        ## 8-byte Reload
                                        ## xmm2 = mem[0],zero
	vmulsd	LCPI11_9(%rip), %xmm2, %xmm2
	vsubsd	-72(%rbp), %xmm2, %xmm2 ## 8-byte Folded Reload
	vaddsd	%xmm1, %xmm0, %xmm0
	vaddsd	%xmm0, %xmm2, %xmm0
	vaddsd	_profilers+64(%rip), %xmm0, %xmm0
	vmovsd	%xmm0, _profilers+64(%rip)
	leaq	L_.str.2(%rip), %rdi
	leaq	L_.str.6(%rip), %rsi
	movb	$1, %al
	callq	_printf
	## InlineAsm Start
	rdtsc
	## InlineAsm End
	movl	%edx, %r15d
	movl	%eax, %eax
	movq	%rax, -72(%rbp)         ## 8-byte Spill
	leaq	-56(%rbp), %rdi
	xorl	%esi, %esi
	callq	_gettimeofday
	movq	-56(%rbp), %r13
	movl	-48(%rbp), %r12d
	movq	-64(%rbp), %rbx
	.align	4, 0x90
LBB11_9:                                ## =>This Inner Loop Header: Depth=1
	movq	%rbx, %rdx
	subq	$-128, %rdx
	movq	%rbx, %rdi
	leaq	64(%rbx), %rbx
	movq	%rbx, %rsi
	callq	_mm_scalar_loop
	addq	$16, %r14
	cmpq	$1599999968, %r14       ## imm = 0x5F5E0FE0
	jl	LBB11_9
## BB#10:                               ## %test_mm_scalar_loop.exit
	shlq	$32, %r15
	addq	-72(%rbp), %r15         ## 8-byte Folded Reload
	vcvtsi2sdq	%r13, %xmm0, %xmm0
	vmovsd	%xmm0, -72(%rbp)        ## 8-byte Spill
	vxorps	%xmm0, %xmm0, %xmm0
	vcvtsi2sdl	%r12d, %xmm0, %xmm0
	vmovsd	%xmm0, -80(%rbp)        ## 8-byte Spill
	## InlineAsm Start
	rdtsc
	## InlineAsm End
	shlq	$32, %rdx
	movl	%eax, %eax
	orq	%rdx, %rax
	movq	_profilers+72(%rip), %rcx
	subq	%r15, %rcx
	addq	%rax, %rcx
	movq	%rcx, _profilers+72(%rip)
	addl	$99999998, _profilers+80(%rip) ## imm = 0x5F5E0FE
	leaq	-56(%rbp), %rdi
	xorl	%esi, %esi
	callq	_gettimeofday
	vxorps	%xmm0, %xmm0, %xmm0
	vcvtsi2sdq	-56(%rbp), %xmm0, %xmm0
	vcvtsi2sdl	-48(%rbp), %xmm0, %xmm1
	vmulsd	LCPI11_8(%rip), %xmm1, %xmm1
	vmovsd	-80(%rbp), %xmm2        ## 8-byte Reload
                                        ## xmm2 = mem[0],zero
	vmulsd	LCPI11_9(%rip), %xmm2, %xmm2
	vsubsd	-72(%rbp), %xmm2, %xmm2 ## 8-byte Folded Reload
	vaddsd	%xmm1, %xmm0, %xmm0
	vaddsd	%xmm0, %xmm2, %xmm0
	vaddsd	_profilers+88(%rip), %xmm0, %xmm0
	vmovsd	%xmm0, _profilers+88(%rip)
	leaq	L_.str.2(%rip), %rdi
	leaq	L_.str.7(%rip), %rsi
	movb	$1, %al
	callq	_printf
	xorl	%eax, %eax
	addq	$40, %rsp
	popq	%rbx
	popq	%r12
	popq	%r13
	popq	%r14
	popq	%r15
	popq	%rbp
	retq
	.cfi_endproc

	.globl	_profilers              ## @profilers
.zerofill __DATA,__common,_profilers,24576,4
	.section	__TEXT,__const
	.align	4                       ## @unit_test.b
l_unit_test.b:
	.long	0                       ## float 0.000000e+00
	.long	1065353216              ## float 1.000000e+00
	.long	1073741824              ## float 2.000000e+00
	.long	1077936128              ## float 3.000000e+00
	.long	1082130432              ## float 4.000000e+00
	.long	1084227584              ## float 5.000000e+00
	.long	1086324736              ## float 6.000000e+00
	.long	1088421888              ## float 7.000000e+00
	.long	1091567616              ## float 9.000000e+00
	.long	1092616192              ## float 1.000000e+01
	.long	1093664768              ## float 1.100000e+01
	.long	1094713344              ## float 1.200000e+01
	.long	1095761920              ## float 1.300000e+01
	.long	1096810496              ## float 1.400000e+01
	.long	1097859072              ## float 1.500000e+01
	.long	1098907648              ## float 1.600000e+01

	.section	__TEXT,__cstring,cstring_literals
L_.str:                                 ## @.str
	.asciz	"%.1f "

L_.str.2:                               ## @.str.2
	.asciz	"[%-20s]\ttime: %8f\n"

L_.str.3:                               ## @.str.3
	.asciz	"random_generation"

L_.str.4:                               ## @.str.4
	.asciz	"mm_vector_128"

L_.str.5:                               ## @.str.5
	.asciz	"mm_vector_256"

L_.str.6:                               ## @.str.6
	.asciz	"mm_scalar_unrolled"

L_.str.7:                               ## @.str.7
	.asciz	"mm_scalar_loop"


.subsections_via_symbols
