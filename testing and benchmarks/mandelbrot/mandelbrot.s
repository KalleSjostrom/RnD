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

	.section	__TEXT,__literal4,4byte_literals
	.align	2
LCPI2_0:
	.long	965249161               ## float 2.6041668E-4
LCPI2_1:
	.long	3212836864              ## float -1
	.section	__TEXT,__const
	.align	5
LCPI2_2:
	.long	991533466               ## float 2.343750e-03
	.long	992791757               ## float 2.636719e-03
	.long	994050048               ## float 2.929688e-03
	.long	995308340               ## float 3.222656e-03
	.long	996566631               ## float 3.515625e-03
	.long	997824922               ## float 3.808594e-03
	.long	998663783               ## float 4.101563e-03
	.long	999292928               ## float 4.394531e-03
LCPI2_3:
	.long	3221225472              ## float -2.000000e+00
	.long	3221225472              ## float -2.000000e+00
	.long	3221225472              ## float -2.000000e+00
	.long	3221225472              ## float -2.000000e+00
	.long	3221225472              ## float -2.000000e+00
	.long	3221225472              ## float -2.000000e+00
	.long	3221225472              ## float -2.000000e+00
	.long	3221225472              ## float -2.000000e+00
LCPI2_4:
	.long	1249125376              ## float 4.000000e+06
	.long	1249125376              ## float 4.000000e+06
	.long	1249125376              ## float 4.000000e+06
	.long	1249125376              ## float 4.000000e+06
	.long	1249125376              ## float 4.000000e+06
	.long	1249125376              ## float 4.000000e+06
	.long	1249125376              ## float 4.000000e+06
	.long	1249125376              ## float 4.000000e+06
LCPI2_5:
	.long	1114636288              ## float 6.000000e+01
	.long	1114636288              ## float 6.000000e+01
	.long	1114636288              ## float 6.000000e+01
	.long	1114636288              ## float 6.000000e+01
	.long	1114636288              ## float 6.000000e+01
	.long	1114636288              ## float 6.000000e+01
	.long	1114636288              ## float 6.000000e+01
	.long	1114636288              ## float 6.000000e+01
LCPI2_6:
	.long	1082654720              ## float 4.250000e+00
	.long	1082654720              ## float 4.250000e+00
	.long	1082654720              ## float 4.250000e+00
	.long	1082654720              ## float 4.250000e+00
	.long	1082654720              ## float 4.250000e+00
	.long	1082654720              ## float 4.250000e+00
	.long	1082654720              ## float 4.250000e+00
	.long	1082654720              ## float 4.250000e+00
LCPI2_7:
	.long	991533466               ## float 2.343750e-03
	.long	991533466               ## float 2.343750e-03
	.long	991533466               ## float 2.343750e-03
	.long	991533466               ## float 2.343750e-03
	.long	991533466               ## float 2.343750e-03
	.long	991533466               ## float 2.343750e-03
	.long	991533466               ## float 2.343750e-03
	.long	991533466               ## float 2.343750e-03
	.section	__TEXT,__literal8,8byte_literals
	.align	3
LCPI2_8:
	.quad	4517329193108106637     ## double 9.9999999999999995E-7
LCPI2_9:
	.quad	-4706042843746669171    ## double -9.9999999999999995E-7
	.section	__TEXT,__text,regular,pure_instructions
	.globl	_sequential_vector
	.align	4, 0x90
_sequential_vector:                     ## @sequential_vector
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
	pushq	%r15
	pushq	%r14
	pushq	%r12
	pushq	%rbx
	subq	$32, %rsp
Ltmp9:
	.cfi_offset %rbx, -48
Ltmp10:
	.cfi_offset %r12, -40
Ltmp11:
	.cfi_offset %r14, -32
Ltmp12:
	.cfi_offset %r15, -24
	movq	%rdi, %rbx
	## InlineAsm Start
	rdtsc
	## InlineAsm End
	movl	%edx, %r14d
	movl	%eax, %r15d
	xorl	%r12d, %r12d
	leaq	-48(%rbp), %rdi
	xorl	%esi, %esi
	callq	_gettimeofday
	movq	-48(%rbp), %r8
	movl	-40(%rbp), %r9d
	vmovaps	LCPI2_3(%rip), %ymm3
	vmovaps	LCPI2_4(%rip), %ymm4
	vxorps	%ymm5, %ymm5, %ymm5
	vmovaps	LCPI2_5(%rip), %ymm6
	vmovaps	LCPI2_6(%rip), %ymm7
	vmovaps	LCPI2_7(%rip), %ymm8
	.align	4, 0x90
LBB2_1:                                 ## =>This Loop Header: Depth=1
                                        ##     Child Loop BB2_2 Depth 2
                                        ##       Child Loop BB2_3 Depth 3
	vxorps	%xmm0, %xmm0, %xmm0
	vcvtsi2ssl	%r12d, %xmm0, %xmm0
	vmulss	LCPI2_0(%rip), %xmm0, %xmm0
	vaddss	LCPI2_1(%rip), %xmm0, %xmm0
	vpshufd	$0, %xmm0, %xmm0        ## xmm0 = xmm0[0,0,0,0]
	vinsertf128	$1, %xmm0, %ymm0, %ymm9
	movq	%r12, %rax
	shlq	$11, %rax
	leaq	(%rax,%rax,4), %rdx
	xorl	%esi, %esi
	vmovaps	LCPI2_2(%rip), %ymm10
	.align	4, 0x90
LBB2_2:                                 ##   Parent Loop BB2_1 Depth=1
                                        ## =>  This Loop Header: Depth=2
                                        ##       Child Loop BB2_3 Depth 3
	vaddps	%ymm3, %ymm10, %ymm11
	xorl	%edi, %edi
	vxorps	%ymm13, %ymm13, %ymm13
	movl	$59, %eax
	vxorps	%ymm14, %ymm14, %ymm14
	vxorps	%ymm12, %ymm12, %ymm12
	.align	4, 0x90
LBB2_3:                                 ##   Parent Loop BB2_1 Depth=1
                                        ##     Parent Loop BB2_2 Depth=2
                                        ## =>    This Inner Loop Header: Depth=3
	vmulps	%ymm13, %ymm13, %ymm15
	vmulps	%ymm14, %ymm14, %ymm0
	vaddps	%ymm15, %ymm0, %ymm1
	vcmpgt_oqps	%ymm4, %ymm1, %ymm1
	vmovd	%eax, %xmm2
	vpshufd	$0, %xmm2, %xmm2        ## xmm2 = xmm2[0,0,0,0]
	vinsertf128	$1, %xmm2, %ymm2, %ymm2
	vcvtdq2ps	%ymm2, %ymm2
	vandps	%ymm1, %ymm2, %ymm2
	vmaxps	%ymm12, %ymm2, %ymm12
	vmovmskps	%ymm1, %ecx
	cmpl	$255, %ecx
	je	LBB2_5
