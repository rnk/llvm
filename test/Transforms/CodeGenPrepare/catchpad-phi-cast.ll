; RUN: opt -codegenprepare -S < %s | FileCheck %s

; The following target lines are needed for the test to exercise what it should.
; Without these lines, CodeGenPrepare does not try to sink the bitcasts.
target datalayout = "e-m:w-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-pc-windows-msvc"

declare i32 @__CxxFrameHandler3(...)

declare void @f()

declare void @g(i8*)

; CodeGenPrepare will want to sink these bitcasts, but it selects the catchpad
; blocks as the place to which the bitcast should be sunk.  Since catchpads
; do not allow non-phi instructions before the terminator, this isn't possible. 

; CHECK-LABEL: @test(
define void @test(i32* %addr) personality i32 (...)* @__CxxFrameHandler3 {
entry:
  %x = getelementptr i32, i32* %addr, i32 1
  %p1 = bitcast i32* %x to i8*
  invoke void @f()
          to label %invoke.cont unwind label %catch1

; CHECK: invoke.cont:
; CHECK-NEXT: %y = getelementptr i32, i32* %addr, i32 2
invoke.cont:
  %y = getelementptr i32, i32* %addr, i32 2
  %p2 = bitcast i32* %y to i8*
  invoke void @f()
          to label %done unwind label %catch2

done:
  ret void

catch1:
  %cs1 = catchswitch none, unwind to caller [label %handler1]

handler1:
  %cp1 = catchpad %cs1 []
  br label %catch.shared
; CHECK: handler1:
; CHECK-NEXT: catchpad %cs1
; CHECK: %[[p1:[0-9]+]] = bitcast i32* %x to i8*

catch2:
  %cs2 = catchswitch none, unwind to caller [label %handler2]

handler2:
  %cp2 = catchpad %cs2 []
  br label %catch.shared
; CHECK: handler2:
; CHECK: catchpad %cs2
; CHECK: %[[p2:[0-9]+]] = bitcast i32* %y to i8*

; CHECK: catch.shared:
; CHECK-NEXT: %p = phi i8* [ %[[p1]], %handler1 ], [ %[[p2]], %handler2 ]
catch.shared:
  %p = phi i8* [ %p1, %handler1 ], [ %p2, %handler2 ]
  call void @g(i8* %p)
  unreachable
}
