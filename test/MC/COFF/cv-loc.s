# RUN: llvm-mc < %s | FileCheck %s

.cv_file 1 "a.c"
.cv_file 2 "t.inc"

# Implements this C:
# void f(volatile int *x) {
#   ++*x;
# #include "t.h" // contains two ++*x; statements
#   ++*x;
# }

.text
.def	 f;
	.scl	2;
	.type	32;
	.endef
	.text
	.globl	f
	.align	16, 0x90
f:
.Lfunc_begin0:
  .cv_loc 0 1 5 2
  incl (%rdi)
  # #include "t.h" start
  .cv_loc 0 2 0 0
  incl (%rdi)
  .cv_loc 0 2 1 0
  incl (%rdi)
  # #include "t.h" end
  .cv_loc 0 1 6 2
  incl (%rdi)
  retq
.Lfunc_end0:

.section .debug$S
.long 4
.long 0xF2 # Lines substream
.long .Llinetable_end0 - .Llinetable_begin0
.Llinetable_begin0:
.cv_linetable 0, f, .Lfunc_end0
.Llinetable_end0:

# CHECK: asdf