## BB#4:                                ##   in Loop: Header=BB2_3 Depth=3
	vaddps	%ymm14, %ymm14, %ymm1
	vmulps	%ymm13, %ymm1, %ymm1
	vaddps	%ymm1, %ymm9, %ymm14
	vsubps	%ymm0, %ymm15, %ymm0
	vaddps	%ymm0, %ymm11, %ymm13
	incl	%edi
	decl	%eax
	cmpl	$60, %edi
	jl	LBB2_3
LBB2_5:                                 ##   in Loop: Header=BB2_2 Depth=2
	vcmpgt_oqps	%ymm5, %ymm12, %ymm0
	vsubps	%ymm12, %ymm6, %ymm1
	vandps	%ymm0, %ymm1, %ymm0
	leaq	(%rsi,%rdx), %rax
	vmulps	%ymm7, %ymm0, %ymm0
	vextractf128	$1, %ymm0, 16(%rbx,%rax,4)
	vmovaps	%xmm0, (%rbx,%rax,4)
	addq	$8, %rsi
	vaddps	%ymm8, %ymm10, %ymm10
	cmpl	$10240, %esi            ## imm = 0x2800
	jl	LBB2_2
## BB#6:                                ##   in Loop: Header=BB2_1 Depth=1
	incq	%r12
	cmpq	$7680, %r12             ## imm = 0x1E00
	jne	LBB2_1
## BB#7:
	shlq	$32, %r14
	orq	%r15, %r14
	vxorps	%xmm0, %xmm0, %xmm0
	vcvtsi2sdq	%r8, %xmm0, %xmm0
	vmovsd	%xmm0, -56(%rbp)        ## 8-byte Spill
	vxorps	%xmm0, %xmm0, %xmm0
	vcvtsi2sdl	%r9d, %xmm0, %xmm0
	vmovsd	%xmm0, -64(%rbp)        ## 8-byte Spill
	## InlineAsm Start
	rdtsc
	## InlineAsm End
                                        ## kill: EDX<def> EDX<kill> RDX<def>
	shlq	$32, %rdx
	movl	%eax, %eax
	orq	%rdx, %rax
	movq	_profilers+24(%rip), %rcx
	subq	%r14, %rcx
	addq	%rax, %rcx
	movq	%rcx, _profilers+24(%rip)
	addl	$9830400, _profilers+32(%rip) ## imm = 0x960000
	leaq	-48(%rbp), %rdi
	xorl	%esi, %esi
	vzeroupper
	callq	_gettimeofday
	vcvtsi2sdq	-48(%rbp), %xmm0, %xmm0
	vcvtsi2sdl	-40(%rbp), %xmm0, %xmm1
	vmulsd	LCPI2_8(%rip), %xmm1, %xmm1
	vmovsd	-64(%rbp), %xmm2        ## 8-byte Reload
	vmulsd	LCPI2_9(%rip), %xmm2, %xmm2
	vsubsd	-56(%rbp), %xmm2, %xmm2 ## 8-byte Folded Reload
	vaddsd	%xmm0, %xmm2, %xmm0
	vaddsd	%xmm1, %xmm0, %xmm0
	vaddsd	_profilers+40(%rip), %xmm0, %xmm0
	vmovsd	%xmm0, _profilers+40(%rip)
	addq	$32, %rsp
	popq	%rbx
	popq	%r12
	popq	%r14
	popq	%r15
	popq	%rbp
	retq
	.cfi_endproc

	.section	__TEXT,__literal4,4byte_literals
	.align	2
LCPI3_0:
	.long	965249161               ## float 2.6041668E-4
LCPI3_1:
	.long	3212836864              ## float -1
LCPI3_2:
	.long	966367642               ## float 2.92968762E-4
LCPI3_3:
	.long	3221225472              ## float -2
LCPI3_4:
	.long	1249125376              ## float 4.0E+6
LCPI3_5:
	.long	1082654720              ## float 4.25
	.section	__TEXT,__literal8,8byte_literals
	.align	3
LCPI3_6:
	.quad	4517329193108106637     ## double 9.9999999999999995E-7
LCPI3_7:
	.quad	-4706042843746669171    ## double -9.9999999999999995E-7
	.section	__TEXT,__text,regular,pure_instructions
	.globl	_sequential_scalar
	.align	4, 0x90
_sequential_scalar:                     ## @sequential_scalar
	.cfi_startproc
## BB#0:
	pushq	%rbp
Ltmp13:
	.cfi_def_cfa_offset 16
Ltmp14:
	.cfi_offset %rbp, -16
	movq	%rsp, %rbp
Ltmp15:
	.cfi_def_cfa_register %rbp
	pushq	%r15
	pushq	%r14
	pushq	%r12
	pushq	%rbx
	subq	$32, %rsp
Ltmp16:
	.cfi_offset %rbx, -48
Ltmp17:
	.cfi_offset %r12, -40
Ltmp18:
	.cfi_offset %r14, -32
Ltmp19:
	.cfi_offset %r15, -24
	movq	%rdi, %rbx
	## InlineAsm Start
	rdtsc
	## InlineAsm End
	movl	%edx, %r14d
	movl	%eax, %r15d
	xorl	%r12d, %r12d
	leaq	-48(%rbp), %rdi
	xorl	%esi, %esi
	callq	_gettimeofday
	movq	-48(%rbp), %r8
	movl	-40(%rbp), %ecx
	vmovss	LCPI3_0(%rip), %xmm8
	vmovss	LCPI3_1(%rip), %xmm9
	vmovss	LCPI3_2(%rip), %xmm11
	vmovss	LCPI3_3(%rip), %xmm3
	vmovss	LCPI3_5(%rip), %xmm10
	.align	4, 0x90
LBB3_1:                                 ## =>This Loop Header: Depth=1
                                        ##     Child Loop BB3_2 Depth 2
                                        ##       Child Loop BB3_3 Depth 3
	movq	%r12, %rdx
	shlq	$11, %rdx
	leaq	(%rdx,%rdx,4), %rdx
	vxorps	%xmm0, %xmm0, %xmm0
	vcvtsi2ssl	%r12d, %xmm0, %xmm0
	vmulss	%xmm8, %xmm0, %xmm0
	vaddss	%xmm9, %xmm0, %xmm5
	xorl	%esi, %esi
	.align	4, 0x90
LBB3_2:                                 ##   Parent Loop BB3_1 Depth=1
                                        ## =>  This Loop Header: Depth=2
                                        ##       Child Loop BB3_3 Depth 3
	vxorps	%xmm0, %xmm0, %xmm0
	vcvtsi2ssl	%esi, %xmm0, %xmm0
	vmulss	%xmm11, %xmm0, %xmm0
	vaddss	%xmm3, %xmm0, %xmm6
	xorl	%edi, %edi
	vxorps	%xmm7, %xmm7, %xmm7
	vxorps	%xmm0, %xmm0, %xmm0
	.align	4, 0x90
