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

	.section	__TEXT,__literal8,8byte_literals
	.align	3
LCPI2_0:
	.quad	-4616189618054758400    ## double -1
	.section	__TEXT,__literal16,16byte_literals
	.align	4
LCPI2_1:
	.quad	-9223372036854775808    ## 0x8000000000000000
	.quad	-9223372036854775808    ## 0x8000000000000000
	.section	__TEXT,__text,regular,pure_instructions
	.globl	_perlin
	.align	4, 0x90
_perlin:                                ## @perlin
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
	vcvttsd2si	%xmm0, %eax
	vcvttsd2si	%xmm1, %ecx
	vcvtsi2sdl	%eax, %xmm0, %xmm4
	vsubsd	%xmm4, %xmm0, %xmm2
	vcvtsi2sdl	%ecx, %xmm0, %xmm5
	vsubsd	%xmm5, %xmm1, %xmm3
	vsubsd	%xmm0, %xmm4, %xmm0
	vsubsd	%xmm1, %xmm5, %xmm12
	vmaxsd	%xmm3, %xmm2, %xmm4
	vcmpltsd	%xmm3, %xmm2, %xmm5
	vandpd	%xmm0, %xmm5, %xmm6
	vandnpd	%xmm12, %xmm5, %xmm5
	vorpd	%xmm6, %xmm5, %xmm5
	vmaxsd	%xmm5, %xmm4, %xmm10
	vmovsd	LCPI2_0(%rip), %xmm8    ## xmm8 = mem[0],zero
	vaddsd	%xmm8, %xmm3, %xmm6
	vmovsd	LCPI2_1(%rip), %xmm9    ## xmm9 = mem[0],zero
	vxorpd	%xmm9, %xmm6, %xmm5
	vmaxsd	%xmm6, %xmm2, %xmm7
	vcmpltsd	%xmm6, %xmm2, %xmm4
	vandpd	%xmm0, %xmm4, %xmm0
	vandnpd	%xmm5, %xmm4, %xmm4
	vorpd	%xmm0, %xmm4, %xmm0
	vmaxsd	%xmm0, %xmm7, %xmm11
	vaddsd	%xmm8, %xmm2, %xmm4
	vxorpd	%xmm9, %xmm4, %xmm7
	vmaxsd	%xmm3, %xmm4, %xmm8
	vcmpltsd	%xmm3, %xmm4, %xmm1
	vandpd	%xmm7, %xmm1, %xmm0
	vandnpd	%xmm12, %xmm1, %xmm1
	vorpd	%xmm0, %xmm1, %xmm0
	vmaxsd	%xmm0, %xmm8, %xmm0
	vmaxsd	%xmm6, %xmm4, %xmm1
	vcmpltsd	%xmm6, %xmm4, %xmm4
	vandpd	%xmm7, %xmm4, %xmm6
	vandnpd	%xmm5, %xmm4, %xmm4
	vorpd	%xmm6, %xmm4, %xmm4
	vmaxsd	%xmm4, %xmm1, %xmm1
	vsubsd	%xmm10, %xmm11, %xmm4
	vmulsd	%xmm3, %xmm4, %xmm4
	vaddsd	%xmm10, %xmm4, %xmm4
	vsubsd	%xmm0, %xmm1, %xmm1
	vmulsd	%xmm3, %xmm1, %xmm1
	vaddsd	%xmm0, %xmm1, %xmm0
	vsubsd	%xmm4, %xmm0, %xmm0
	vmulsd	%xmm2, %xmm0, %xmm0
	vaddsd	%xmm4, %xmm0, %xmm0
	popq	%rbp
	retq
	.cfi_endproc

	.section	__TEXT,__literal8,8byte_literals
	.align	3
LCPI3_0:
	.quad	4603225197417343712     ## double 0.56066017074166652
LCPI3_1:
	.quad	4603097816325333214     ## double 0.54651802861647147
LCPI3_2:
	.quad	4569052338186839808     ## double 0.0029949635048581724
LCPI3_3:
	.quad	4569052427278063918     ## double 0.0029950021420176642
LCPI3_4:
	.quad	4569052397600853784     ## double 0.002994989271579384
LCPI3_5:
	.quad	4602678819172646912     ## double 0.5
LCPI3_6:
	.quad	4602683313707324896     ## double 0.50049899358844741
LCPI3_7:
	.quad	4603241769126068224     ## double 0.5625
LCPI3_8:
	.quad	4603219251117193952     ## double 0.55999999880790696
LCPI3_9:
	.quad	-4571364728013586432    ## double -1000
LCPI3_10:
	.quad	4546834183793252762     ## double 9.7656250000000005E-5
LCPI3_11:
	.quad	-4616189618054758400    ## double -1
LCPI3_13:
	.quad	4607182418800017408     ## double 1
LCPI3_14:
	.quad	4603153273472328512     ## double 0.55267500876389164
LCPI3_15:
	.quad	4604930621670686720     ## double 0.75000029802322388
