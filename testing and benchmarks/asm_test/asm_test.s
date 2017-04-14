# hello_asm.s
# as hello_asm.s -o hello_asm.o
# ld hello_asm.o -e _main -o hello_asm
.section __DATA,__data
str:
  .asciz "Hello world!\n"

.section __TEXT,__text
.globl _main
_main:
  movl $0x2000004, %eax		# preparing system call 4
  movl $1, %edi				# STDOUT file descriptor is 1
  leaq str(%rip), %rsi		# The value to print
  movq $100, %rdx			# the size of the value to print
  syscall

  # callq _exit
  movl $0, %ebx
  movl $0x2000001, %eax		# exit 0
  syscall