LBB3_3:                                 ##   Parent Loop BB3_1 Depth=1
                                        ##     Parent Loop BB3_2 Depth=2
                                        ## =>    This Inner Loop Header: Depth=3
	vmulss	%xmm7, %xmm7, %xmm1
	vmulss	%xmm0, %xmm0, %xmm4
	vaddss	%xmm4, %xmm1, %xmm2
	vucomiss	LCPI3_4(%rip), %xmm2
	ja	LBB3_5
## BB#4:                                ##   in Loop: Header=BB3_3 Depth=3
	vaddss	%xmm0, %xmm0, %xmm0
	vmulss	%xmm7, %xmm0, %xmm0
	vaddss	%xmm0, %xmm5, %xmm0
	vsubss	%xmm4, %xmm6, %xmm2
	vaddss	%xmm1, %xmm2, %xmm7
	incl	%edi
	cmpl	$60, %edi
	jl	LBB3_3
	jmp	LBB3_6
	.align	4, 0x90
LBB3_5:                                 ##   in Loop: Header=BB3_2 Depth=2
	leaq	(%rsi,%rdx), %rax
	decl	%edi
	vxorps	%xmm0, %xmm0, %xmm0
	vcvtsi2ssl	%edi, %xmm0, %xmm0
	vmulss	%xmm10, %xmm0, %xmm0
	vmovss	%xmm0, (%rbx,%rax,4)
LBB3_6:                                 ## %.critedge
                                        ##   in Loop: Header=BB3_2 Depth=2
	incq	%rsi
	cmpq	$10240, %rsi            ## imm = 0x2800
	jne	LBB3_2
## BB#7:                                ##   in Loop: Header=BB3_1 Depth=1
	incq	%r12
	cmpq	$7680, %r12             ## imm = 0x1E00
	jne	LBB3_1
## BB#8:
	shlq	$32, %r14
	orq	%r15, %r14
	vxorps	%xmm0, %xmm0, %xmm0
	vcvtsi2sdq	%r8, %xmm0, %xmm0
	vmovsd	%xmm0, -56(%rbp)        ## 8-byte Spill
	vxorps	%xmm0, %xmm0, %xmm0
	vcvtsi2sdl	%ecx, %xmm0, %xmm0
	vmovsd	%xmm0, -64(%rbp)        ## 8-byte Spill
	## InlineAsm Start
	rdtsc
	## InlineAsm End
                                        ## kill: EDX<def> EDX<kill> RDX<def>
	shlq	$32, %rdx
	movl	%eax, %eax
	orq	%rdx, %rax
	movq	_profilers+72(%rip), %rcx
	subq	%r14, %rcx
	addq	%rax, %rcx
	movq	%rcx, _profilers+72(%rip)
	addl	$78643200, _profilers+80(%rip) ## imm = 0x4B00000
	leaq	-48(%rbp), %rdi
	xorl	%esi, %esi
	callq	_gettimeofday
	vcvtsi2sdq	-48(%rbp), %xmm0, %xmm0
	vcvtsi2sdl	-40(%rbp), %xmm0, %xmm1
	vmulsd	LCPI3_6(%rip), %xmm1, %xmm1
	vmovsd	-64(%rbp), %xmm2        ## 8-byte Reload
	vmulsd	LCPI3_7(%rip), %xmm2, %xmm2
	vsubsd	-56(%rbp), %xmm2, %xmm2 ## 8-byte Folded Reload
	vaddsd	%xmm0, %xmm2, %xmm0
	vaddsd	%xmm1, %xmm0, %xmm0
	vaddsd	_profilers+88(%rip), %xmm0, %xmm0
	vmovsd	%xmm0, _profilers+88(%rip)
	addq	$32, %rsp
	popq	%rbx
	popq	%r12
	popq	%r14
	popq	%r15
	popq	%rbp
	retq
	.cfi_endproc

	.section	__TEXT,__literal4,4byte_literals
	.align	2
LCPI4_0:
	.long	965249161               ## float 2.6041668E-4
LCPI4_1:
	.long	3212836864              ## float -1
	.section	__TEXT,__const
	.align	5
LCPI4_2:
	.long	0                       ## float 0.000000e+00
	.long	1065353216              ## float 1.000000e+00
	.long	1073741824              ## float 2.000000e+00
	.long	1077936128              ## float 3.000000e+00
	.long	1082130432              ## float 4.000000e+00
	.long	1084227584              ## float 5.000000e+00
	.long	1086324736              ## float 6.000000e+00
	.long	1088421888              ## float 7.000000e+00
LCPI4_3:
	.long	966367642               ## float 2.929688e-04
	.long	966367642               ## float 2.929688e-04
	.long	966367642               ## float 2.929688e-04
	.long	966367642               ## float 2.929688e-04
	.long	966367642               ## float 2.929688e-04
	.long	966367642               ## float 2.929688e-04
	.long	966367642               ## float 2.929688e-04
	.long	966367642               ## float 2.929688e-04
LCPI4_4:
	.long	3221225472              ## float -2.000000e+00
	.long	3221225472              ## float -2.000000e+00
	.long	3221225472              ## float -2.000000e+00
	.long	3221225472              ## float -2.000000e+00
	.long	3221225472              ## float -2.000000e+00
	.long	3221225472              ## float -2.000000e+00
	.long	3221225472              ## float -2.000000e+00
	.long	3221225472              ## float -2.000000e+00
LCPI4_5:
	.long	1249125376              ## float 4.000000e+06
	.long	1249125376              ## float 4.000000e+06
	.long	1249125376              ## float 4.000000e+06
	.long	1249125376              ## float 4.000000e+06
	.long	1249125376              ## float 4.000000e+06
	.long	1249125376              ## float 4.000000e+06
	.long	1249125376              ## float 4.000000e+06
	.long	1249125376              ## float 4.000000e+06
LCPI4_6:
	.long	1114636288              ## float 6.000000e+01
	.long	1114636288              ## float 6.000000e+01
	.long	1114636288              ## float 6.000000e+01
	.long	1114636288              ## float 6.000000e+01
	.long	1114636288              ## float 6.000000e+01
	.long	1114636288              ## float 6.000000e+01
	.long	1114636288              ## float 6.000000e+01
	.long	1114636288              ## float 6.000000e+01
LCPI4_7:
	.long	1082654720              ## float 4.250000e+00
	.long	1082654720              ## float 4.250000e+00
	.long	1082654720              ## float 4.250000e+00
	.long	1082654720              ## float 4.250000e+00
	.long	1082654720              ## float 4.250000e+00
	.long	1082654720              ## float 4.250000e+00
	.long	1082654720              ## float 4.250000e+00
	.long	1082654720              ## float 4.250000e+00
	.section	__TEXT,__text,regular,pure_instructions
	.globl	_pthread_vector_task
	.align	4, 0x90
_pthread_vector_task:                   ## @pthread_vector_task
	.cfi_startproc
## BB#0:
	pushq	%rbp
Ltmp20:
	.cfi_def_cfa_offset 16