LCPI3_16:
	.quad	4605076478936909452     ## double 0.76619370755015969
LCPI3_17:
	.quad	4603133896811413472     ## double 0.55052376725506846
LCPI3_18:
	.quad	4603239407112670755     ## double 0.56223776383416568
	.section	__TEXT,__literal16,16byte_literals
	.align	4
LCPI3_12:
	.quad	-9223372036854775808    ## 0x8000000000000000
	.quad	-9223372036854775808    ## 0x8000000000000000
	.section	__TEXT,__text,regular,pure_instructions
	.globl	_main
	.align	4, 0x90
_main:                                  ## @main
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
	pushq	%rbx
	subq	$40, %rsp
Ltmp12:
	.cfi_offset %rbx, -24
	leaq	L_.str(%rip), %rbx
	vmovsd	LCPI3_0(%rip), %xmm0    ## xmm0 = mem[0],zero
	movb	$1, %al
	movq	%rbx, %rdi
	callq	_printf
	vmovsd	LCPI3_1(%rip), %xmm0    ## xmm0 = mem[0],zero
	movb	$1, %al
	movq	%rbx, %rdi
	callq	_printf
	vmovsd	LCPI3_2(%rip), %xmm0    ## xmm0 = mem[0],zero
	movb	$1, %al
	movq	%rbx, %rdi
	callq	_printf
	vmovsd	LCPI3_3(%rip), %xmm0    ## xmm0 = mem[0],zero
	movb	$1, %al
	movq	%rbx, %rdi
	callq	_printf
	vmovsd	LCPI3_4(%rip), %xmm0    ## xmm0 = mem[0],zero
	movb	$1, %al
	movq	%rbx, %rdi
	callq	_printf
	vmovsd	LCPI3_5(%rip), %xmm0    ## xmm0 = mem[0],zero
	movb	$1, %al
	movq	%rbx, %rdi
	callq	_printf
	vmovsd	LCPI3_6(%rip), %xmm0    ## xmm0 = mem[0],zero
	movb	$1, %al
	movq	%rbx, %rdi
	callq	_printf
	vmovsd	LCPI3_7(%rip), %xmm0    ## xmm0 = mem[0],zero
	movb	$1, %al
	movq	%rbx, %rdi
	callq	_printf
	vmovsd	LCPI3_8(%rip), %xmm0    ## xmm0 = mem[0],zero
	movb	$1, %al
	movq	%rbx, %rdi
	callq	_printf
	xorl	%eax, %eax
	vxorpd	%xmm3, %xmm3, %xmm3
	vmovsd	LCPI3_9(%rip), %xmm2    ## xmm2 = mem[0],zero
                                        ## implicit-def: XMM1
                                        ## implicit-def: XMM0
	.align	4, 0x90
LBB3_1:                                 ## %.preheader
                                        ## =>This Loop Header: Depth=1
                                        ##     Child Loop BB3_2 Depth 2
	vmovsd	%xmm3, -40(%rbp)        ## 8-byte Spill
	vmulsd	LCPI3_10(%rip), %xmm3, %xmm4
	vmovsd	%xmm4, -16(%rbp)        ## 8-byte Spill
	vcvttsd2si	%xmm4, %ecx
	vcvtsi2sdl	%ecx, %xmm0, %xmm3
	vsubsd	%xmm3, %xmm4, %xmm11
	vsubsd	%xmm4, %xmm3, %xmm3
	vmovsd	%xmm3, -24(%rbp)        ## 8-byte Spill
	vaddsd	LCPI3_11(%rip), %xmm11, %xmm9
	vxorpd	LCPI3_12(%rip), %xmm9, %xmm3
	vmovsd	%xmm3, -32(%rbp)        ## 8-byte Spill
	movl	$10240, %ecx            ## imm = 0x2800
	vxorps	%xmm15, %xmm15, %xmm15
	vmovaps	%xmm1, %xmm10
	.align	4, 0x90
