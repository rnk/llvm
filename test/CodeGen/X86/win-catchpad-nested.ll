; RUN: llc -mtriple=x86_64-pc-windows-coreclr < %s | FileCheck %s

declare void @ProcessCLRException()

declare void @f()

define void @test1() personality void ()* @ProcessCLRException {
entry:
  invoke void @f()
          to label %exit unwind label %catch.dispatch.1
exit:
  ret void

catch.dispatch.1:
  %cs1 = catchswitch none, unwind to caller [label %outer.catch]

outer.catch:
  %cp1 = catchpad %cs1 [i32 1]
  invoke void @f()
          to label %outer.ret unwind label %catch.dispatch.2
outer.ret:
  catchret %cp1 to label %exit

catch.dispatch.2:
  %cs2 = catchswitch %cp1, unwind to caller [label %inner.catch]
inner.catch:
  %cp2 = catchpad %cs2 [i32 2]
  catchret %cp2 to label %outer.ret
}

; Check the catchret targets
; CHECK-LABEL: test1: # @test1
; CHECK: [[Exit:^[^: ]+]]: # Block address taken
; CHECK-NEXT:              # %exit
; CHECK: [[OuterRet:^[^: ]+]]: # Block address taken
; CHECK-NEXT:                  # %outer.ret
; CHECK-NEXT: leaq [[Exit]](%rip), %rax
; CHECK:      retq   # CATCHRET
; CHECK: {{^[^: ]+}}: # %inner.pad
; CHECK: .seh_endprolog
; CHECK-NEXT: leaq [[OuterRet]](%rip), %rax
; CHECK:      retq   # CATCHRET