Ltmp21:
	.cfi_offset %rbp, -16
	movq	%rsp, %rbp
Ltmp22:
	.cfi_def_cfa_register %rbp
	movq	(%rdi), %rax
	imull	$120, 8(%rdi), %r8d
	movslq	%r8d, %r9
	vmovaps	LCPI4_2(%rip), %ymm2
	vmovaps	LCPI4_3(%rip), %ymm3
	vmovaps	LCPI4_4(%rip), %ymm4
	vmovaps	LCPI4_5(%rip), %ymm5
	vxorps	%ymm6, %ymm6, %ymm6
	vmovaps	LCPI4_6(%rip), %ymm7
	vmovaps	LCPI4_7(%rip), %ymm8
	.align	4, 0x90
LBB4_1:                                 ## =>This Loop Header: Depth=1
                                        ##     Child Loop BB4_2 Depth 2
                                        ##       Child Loop BB4_3 Depth 3
	vxorps	%xmm0, %xmm0, %xmm0
	vcvtsi2ssl	%r9d, %xmm0, %xmm0
	vmulss	LCPI4_0(%rip), %xmm0, %xmm0
	vaddss	LCPI4_1(%rip), %xmm0, %xmm0
	vpshufd	$0, %xmm0, %xmm0        ## xmm0 = xmm0[0,0,0,0]
	vinsertf128	$1, %xmm0, %ymm0, %ymm9
	movq	%r9, %rcx
	shlq	$43, %rcx
	leaq	(%rcx,%rcx,4), %r10
	sarq	$32, %r10
	xorl	%edi, %edi
	.align	4, 0x90
LBB4_2:                                 ##   Parent Loop BB4_1 Depth=1
                                        ## =>  This Loop Header: Depth=2
                                        ##       Child Loop BB4_3 Depth 3
	vmovd	%edi, %xmm0
	vpshufd	$0, %xmm0, %xmm0        ## xmm0 = xmm0[0,0,0,0]
	vinsertf128	$1, %xmm0, %ymm0, %ymm0
	vcvtdq2ps	%ymm0, %ymm0
	vaddps	%ymm2, %ymm0, %ymm0
	vmulps	%ymm3, %ymm0, %ymm0
	vaddps	%ymm4, %ymm0, %ymm10
	xorl	%ecx, %ecx
	vxorps	%ymm12, %ymm12, %ymm12
	movl	$59, %edx
	vxorps	%ymm13, %ymm13, %ymm13
	vxorps	%ymm11, %ymm11, %ymm11
	.align	4, 0x90
LBB4_3:                                 ##   Parent Loop BB4_1 Depth=1
                                        ##     Parent Loop BB4_2 Depth=2
                                        ## =>    This Inner Loop Header: Depth=3
	vmulps	%ymm12, %ymm12, %ymm14
	vmulps	%ymm13, %ymm13, %ymm15
	vaddps	%ymm14, %ymm15, %ymm0
	vcmpgt_oqps	%ymm5, %ymm0, %ymm0
	vmovd	%edx, %xmm1
	vpshufd	$0, %xmm1, %xmm1        ## xmm1 = xmm1[0,0,0,0]
	vinsertf128	$1, %xmm1, %ymm1, %ymm1
	vcvtdq2ps	%ymm1, %ymm1
	vandps	%ymm0, %ymm1, %ymm1
	vmaxps	%ymm11, %ymm1, %ymm11
	vmovmskps	%ymm0, %esi
	cmpl	$255, %esi
	je	LBB4_5
## BB#4:                                ##   in Loop: Header=BB4_3 Depth=3
	vaddps	%ymm13, %ymm13, %ymm0
	vmulps	%ymm12, %ymm0, %ymm0
	vaddps	%ymm0, %ymm9, %ymm13
	vsubps	%ymm15, %ymm14, %ymm0
	vaddps	%ymm0, %ymm10, %ymm12
	incl	%ecx
	decl	%edx
	cmpl	$60, %ecx
	jl	LBB4_3
LBB4_5:                                 ##   in Loop: Header=BB4_2 Depth=2
	vcmpgt_oqps	%ymm6, %ymm11, %ymm0
	vsubps	%ymm11, %ymm7, %ymm1
	vandps	%ymm0, %ymm1, %ymm0
	leaq	(%rdi,%r10), %rcx
	vmulps	%ymm8, %ymm0, %ymm0
	vextractf128	$1, %ymm0, 16(%rax,%rcx,4)
	vmovaps	%xmm0, (%rax,%rcx,4)
	addq	$8, %rdi
	cmpl	$10240, %edi            ## imm = 0x2800
	jl	LBB4_2
## BB#6:                                ##   in Loop: Header=BB4_1 Depth=1
	movq	%r9, %rcx
	incq	%rcx
	leal	119(%r8), %edx
	cmpl	%edx, %r9d
	movq	%rcx, %r9
	jl	LBB4_1
## BB#7:
	xorl	%eax, %eax
	popq	%rbp
	vzeroupper
	retq
	.cfi_endproc

	.section	__TEXT,__literal4,4byte_literals
	.align	2
LCPI5_0:
	.long	965249161               ## float 2.6041668E-4
LCPI5_1:
	.long	3212836864              ## float -1
LCPI5_2:
	.long	966367642               ## float 2.92968762E-4
LCPI5_3:
	.long	3221225472              ## float -2
LCPI5_4:
	.long	1249125376              ## float 4.0E+6
LCPI5_5:
	.long	1082654720              ## float 4.25
	.section	__TEXT,__text,regular,pure_instructions
	.globl	_pthread_scalar_task
	.align	4, 0x90
_pthread_scalar_task:                   ## @pthread_scalar_task
	.cfi_startproc
## BB#0:
	pushq	%rbp
Ltmp23:
	.cfi_def_cfa_offset 16
Ltmp24:
	.cfi_offset %rbp, -16
	movq	%rsp, %rbp
Ltmp25:
	.cfi_def_cfa_register %rbp
	movq	(%rdi), %rax
	imull	$120, 8(%rdi), %r8d
	movslq	%r8d, %r9
	vmovss	LCPI5_0(%rip), %xmm8
	vmovss	LCPI5_1(%rip), %xmm9
	vmovss	LCPI5_2(%rip), %xmm10
	vmovss	LCPI5_3(%rip), %xmm11
	vmovss	LCPI5_5(%rip), %xmm4
	.align	4, 0x90
LBB5_1:                                 ## =>This Loop Header: Depth=1
                                        ##     Child Loop BB5_2 Depth 2
                                        ##       Child Loop BB5_3 Depth 3
	vxorps	%xmm0, %xmm0, %xmm0
	vcvtsi2ssl	%r9d, %xmm0, %xmm0
	vmulss	%xmm8, %xmm0, %xmm0
	vaddss	%xmm9, %xmm0, %xmm5
	movq	%r9, %rcx
	shlq	$43, %rcx
	leaq	(%rcx,%rcx,4), %rsi
	sarq	$32, %rsi
	xorl	%edi, %edi
	.align	4, 0x90