LBB3_2:                                 ##   Parent Loop BB3_1 Depth=1
                                        ## =>  This Inner Loop Header: Depth=2
	vmulsd	LCPI3_10(%rip), %xmm15, %xmm1
	vcvttsd2si	%xmm1, %edx
	vcvtsi2sdl	%edx, %xmm0, %xmm7
	vsubsd	%xmm7, %xmm1, %xmm6
	vsubsd	%xmm1, %xmm7, %xmm14
	vmaxsd	%xmm6, %xmm11, %xmm8
	vcmpltsd	%xmm6, %xmm11, %xmm4
	vandnpd	%xmm14, %xmm4, %xmm5
	vmovsd	-24(%rbp), %xmm3        ## 8-byte Reload
                                        ## xmm3 = mem[0],zero
	vandpd	%xmm3, %xmm4, %xmm4
	vorpd	%xmm4, %xmm5, %xmm4
	vmaxsd	%xmm4, %xmm8, %xmm8
	vaddsd	LCPI3_11(%rip), %xmm6, %xmm5
	vxorpd	LCPI3_12(%rip), %xmm5, %xmm4
	vcmpltsd	%xmm5, %xmm11, %xmm12
	vandnpd	%xmm4, %xmm12, %xmm13
	vandpd	%xmm3, %xmm12, %xmm7
	vorpd	%xmm7, %xmm13, %xmm7
	vmaxsd	%xmm5, %xmm11, %xmm3
	vmaxsd	%xmm7, %xmm3, %xmm12
	vcmpltsd	%xmm6, %xmm9, %xmm7
	vandnpd	%xmm14, %xmm7, %xmm13
	vmovsd	-32(%rbp), %xmm3        ## 8-byte Reload
                                        ## xmm3 = mem[0],zero
	vandpd	%xmm3, %xmm7, %xmm7
	vorpd	%xmm7, %xmm13, %xmm13
	vmaxsd	%xmm6, %xmm9, %xmm7
	vmaxsd	%xmm13, %xmm7, %xmm13
	vcmpltsd	%xmm5, %xmm9, %xmm7
	vandnpd	%xmm4, %xmm7, %xmm4
	vandpd	%xmm3, %xmm7, %xmm7
	vorpd	%xmm7, %xmm4, %xmm4
	vmaxsd	%xmm5, %xmm9, %xmm5
	vmaxsd	%xmm4, %xmm5, %xmm4
	vsubsd	%xmm8, %xmm12, %xmm5
	vmulsd	%xmm6, %xmm5, %xmm5
	vaddsd	%xmm8, %xmm5, %xmm5
	vsubsd	%xmm13, %xmm4, %xmm4
	vmulsd	%xmm6, %xmm4, %xmm4
	vaddsd	%xmm13, %xmm4, %xmm3
	vsubsd	%xmm5, %xmm3, %xmm3
	vmulsd	%xmm11, %xmm3, %xmm3
	vaddsd	%xmm5, %xmm3, %xmm7
	vucomisd	%xmm2, %xmm7
	vcmpltsd	%xmm7, %xmm2, %xmm3
	vandnpd	%xmm0, %xmm3, %xmm0
	vmovsd	-16(%rbp), %xmm4        ## 8-byte Reload
                                        ## xmm4 = mem[0],zero
	vandpd	%xmm4, %xmm3, %xmm8
	ja	LBB3_4
## BB#3:                                ##   in Loop: Header=BB3_2 Depth=2
	vmovaps	%xmm10, %xmm1
LBB3_4:                                 ##   in Loop: Header=BB3_2 Depth=2
	vorpd	%xmm8, %xmm0, %xmm0
	vmaxsd	%xmm2, %xmm7, %xmm2
	vaddsd	LCPI3_13(%rip), %xmm15, %xmm15
	vmovaps	%xmm1, %xmm10
	decl	%ecx
	jne	LBB3_2
## BB#5:                                ##   in Loop: Header=BB3_1 Depth=1
	vmovsd	-40(%rbp), %xmm3        ## 8-byte Reload
                                        ## xmm3 = mem[0],zero
	vaddsd	LCPI3_13(%rip), %xmm3, %xmm3
	incl	%eax
	cmpl	$10240, %eax            ## imm = 0x2800
	jne	LBB3_1
## BB#6:
	leaq	L_.str1(%rip), %rdi
	movb	$3, %al
	callq	_printf
	leaq	L_.str(%rip), %rbx
	vmovsd	LCPI3_14(%rip), %xmm0   ## xmm0 = mem[0],zero
	movb	$1, %al
	movq	%rbx, %rdi
	callq	_printf
	vmovsd	LCPI3_15(%rip), %xmm0   ## xmm0 = mem[0],zero
	movb	$1, %al
	movq	%rbx, %rdi
	callq	_printf
	vmovsd	LCPI3_16(%rip), %xmm0   ## xmm0 = mem[0],zero
	movb	$1, %al
	movq	%rbx, %rdi
	callq	_printf
	vmovsd	LCPI3_17(%rip), %xmm0   ## xmm0 = mem[0],zero
	movb	$1, %al
	movq	%rbx, %rdi
	callq	_printf
	vmovsd	LCPI3_18(%rip), %xmm0   ## xmm0 = mem[0],zero
	movb	$1, %al
	movq	%rbx, %rdi
	callq	_printf
	vmovsd	LCPI3_7(%rip), %xmm0    ## xmm0 = mem[0],zero
	movb	$1, %al
	movq	%rbx, %rdi
	callq	_printf
	xorl	%eax, %eax
	addq	$40, %rsp
	popq	%rbx
	popq	%rbp
	retq
	.cfi_endproc

	.globl	_profilers              ## @profilers
.zerofill __DATA,__common,_profilers,24576,4
	.section	__TEXT,__cstring,cstring_literals
L_.str:                                 ## @.str
	.asciz	"%.10f\n"

L_.str1:                                ## @.str1
	.asciz	"[%.10f,%.10f] %.10f\n"


.subsections_via_symbols
