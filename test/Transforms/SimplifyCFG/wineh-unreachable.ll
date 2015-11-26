; RUN: opt -S -simplifycfg < %s | FileCheck %s

declare void @Personality()
declare void @f()

; CHECK-LABEL: define void @test1()
define void @test1() personality i8* bitcast (void ()* @Personality to i8*) {
entry:
  ; CHECK: call void @f()
  invoke void @f()
    to label %exit unwind label %unreachable.unwind
exit:
  ret void
unreachable.unwind:
  cleanuppad none []
  unreachable  
}

; CHECK-LABEL: define void @test2()
define void @test2() personality i8* bitcast (void ()* @Personality to i8*) {
entry:
  invoke void @f()
    to label %exit unwind label %catch.pad
catch.pad:
  %cs1 = catchswitch none, unwind label %unreachable.unwind [label %catch.body]
  ; CHECK: catch.pad:
  ; CHECK-NEXT: catchswitch none, unwind label %unreachable.unwind [label %catch.body]
catch.body:
  ; CHECK:      catch.body:
  ; CHECK-NEXT:   catchpad %cs1
  ; CHECK-NEXT:   call void @f()
  ; CHECK-NEXT:   unreachable
  %catch = catchpad %cs1 []
  call void @f()
  catchret %catch to label %unreachable
exit:
  ret void
unreachable.unwind:
  cleanuppad none []
  unreachable
unreachable:
  unreachable
}

; CHECK-LABEL: define void @test3()
define void @test3() personality i8* bitcast (void ()* @Personality to i8*) {
entry:
  invoke void @f()
    to label %exit unwind label %cleanup.pad
cleanup.pad:
  ; CHECK: %cleanup = cleanuppad none []
  ; CHECK-NEXT: call void @f()
  ; CHECK-NEXT: unreachable
  %cleanup = cleanuppad none []
  invoke void @f()
    to label %cleanup.ret unwind label %unreachable.unwind
cleanup.ret:
  ; This cleanupret should be rewritten to unreachable,
  ; and merged into the pred block.
  cleanupret %cleanup unwind label %unreachable.unwind
exit:
  ret void
unreachable.unwind:
  cleanuppad none []
  unreachable
}

; CHECK-LABEL: define void @test4()
define void @test4() personality i8* bitcast (void ()* @Personality to i8*) {
entry:
  invoke void @f()
    to label %exit unwind label %terminate.pad
terminate.pad:
  ; CHECK: terminatepad none [] unwind to caller
  terminatepad none [] unwind label %unreachable.unwind
exit:
  ret void
unreachable.unwind:
  cleanuppad none []
  unreachable
}

; CHECK-LABEL: define void @test5()
define void @test5() personality i8* bitcast (void ()* @Personality to i8*) {
entry:
  invoke void @f()
          to label %exit unwind label %catch.pad

catch.pad:
  %cs1 = catchswitch none, unwind to caller [label %catch.body]

catch.body:
  %catch = catchpad %cs1 []
  catchret %catch to label %exit

exit:
  unreachable
}