LBB5_2:                                 ##   Parent Loop BB5_1 Depth=1
                                        ## =>  This Loop Header: Depth=2
                                        ##       Child Loop BB5_3 Depth 3
	vxorps	%xmm0, %xmm0, %xmm0
	vcvtsi2ssl	%edi, %xmm0, %xmm0
	vmulss	%xmm10, %xmm0, %xmm0
	vaddss	%xmm11, %xmm0, %xmm6
	xorl	%ecx, %ecx
	vxorps	%xmm7, %xmm7, %xmm7
	vxorps	%xmm0, %xmm0, %xmm0
	.align	4, 0x90
LBB5_3:                                 ##   Parent Loop BB5_1 Depth=1
                                        ##     Parent Loop BB5_2 Depth=2
                                        ## =>    This Inner Loop Header: Depth=3
	vmulss	%xmm7, %xmm7, %xmm1
	vmulss	%xmm0, %xmm0, %xmm2
	vaddss	%xmm2, %xmm1, %xmm3
	vucomiss	LCPI5_4(%rip), %xmm3
	ja	LBB5_5
## BB#4:                                ##   in Loop: Header=BB5_3 Depth=3
	vaddss	%xmm0, %xmm0, %xmm0
	vmulss	%xmm7, %xmm0, %xmm0
	vaddss	%xmm0, %xmm5, %xmm0
	vsubss	%xmm2, %xmm6, %xmm2
	vaddss	%xmm1, %xmm2, %xmm7
	incl	%ecx
	cmpl	$60, %ecx
	jl	LBB5_3
LBB5_5:                                 ##   in Loop: Header=BB5_2 Depth=2
	leaq	(%rdi,%rsi), %rdx
	decl	%ecx
	vxorps	%xmm0, %xmm0, %xmm0
	vcvtsi2ssl	%ecx, %xmm0, %xmm0
	vmulss	%xmm4, %xmm0, %xmm0
	vmovss	%xmm0, (%rax,%rdx,4)
	incq	%rdi
	cmpq	$10240, %rdi            ## imm = 0x2800
	jne	LBB5_2
## BB#6:                                ##   in Loop: Header=BB5_1 Depth=1
	movq	%r9, %rcx
	incq	%rcx
	leal	119(%r8), %edx
	cmpl	%edx, %r9d
	movq	%rcx, %r9
	jl	LBB5_1
## BB#7:
	xorl	%eax, %eax
	popq	%rbp
	retq
	.cfi_endproc

	.globl	_run_pthread_task
	.align	4, 0x90
_run_pthread_task:                      ## @run_pthread_task
	.cfi_startproc
## BB#0:
	pushq	%rbp
Ltmp26:
	.cfi_def_cfa_offset 16
Ltmp27:
	.cfi_offset %rbp, -16
	movq	%rsp, %rbp
Ltmp28:
	.cfi_def_cfa_register %rbp
	pushq	%r15
	pushq	%r14
	pushq	%r13
	pushq	%r12
	pushq	%rbx
	subq	$1112, %rsp             ## imm = 0x458
Ltmp29:
	.cfi_offset %rbx, -56
Ltmp30:
	.cfi_offset %r12, -48
Ltmp31:
	.cfi_offset %r13, -40
Ltmp32:
	.cfi_offset %r14, -32
Ltmp33:
	.cfi_offset %r15, -24
	movq	%rsi, %r13
	movq	%rdi, -1144(%rbp)       ## 8-byte Spill
	movq	___stack_chk_guard@GOTPCREL(%rip), %rax
	movq	(%rax), %rax
	movq	%rax, -48(%rbp)
	movl	$64, %edi
	movl	$8, %esi
	callq	_calloc
	movq	%rax, %r12
	movq	%r12, -1152(%rbp)       ## 8-byte Spill
	leaq	-112(%rbp), %rbx
	movq	%rbx, %rdi
	callq	_pthread_attr_init
	movl	$1, %esi
	movq	%rbx, %rdi
	movq	%rbx, %r15
	callq	_pthread_attr_setdetachstate
	leaq	-1136(%rbp), %rbx
	movl	$1024, %esi             ## imm = 0x400
	movq	%rbx, %rdi
	callq	___bzero
	xorl	%r14d, %r14d
	.align	4, 0x90
LBB6_1:                                 ## =>This Inner Loop Header: Depth=1
	movq	%r13, (%rbx)
	movl	%r14d, 8(%rbx)
	movq	%r12, %rdi
	movq	%r15, %rsi
	movq	-1144(%rbp), %rdx       ## 8-byte Reload
	movq	%rbx, %rcx
	callq	_pthread_create
	testl	%eax, %eax
	je	LBB6_3
## BB#2:                                ##   in Loop: Header=BB6_1 Depth=1
	movq	___stderrp@GOTPCREL(%rip), %rax
	movq	(%rax), %rcx
	movl	$26, %esi
	movl	$1, %edx
	leaq	L_.str(%rip), %rdi
	callq	_fwrite
LBB6_3:                                 ##   in Loop: Header=BB6_1 Depth=1
	incq	%r14
	addq	$8, %r12
	addq	$16, %rbx
	cmpq	$63, %r14
	jne	LBB6_1
## BB#4:
	leaq	-128(%rbp), %rdi
	movq	%r13, -128(%rbp)
	movl	$63, -120(%rbp)
	callq	*-1144(%rbp)            ## 8-byte Folded Reload
	leaq	-112(%rbp), %rdi
	callq	_pthread_attr_destroy
	xorl	%ebx, %ebx
	leaq	L_.str1(%rip), %r14
	movq	-1152(%rbp), %r15       ## 8-byte Reload
	.align	4, 0x90
LBB6_5:                                 ## =>This Inner Loop Header: Depth=1
	movq	(%r15,%rbx,8), %rdi
	xorl	%esi, %esi
	callq	_pthread_join
	testl	%eax, %eax
	je	LBB6_7
## BB#6:                                ##   in Loop: Header=BB6_5 Depth=1
	movq	___stderrp@GOTPCREL(%rip), %rax
	movq	(%rax), %rcx
	movl	$24, %esi
	movl	$1, %edx
	movq	%r14, %rdi
	callq	_fwrite
LBB6_7:                                 ##   in Loop: Header=BB6_5 Depth=1
	incq	%rbx
	cmpq	$63, %rbx
	jne	LBB6_5
## BB#8:
	movq	%r15, %rdi
	callq	_free
	movq	___stack_chk_guard@GOTPCREL(%rip), %rax
	movq	(%rax), %rax
	cmpq	-48(%rbp), %rax
	jne	LBB6_10
## BB#9:
	addq	$1112, %rsp             ## imm = 0x458
	popq	%rbx
	popq	%r12
	popq	%r13
	popq	%r14
	popq	%r15
	popq	%rbp
	retq
