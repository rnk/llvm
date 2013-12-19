; RUN: opt %s -argpromotion -scalarrepl -S | FileCheck %s
; CHECK-NOT: load

target datalayout = "E-p:64:64:64-a0:0:8-f32:32:32-f64:64:64-i1:8:8-i8:8:8-i16:16:16-i32:32:32-i64:32:64-v64:64:64-v128:128:128"
; Argpromote + scalarrepl should change this to passing the two integers by value.

%struct.ss = type { i32, i32 }

define internal i32 @f(%struct.ss* inalloca  %s) {
entry:
  %f0 = getelementptr %struct.ss* %s, i32 0, i32 0
  %f1 = getelementptr %struct.ss* %s, i32 0, i32 1
  %a = load i32* %f0, align 4
  %b = load i32* %f1, align 4
  %r = add i32 %a, %b
  ret i32 %r
}

define i32 @main() {
entry:
  %S = alloca %struct.ss
  %f0 = getelementptr %struct.ss* %S, i32 0, i32 0
  %f1 = getelementptr %struct.ss* %S, i32 0, i32 1
  store i32 1, i32* %f0, align 4
  store i32 2, i32* %f1, align 4
  %r = call i32 @f(%struct.ss* inalloca %S)
  ret i32 %r
}
