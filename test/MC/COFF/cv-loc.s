# RUN: llvm-mc < %s -filetype=obj | llvm-readobj - -codeview | FileCheck %s

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
.long 0xF4 # Filechecksums substream
.long .Lfilechecksums_end - .Lfilechecksums_begin
.Lfilechecksums_begin:
.long 1
.long 0
.long 5
.long 0
.Lfilechecksums_end:
.long 0xF3 # String table substream
.long .Lstrtab_end - .Lstrtab_begin
.Lstrtab_begin:
.byte 0
.asciz "a.c"
.asciz "t.h"
.align 4
.Lstrtab_end:
.long 0xF2 # Lines substream
.long .Llinetable_end0 - .Llinetable_begin0
.Llinetable_begin0:
.cv_linetable 0, f, .Lfunc_end0
.Llinetable_end0:

# CHECK: FunctionLineTable [
# CHECK:   LinkageName: f
# CHECK:   Flags: 0x0
# CHECK:   CodeSize: 0x9
# CHECK:   FilenameSegment [
# CHECK:     Filename: a.c (0x0)
# CHECK:     +0x0 [
# CHECK:       LineNumberStart: 5
# CHECK:       LineNumberEndDelta: 0
# CHECK:       IsStatement: No
# CHECK:     ]
# CHECK:   ]
# CHECK:   FilenameSegment [
# CHECK:     Filename: t.h (0x8)
# CHECK:     +0x2 [
# CHECK:       LineNumberStart: 0
# CHECK:       LineNumberEndDelta: 0
# CHECK:       IsStatement: No
# CHECK:     ]
# CHECK:     +0x4 [
# CHECK:       LineNumberStart: 1
# CHECK:       LineNumberEndDelta: 0
# CHECK:       IsStatement: No
# CHECK:     ]
# CHECK:   ]
# CHECK:   FilenameSegment [
# CHECK:     Filename: a.c (0x0)
# CHECK:     +0x6 [
# CHECK:       LineNumberStart: 6
# CHECK:       LineNumberEndDelta: 0
# CHECK:       IsStatement: No
# CHECK:     ]
# CHECK:   ]
# CHECK: ]