LBB6_10:
	callq	___stack_chk_fail
	.cfi_endproc

	.globl	_write_ppm
	.align	4, 0x90
_write_ppm:                             ## @write_ppm
	.cfi_startproc
## BB#0:                                ## %overflow.checked
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
	pushq	%rbx
	pushq	%rax
Ltmp37:
	.cfi_offset %rbx, -40
Ltmp38:
	.cfi_offset %r14, -32
Ltmp39:
	.cfi_offset %r15, -24
	movq	%rdi, %r15
	leaq	L_.str2(%rip), %rax
	movq	%rsi, %rdi
	movq	%rax, %rsi
	callq	_fopen
	movq	%rax, %r14
	leaq	L_.str3(%rip), %rdi
	movl	$3, %esi
	movl	$1, %edx
	movq	%r14, %rcx
	callq	_fwrite
	leaq	L_.str4(%rip), %rsi
	xorl	%ebx, %ebx
	movl	$10240, %edx            ## imm = 0x2800
	movl	$7680, %ecx             ## imm = 0x1E00
	xorl	%eax, %eax
	movq	%r14, %rdi
	callq	_fprintf
	leaq	L_.str5(%rip), %rdi
	movl	$4, %esi
	movl	$1, %edx
	movq	%r14, %rcx
	callq	_fwrite
	movl	$235929600, %edi        ## imm = 0xE100000
	movl	$1, %esi
	callq	_calloc
	addq	$12, %r15
	.align	4, 0x90
LBB7_1:                                 ## %vector.body
                                        ## =>This Inner Loop Header: Depth=1
	vcvttss2si	-12(%r15), %ecx
	vcvttss2si	-8(%r15), %edx
	vcvttss2si	-4(%r15), %esi
	vcvttss2si	(%r15), %edi
	movb	%cl, (%rax,%rbx)
	movb	%dl, 3(%rax,%rbx)
	movb	%sil, 6(%rax,%rbx)
	movb	%dil, 9(%rax,%rbx)
	addq	$16, %r15
	addq	$12, %rbx
	cmpq	$235929600, %rbx        ## imm = 0xE100000
	jne	LBB7_1
## BB#2:                                ## %middle.block
	movl	$1, %esi
	movl	$235929600, %edx        ## imm = 0xE100000
	movq	%rax, %rdi
	movq	%r14, %rcx
	callq	_fwrite
	movq	%r14, %rdi
	addq	$8, %rsp
	popq	%rbx
	popq	%r14
	popq	%r15
	popq	%rbp
	jmp	_fclose                 ## TAILCALL
	.cfi_endproc

	.section	__TEXT,__literal8,8byte_literals
	.align	3
LCPI8_0:
	.quad	4517329193108106637     ## double 9.9999999999999995E-7
LCPI8_1:
	.quad	-4706042843746669171    ## double -9.9999999999999995E-7
	.section	__TEXT,__text,regular,pure_instructions
	.globl	_main
	.align	4, 0x90
_main:                                  ## @main
	.cfi_startproc
## BB#0:
	pushq	%rbp
Ltmp40:
	.cfi_def_cfa_offset 16
Ltmp41:
	.cfi_offset %rbp, -16
	movq	%rsp, %rbp
Ltmp42:
	.cfi_def_cfa_register %rbp
	pushq	%r14
	pushq	%rbx
	subq	$48, %rsp
Ltmp43:
	.cfi_offset %rbx, -32
Ltmp44:
	.cfi_offset %r14, -24
	leaq	-40(%rbp), %rdi
	movl	$32, %esi
	movl	$314572800, %edx        ## imm = 0x12C00000
	callq	_posix_memalign
	## InlineAsm Start
	rdtsc
	## InlineAsm End
                                        ## kill: EDX<def> EDX<kill> RDX<def>
	shlq	$32, %rdx
	movl	%eax, %ebx
	orq	%rdx, %rbx
	xorl	%r14d, %r14d
	leaq	-32(%rbp), %rdi
	xorl	%esi, %esi
	callq	_gettimeofday
	vcvtsi2sdq	-32(%rbp), %xmm0, %xmm0
	vmovsd	%xmm0, -48(%rbp)        ## 8-byte Spill
	vcvtsi2sdl	-24(%rbp), %xmm0, %xmm0
	vmovsd	%xmm0, -56(%rbp)        ## 8-byte Spill
	movq	-40(%rbp), %rdi
	callq	_sequential_vector
	## InlineAsm Start
	rdtsc
	## InlineAsm End
                                        ## kill: EDX<def> EDX<kill> RDX<def>
	shlq	$32, %rdx
	movl	%eax, %eax
	orq	%rdx, %rax
	movq	_profilers(%rip), %rcx
	subq	%rbx, %rcx
	addq	%rax, %rcx
	movq	%rcx, _profilers(%rip)
	incl	_profilers+8(%rip)
	leaq	-32(%rbp), %rdi
	xorl	%esi, %esi
	callq	_gettimeofday
	vcvtsi2sdq	-32(%rbp), %xmm0, %xmm0
	vcvtsi2sdl	-24(%rbp), %xmm0, %xmm1
	vmulsd	LCPI8_0(%rip), %xmm1, %xmm1
	vmovsd	-56(%rbp), %xmm2        ## 8-byte Reload
	vmulsd	LCPI8_1(%rip), %xmm2, %xmm2
	vsubsd	-48(%rbp), %xmm2, %xmm2 ## 8-byte Folded Reload
	vaddsd	%xmm0, %xmm2, %xmm0
	vaddsd	%xmm1, %xmm0, %xmm0
	vaddsd	_profilers+16(%rip), %xmm0, %xmm0
	vmovsd	%xmm0, _profilers+16(%rip)
	movq	_profilers(%rip), %rbx
	movl	_profilers+8(%rip), %ecx
	testq	%rcx, %rcx
	je	LBB8_2
## BB#1:
	xorl	%edx, %edx
	movq	%rbx, %rax
	divq	%rcx
	movq	%rax, %r14
LBB8_2:
	leaq	L_.str6(%rip), %rdi
	leaq	L_.str7(%rip), %rsi
	movb	$1, %al
	movq	%rbx, %rdx
                                        ## kill: ECX<def> ECX<kill> RCX<kill>
	movq	%r14, %r8
	callq	_printf
	movq	_profilers+24(%rip), %rbx
	movl	_profilers+32(%rip), %ecx
	xorl	%r8d, %r8d
	testq	%rcx, %rcx
	je	LBB8_4
## BB#3:
	xorl	%edx, %edx
	movq	%rbx, %rax
	divq	%rcx
	movq	%rax, %r8
