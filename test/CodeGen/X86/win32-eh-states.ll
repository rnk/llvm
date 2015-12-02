; RUN: llc -mtriple=i686-pc-windows-msvc   < %s | FileCheck %s --check-prefix=X86
; RUN: llc -mtriple=x86_64-pc-windows-msvc < %s | FileCheck %s --check-prefix=X64

; Based on this source:
; extern "C" void may_throw(int);
; void f() {
;   try {
;     may_throw(1);
;     try {
;       may_throw(2);
;     } catch (int) {
;       may_throw(3);
;     }
;   } catch (int) {
;     may_throw(4);
;   }
; }

%rtti.TypeDescriptor2 = type { i8**, i8*, [3 x i8] }
%eh.CatchHandlerType = type { i32, i8* }

declare void @may_throw(i32)
declare i32 @__CxxFrameHandler3(...)
declare void @llvm.eh.begincatch(i8*, i8*)
declare void @llvm.eh.endcatch()
declare i32 @llvm.eh.typeid.for(i8*)

$"\01??_R0H@8" = comdat any

@"\01??_7type_info@@6B@" = external constant i8*
@"\01??_R0H@8" = linkonce_odr global %rtti.TypeDescriptor2 { i8** @"\01??_7type_info@@6B@", i8* null, [3 x i8] c".H\00" }, comdat
@llvm.eh.handlertype.H.0 = private unnamed_addr constant %eh.CatchHandlerType { i32 0, i8* bitcast (%rtti.TypeDescriptor2* @"\01??_R0H@8" to i8*) }, section "llvm.metadata"

define void @f() #0 personality i8* bitcast (i32 (...)* @__CxxFrameHandler3 to i8*) {
entry:
  invoke void @may_throw(i32 1)
          to label %invoke.cont unwind label %lpad.1

invoke.cont:                                      ; preds = %entry
  invoke void @may_throw(i32 2)
          to label %try.cont.9 unwind label %lpad

try.cont.9:                                       ; preds = %invoke.cont.3, %invoke.cont, %catch.7
  ret void

lpad:                                             ; preds = %catch, %entry
  %cs1 = catchswitch none, unwind label %lpad.1 [label %catch]

catch:                                            ; preds = %lpad.1
  %p1 = catchpad %cs1 [%rtti.TypeDescriptor2* @"\01??_R0H@8", i32 0, i8* null]
  invoke void @may_throw(i32 3)
          to label %invoke.cont.3 unwind label %lpad.1

invoke.cont.3:                                    ; preds = %catch
  catchret %p1 to label %try.cont.9

lpad.1:                                           ; preds = %invoke.cont
  %cs2 = catchswitch none, unwind to caller [label %catch.7]

catch.7:
  %p2 = catchpad %cs2 [%rtti.TypeDescriptor2* @"\01??_R0H@8", i32 0, i8* null]
  call void @may_throw(i32 4)
  catchret %p2 to label %try.cont.9
}

; X86-LABEL: _f:
; X86: movl $-1, [[state:[-0-9]+]](%ebp)
; X86: movl $___ehhandler$f, {{.*}}
;
; X86: movl $0, [[state]](%ebp)
; X86: movl $1, (%esp)
; X86: calll _may_throw
;
; X86: movl $1, [[state]](%ebp)
; X86: movl $2, (%esp)
; X86: calll _may_throw
;
; X86: movl $2, [[state]](%ebp)
; X86: movl $3, (%esp)
; X86: calll _may_throw
;
; X86: movl $3, [[state]](%ebp)
; X86: movl $4, (%esp)
; X86: calll _may_throw

; X86: .safeseh ___ehhandler$f


; X64-LABEL: f:
; X64-LABEL: $ip2state$f:
; X64-NEXT:   .long .Lfunc_begin0@IMGREL
; X64-NEXT:   .long -1
; X64-NEXT:   .long .Ltmp{{.*}}@IMGREL+1
; X64-NEXT:   .long 0
; X64-NEXT:   .long .Ltmp{{.*}}@IMGREL+1
; X64-NEXT:   .long 1
; X64-NEXT:   .long .Ltmp{{.*}}@IMGREL+1
; X64-NEXT:   .long -1
; X64-NEXT:   .long "?catch${{.*}}@?0?f@4HA"@IMGREL
; X64-NEXT:   .long 2
; X64-NEXT:   .long "?catch${{.*}}@?0?f@4HA"@IMGREL
; X64-NEXT:   .long 3
