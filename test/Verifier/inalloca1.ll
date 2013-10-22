; RUN: not llvm-as %s -o /dev/null 2>&1 | FileCheck %s

declare void @b(i64* byval inalloca %p)
; CHECK: Attributes {{.*}} are incompatible

declare void @c(i64* inreg inalloca %p)
; CHECK: Attributes {{.*}} are incompatible

declare void @d(void ()* inalloca %p)
; CHECK: do not support unsized types