LBB8_4:
	vmovsd	_profilers+40(%rip), %xmm0
	leaq	L_.str6(%rip), %rdi
	leaq	L_.str8(%rip), %rsi
	movb	$1, %al
	movq	%rbx, %rdx
                                        ## kill: ECX<def> ECX<kill> RCX<kill>
	callq	_printf
	movq	-40(%rbp), %rdi
	leaq	L_.str9(%rip), %rsi
	callq	_write_ppm
	## InlineAsm Start
	rdtsc
	## InlineAsm End
                                        ## kill: EDX<def> EDX<kill> RDX<def>
	shlq	$32, %rdx
	movl	%eax, %ebx
	orq	%rdx, %rbx
	xorl	%r14d, %r14d
	leaq	-32(%rbp), %rdi
	xorl	%esi, %esi
	callq	_gettimeofday
	vcvtsi2sdq	-32(%rbp), %xmm0, %xmm0
	vmovsd	%xmm0, -48(%rbp)        ## 8-byte Spill
	vcvtsi2sdl	-24(%rbp), %xmm0, %xmm0
	vmovsd	%xmm0, -56(%rbp)        ## 8-byte Spill
	movq	-40(%rbp), %rdi
	callq	_sequential_scalar
	## InlineAsm Start
	rdtsc
	## InlineAsm End
                                        ## kill: EDX<def> EDX<kill> RDX<def>
	shlq	$32, %rdx
	movl	%eax, %eax
	orq	%rdx, %rax
	movq	_profilers+48(%rip), %rcx
	subq	%rbx, %rcx
	addq	%rax, %rcx
	movq	%rcx, _profilers+48(%rip)
	incl	_profilers+56(%rip)
	leaq	-32(%rbp), %rdi
	xorl	%esi, %esi
	callq	_gettimeofday
	vcvtsi2sdq	-32(%rbp), %xmm0, %xmm0
	vcvtsi2sdl	-24(%rbp), %xmm0, %xmm1
	vmulsd	LCPI8_0(%rip), %xmm1, %xmm1
	vmovsd	-56(%rbp), %xmm2        ## 8-byte Reload
	vmulsd	LCPI8_1(%rip), %xmm2, %xmm2
	vsubsd	-48(%rbp), %xmm2, %xmm2 ## 8-byte Folded Reload
	vaddsd	%xmm0, %xmm2, %xmm0
	vaddsd	%xmm1, %xmm0, %xmm0
	vaddsd	_profilers+64(%rip), %xmm0, %xmm0
	vmovsd	%xmm0, _profilers+64(%rip)
	movq	_profilers+48(%rip), %rbx
	movl	_profilers+56(%rip), %ecx
	testq	%rcx, %rcx
	je	LBB8_6
## BB#5:
	xorl	%edx, %edx
	movq	%rbx, %rax
	divq	%rcx
	movq	%rax, %r14
LBB8_6:
	leaq	L_.str6(%rip), %rdi
	leaq	L_.str10(%rip), %rsi
	movb	$1, %al
	movq	%rbx, %rdx
                                        ## kill: ECX<def> ECX<kill> RCX<kill>
	movq	%r14, %r8
	callq	_printf
	movq	_profilers+72(%rip), %rbx
	movl	_profilers+80(%rip), %ecx
	xorl	%r8d, %r8d
	testq	%rcx, %rcx
	je	LBB8_8
## BB#7:
	xorl	%edx, %edx
	movq	%rbx, %rax
	divq	%rcx
	movq	%rax, %r8
LBB8_8:
	vmovsd	_profilers+88(%rip), %xmm0
	leaq	L_.str6(%rip), %rdi
	leaq	L_.str11(%rip), %rsi
	movb	$1, %al
	movq	%rbx, %rdx
                                        ## kill: ECX<def> ECX<kill> RCX<kill>
	callq	_printf
	movq	-40(%rbp), %rdi
	leaq	L_.str12(%rip), %rsi
	callq	_write_ppm
	## InlineAsm Start
	rdtsc
	## InlineAsm End
                                        ## kill: EDX<def> EDX<kill> RDX<def>
	shlq	$32, %rdx
	movl	%eax, %ebx
	orq	%rdx, %rbx
	xorl	%r14d, %r14d
	leaq	-32(%rbp), %rdi
	xorl	%esi, %esi
	callq	_gettimeofday
	vcvtsi2sdq	-32(%rbp), %xmm0, %xmm0
	vmovsd	%xmm0, -48(%rbp)        ## 8-byte Spill
	vcvtsi2sdl	-24(%rbp), %xmm0, %xmm0
	vmovsd	%xmm0, -56(%rbp)        ## 8-byte Spill
	movq	-40(%rbp), %rsi
	leaq	_pthread_scalar_task(%rip), %rdi
	callq	_run_pthread_task
	## InlineAsm Start
	rdtsc
	## InlineAsm End
                                        ## kill: EDX<def> EDX<kill> RDX<def>
	shlq	$32, %rdx
	movl	%eax, %eax
	orq	%rdx, %rax
	movq	_profilers+96(%rip), %rcx
	subq	%rbx, %rcx
	addq	%rax, %rcx
	movq	%rcx, _profilers+96(%rip)
	incl	_profilers+104(%rip)
	leaq	-32(%rbp), %rdi
	xorl	%esi, %esi
	callq	_gettimeofday
	vcvtsi2sdq	-32(%rbp), %xmm0, %xmm0
	vcvtsi2sdl	-24(%rbp), %xmm0, %xmm1
	vmulsd	LCPI8_0(%rip), %xmm1, %xmm1
	vmovsd	-56(%rbp), %xmm2        ## 8-byte Reload
	vmulsd	LCPI8_1(%rip), %xmm2, %xmm2
	vsubsd	-48(%rbp), %xmm2, %xmm2 ## 8-byte Folded Reload
	vaddsd	%xmm0, %xmm2, %xmm0
	vaddsd	%xmm1, %xmm0, %xmm0
	vaddsd	_profilers+112(%rip), %xmm0, %xmm0
	vmovsd	%xmm0, _profilers+112(%rip)
	movq	_profilers+96(%rip), %rbx
	movl	_profilers+104(%rip), %ecx
	testq	%rcx, %rcx
	je	LBB8_10
## BB#9:
	xorl	%edx, %edx
	movq	%rbx, %rax
	divq	%rcx
	movq	%rax, %r14
