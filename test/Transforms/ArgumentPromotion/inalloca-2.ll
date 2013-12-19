; RUN: opt %s -argpromotion -S | FileCheck %s
; Argpromote can't promote %a because of the icmp use.

%struct.ss = type { i32, i64 }

define internal i1 @f(%struct.ss* inalloca  %a, %struct.ss* %b) nounwind  {
; CHECK: define internal i1 @f(%struct.ss* inalloca  %a, %struct.ss* %b)
  entry:
  %c = icmp eq %struct.ss* %a, %b
  ret i1 %c
}

define i32 @test() {
  %S = alloca %struct.ss
  %c = call i1 @f(%struct.ss* inalloca %S, %struct.ss* %S)
; CHECK: call i1 @f(%struct.ss* inalloca %S, %struct.ss* %S)
  ret i32 0
}