LBB8_10:
	leaq	L_.str6(%rip), %rdi
	leaq	L_.str13(%rip), %rsi
	movb	$1, %al
	movq	%rbx, %rdx
                                        ## kill: ECX<def> ECX<kill> RCX<kill>
	movq	%r14, %r8
	callq	_printf
	movq	-40(%rbp), %rdi
	leaq	L_.str14(%rip), %rsi
	callq	_write_ppm
	## InlineAsm Start
	rdtsc
	## InlineAsm End
                                        ## kill: EDX<def> EDX<kill> RDX<def>
	shlq	$32, %rdx
	movl	%eax, %ebx
	orq	%rdx, %rbx
	xorl	%r14d, %r14d
	leaq	-32(%rbp), %rdi
	xorl	%esi, %esi
	callq	_gettimeofday
	vcvtsi2sdq	-32(%rbp), %xmm0, %xmm0
	vmovsd	%xmm0, -48(%rbp)        ## 8-byte Spill
	vcvtsi2sdl	-24(%rbp), %xmm0, %xmm0
	vmovsd	%xmm0, -56(%rbp)        ## 8-byte Spill
	movq	-40(%rbp), %rsi
	leaq	_pthread_vector_task(%rip), %rdi
	callq	_run_pthread_task
	## InlineAsm Start
	rdtsc
	## InlineAsm End
                                        ## kill: EDX<def> EDX<kill> RDX<def>
	shlq	$32, %rdx
	movl	%eax, %eax
	orq	%rdx, %rax
	movq	_profilers+144(%rip), %rcx
	subq	%rbx, %rcx
	addq	%rax, %rcx
	movq	%rcx, _profilers+144(%rip)
	incl	_profilers+152(%rip)
	leaq	-32(%rbp), %rdi
	xorl	%esi, %esi
	callq	_gettimeofday
	vcvtsi2sdq	-32(%rbp), %xmm0, %xmm0
	vcvtsi2sdl	-24(%rbp), %xmm0, %xmm1
	vmulsd	LCPI8_0(%rip), %xmm1, %xmm1
	vmovsd	-56(%rbp), %xmm2        ## 8-byte Reload
	vmulsd	LCPI8_1(%rip), %xmm2, %xmm2
	vsubsd	-48(%rbp), %xmm2, %xmm2 ## 8-byte Folded Reload
	vaddsd	%xmm0, %xmm2, %xmm0
	vaddsd	%xmm1, %xmm0, %xmm0
	vaddsd	_profilers+160(%rip), %xmm0, %xmm0
	vmovsd	%xmm0, _profilers+160(%rip)
	movq	_profilers+144(%rip), %rbx
	movl	_profilers+152(%rip), %ecx
	testq	%rcx, %rcx
	je	LBB8_12
## BB#11:
	xorl	%edx, %edx
	movq	%rbx, %rax
	divq	%rcx
	movq	%rax, %r14
LBB8_12:
	leaq	L_.str6(%rip), %rdi
	leaq	L_.str15(%rip), %rsi
	movb	$1, %al
	movq	%rbx, %rdx
                                        ## kill: ECX<def> ECX<kill> RCX<kill>
	movq	%r14, %r8
	callq	_printf
	## InlineAsm Start
	rdtsc
	## InlineAsm End
                                        ## kill: EDX<def> EDX<kill> RDX<def>
	shlq	$32, %rdx
	movl	%eax, %ebx
	orq	%rdx, %rbx
	xorl	%r14d, %r14d
	leaq	-32(%rbp), %rdi
	xorl	%esi, %esi
	callq	_gettimeofday
	vcvtsi2sdq	-32(%rbp), %xmm0, %xmm0
	vmovsd	%xmm0, -48(%rbp)        ## 8-byte Spill
	vcvtsi2sdl	-24(%rbp), %xmm0, %xmm0
	vmovsd	%xmm0, -56(%rbp)        ## 8-byte Spill
	movq	-40(%rbp), %rdi
	leaq	L_.str16(%rip), %rsi
	callq	_write_ppm
	## InlineAsm Start
	rdtsc
	## InlineAsm End
                                        ## kill: EDX<def> EDX<kill> RDX<def>
	shlq	$32, %rdx
	movl	%eax, %eax
	orq	%rdx, %rax
	movq	_profilers+192(%rip), %rcx
	subq	%rbx, %rcx
	addq	%rax, %rcx
	movq	%rcx, _profilers+192(%rip)
	incl	_profilers+200(%rip)
	leaq	-32(%rbp), %rdi
	xorl	%esi, %esi
	callq	_gettimeofday
	vcvtsi2sdq	-32(%rbp), %xmm0, %xmm0
	vcvtsi2sdl	-24(%rbp), %xmm0, %xmm1
	vmulsd	LCPI8_0(%rip), %xmm1, %xmm1
	vmovsd	-56(%rbp), %xmm2        ## 8-byte Reload
	vmulsd	LCPI8_1(%rip), %xmm2, %xmm2
	vsubsd	-48(%rbp), %xmm2, %xmm2 ## 8-byte Folded Reload
	vaddsd	%xmm0, %xmm2, %xmm0
	vaddsd	%xmm1, %xmm0, %xmm0
	vaddsd	_profilers+208(%rip), %xmm0, %xmm0
	vmovsd	%xmm0, _profilers+208(%rip)
	movq	_profilers+192(%rip), %rbx
	movl	_profilers+200(%rip), %ecx
	testq	%rcx, %rcx
	je	LBB8_14
## BB#13:
	xorl	%edx, %edx
	movq	%rbx, %rax
	divq	%rcx
	movq	%rax, %r14
LBB8_14:
	leaq	L_.str6(%rip), %rdi
	leaq	L_.str17(%rip), %rsi
	movb	$1, %al
	movq	%rbx, %rdx
                                        ## kill: ECX<def> ECX<kill> RCX<kill>
	movq	%r14, %r8
	callq	_printf
	movq	-40(%rbp), %rdi
	callq	_free
	xorl	%eax, %eax
	addq	$48, %rsp
	popq	%rbx
	popq	%r14
	popq	%rbp
	retq
	.cfi_endproc

	.globl	_profilers              ## @profilers
.zerofill __DATA,__common,_profilers,24576,4
	.section	__TEXT,__cstring,cstring_literals
L_.str:                                 ## @.str
	.asciz	"Failed to create a thread\n"

L_.str1:                                ## @.str1
	.asciz	"Failed to join a thread\n"

L_.str2:                                ## @.str2
	.asciz	"w"

L_.str3:                                ## @.str3
	.asciz	"P6\n"

L_.str4:                                ## @.str4
	.asciz	"%i %i\n"

L_.str5:                                ## @.str5
	.asciz	"255\n"

L_.str6:                                ## @.str6
	.asciz	"[%-30s]\tcycles: %12llu,\t\thits: %10u,\t\tcycles/hit: %12llu,\t\ttime: %10f\n"

L_.str7:                                ## @.str7
	.asciz	"sequential_vector"

L_.str8:                                ## @.str8
	.asciz	"sequential_vector_inner"

L_.str9:                                ## @.str9
	.asciz	"mm_fract.ppm"

L_.str10:                               ## @.str10
	.asciz	"sequential_scalar"

L_.str11:                               ## @.str11
	.asciz	"sequential_scalar_inner"

L_.str12:                               ## @.str12
	.asciz	"fract.ppm"

L_.str13:                               ## @.str13
	.asciz	"pthread_scalar"

L_.str14:                               ## @.str14
	.asciz	"pthread_fract.ppm"

L_.str15:                               ## @.str15
	.asciz	"pthread_vector"

L_.str16:                               ## @.str16
	.asciz	"pthread_mm_fract.ppm"

L_.str17:                               ## @.str17
	.asciz	"write_ppm"


.subsections_via_symbols
