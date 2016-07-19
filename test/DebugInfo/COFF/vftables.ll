; RUN: llc < %s -filetype=obj | llvm-readobj - -codeview | FileCheck %s

; C++ source to regenerate:
; struct A {
;   virtual void f();
;   virtual void g();
;   int a = 0;
; };
; struct B {
;   virtual void g();
;   virtual void f();
;   int b = 0;
; };
; struct C : A, B {
;   virtual void g();
;   virtual void f();
;   int c = 0;
; };
; struct D : C {
;   virtual void g();
;   virtual void f();
;   int d = 0;
; };
; void h() { D d; }

; CHECK:  VFTableShape (0x{{[A-Z0-9]+}}) {
; CHECK:    TypeLeafKind: LF_VTSHAPE (0xA)
; CHECK:    VFEntryCount: 2
; CHECK:  }

; CHECK:   Struct ([[a_complete:0x[A-Z0-9]+]]) {
; CHECK:     TypeLeafKind: LF_STRUCTURE (0x1505)
; CHECK:     MemberCount: 4
; CHECK:     Properties [ (0x200)
; CHECK:       HasUniqueName (0x200)
; CHECK:     ]
; CHECK:     FieldList: <field list>
; CHECK:     DerivedFrom: 0x0
; CHECK:     VShape: 0x0
; CHECK:     SizeOf: 16
; CHECK:     Name: A
; CHECK:     LinkageName: .?AUA@@
; CHECK:   }

; CHECK:  VFTable ([[a_vftable:0x[A-Z0-9]+]]) {
; CHECK:    TypeLeafKind: LF_VFTABLE (0x151D)
; CHECK:    CompleteClass: A ([[a_complete]])
; CHECK:    OverriddenVFTable: 0x0
; CHECK:    VFPtrOffset: 0x0
; CHECK:    VFTableName: ??_7A@@6B@
; CHECK:    MethodName: ?f@A@@UEAAXXZ
; CHECK:    MethodName: ?g@A@@UEAAXXZ
; CHECK:  }

; CHECK:  VFTable ([[c_a_vftable:0x[A-Z0-9]+]]) {
; CHECK:    TypeLeafKind: LF_VFTABLE (0x151D)
; CHECK:    CompleteClass: C
; CHECK:    OverriddenVFTable: ??_7A@@6B@ ([[a_vftable]])
; CHECK:    VFPtrOffset: 0x0
; CHECK:    VFTableName: ??_7C@@6BA@@@
; CHECK:    MethodName: ?f@C@@UEAAXXZ
; CHECK:    MethodName: ?g@C@@UEAAXXZ
; CHECK:  }

; CHECK:  VFTable ({{.*}}) {
; CHECK:    TypeLeafKind: LF_VFTABLE (0x151D)
; CHECK:    CompleteClass: D
; CHECK:    OverriddenVFTable: ??_7C@@6BA@@@ ([[c_a_vftable]])
; CHECK:    VFPtrOffset: 0x0
; CHECK:    VFTableName: ??_7D@@6BA@@@
; CHECK:    MethodName: ?f@D@@UEAAXXZ
; CHECK:    MethodName: ?g@D@@UEAAXXZ
; CHECK:  }

; CHECK:  VFTable ([[b_vftable:0x[A-Z0-9]+]]) {
; CHECK:    TypeLeafKind: LF_VFTABLE (0x151D)
; CHECK:    CompleteClass: B
; CHECK:    OverriddenVFTable: 0x0
; CHECK:    VFPtrOffset: 0x0
; CHECK:    VFTableName: ??_7B@@6B@
; CHECK:    MethodName: ?g@B@@UEAAXXZ
; CHECK:    MethodName: ?f@B@@UEAAXXZ
; CHECK:  }

; CHECK:  VFTable ([[c_b_vftable:0x[A-Z0-9]+]]) {
; CHECK:    TypeLeafKind: LF_VFTABLE (0x151D)
; CHECK:    CompleteClass: C
; CHECK:    OverriddenVFTable: ??_7B@@6B@ ([[b_vftable]])
; CHECK:    VFPtrOffset: 0x10
; CHECK:    VFTableName: ??_7C@@6BB@@@
; CHECK:    MethodName: ?g@C@@WBA@EAAXXZ
; CHECK:    MethodName: ?f@C@@WBA@EAAXXZ
; CHECK:  }

; CHECK:  VFTable ({{.*}}) {
; CHECK:    TypeLeafKind: LF_VFTABLE (0x151D)
; CHECK:    CompleteClass: D
; CHECK:    OverriddenVFTable: ??_7C@@6BB@@@ ([[c_b_vftable]])
; CHECK:    VFPtrOffset: 0x10
; CHECK:    VFTableName: ??_7D@@6BB@@@
; CHECK:    MethodName: ?g@D@@WBA@EAAXXZ
; CHECK:    MethodName: ?f@D@@WBA@EAAXXZ
; CHECK:  }

; ModuleID = 't.cpp'
source_filename = "t.cpp"
target datalayout = "e-m:w-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-pc-windows-msvc"

%rtti.CompleteObjectLocator = type { i32, i32, i32, i32, i32, i32 }
%rtti.TypeDescriptor7 = type { i8**, i8*, [8 x i8] }
%rtti.ClassHierarchyDescriptor = type { i32, i32, i32, i32 }
%rtti.BaseClassDescriptor = type { i32, i32, i32, i32, i32, i32, i32 }
%struct.D = type { %struct.C, i32 }
%struct.C = type { %struct.A, %struct.B, i32 }
%struct.A = type { i32 (...)**, i32 }
%struct.B = type { i32 (...)**, i32 }

$"\01??0D@@QEAA@XZ" = comdat any

$"\01??0C@@QEAA@XZ" = comdat any

$"\01?g@D@@WBA@EAAXXZ" = comdat any

$"\01?f@D@@WBA@EAAXXZ" = comdat any

$"\01??0A@@QEAA@XZ" = comdat any

$"\01??0B@@QEAA@XZ" = comdat any

$"\01?g@C@@WBA@EAAXXZ" = comdat any

$"\01?f@C@@WBA@EAAXXZ" = comdat any

$"\01??_7D@@6BA@@@" = comdat largest

$"\01??_7D@@6BB@@@" = comdat largest

$"\01??_R4D@@6BA@@@" = comdat any

$"\01??_R0?AUD@@@8" = comdat any

$"\01??_R3D@@8" = comdat any

$"\01??_R2D@@8" = comdat any

$"\01??_R1A@?0A@EA@D@@8" = comdat any

$"\01??_R1A@?0A@EA@C@@8" = comdat any

$"\01??_R0?AUC@@@8" = comdat any

$"\01??_R3C@@8" = comdat any

$"\01??_R2C@@8" = comdat any

$"\01??_R1A@?0A@EA@A@@8" = comdat any

$"\01??_R0?AUA@@@8" = comdat any

$"\01??_R3A@@8" = comdat any

$"\01??_R2A@@8" = comdat any

$"\01??_R1BA@?0A@EA@B@@8" = comdat any

$"\01??_R0?AUB@@@8" = comdat any

$"\01??_R3B@@8" = comdat any

$"\01??_R2B@@8" = comdat any

$"\01??_R1A@?0A@EA@B@@8" = comdat any

$"\01??_R4D@@6BB@@@" = comdat any

$"\01??_7C@@6BA@@@" = comdat largest

$"\01??_7C@@6BB@@@" = comdat largest

$"\01??_R4C@@6BA@@@" = comdat any

$"\01??_R4C@@6BB@@@" = comdat any

$"\01??_7A@@6B@" = comdat largest

$"\01??_R4A@@6B@" = comdat any

$"\01??_7B@@6B@" = comdat largest

$"\01??_R4B@@6B@" = comdat any

@0 = private unnamed_addr constant [3 x i8*] [i8* bitcast (%rtti.CompleteObjectLocator* @"\01??_R4D@@6BA@@@" to i8*), i8* bitcast (void (%struct.D*)* @"\01?f@D@@UEAAXXZ" to i8*), i8* bitcast (void (%struct.D*)* @"\01?g@D@@UEAAXXZ" to i8*)], comdat($"\01??_7D@@6BA@@@")
@1 = private unnamed_addr constant [3 x i8*] [i8* bitcast (%rtti.CompleteObjectLocator* @"\01??_R4D@@6BB@@@" to i8*), i8* bitcast (void (%struct.D*)* @"\01?g@D@@WBA@EAAXXZ" to i8*), i8* bitcast (void (%struct.D*)* @"\01?f@D@@WBA@EAAXXZ" to i8*)], comdat($"\01??_7D@@6BB@@@")
@"\01??_R4D@@6BA@@@" = linkonce_odr constant %rtti.CompleteObjectLocator { i32 1, i32 0, i32 0, i32 trunc (i64 sub nuw nsw (i64 ptrtoint (%rtti.TypeDescriptor7* @"\01??_R0?AUD@@@8" to i64), i64 ptrtoint (i8* @__ImageBase to i64)) to i32), i32 trunc (i64 sub nuw nsw (i64 ptrtoint (%rtti.ClassHierarchyDescriptor* @"\01??_R3D@@8" to i64), i64 ptrtoint (i8* @__ImageBase to i64)) to i32), i32 trunc (i64 sub nuw nsw (i64 ptrtoint (%rtti.CompleteObjectLocator* @"\01??_R4D@@6BA@@@" to i64), i64 ptrtoint (i8* @__ImageBase to i64)) to i32) }, comdat
@"\01??_7type_info@@6B@" = external constant i8*
@"\01??_R0?AUD@@@8" = linkonce_odr global %rtti.TypeDescriptor7 { i8** @"\01??_7type_info@@6B@", i8* null, [8 x i8] c".?AUD@@\00" }, comdat
@__ImageBase = external constant i8
@"\01??_R3D@@8" = linkonce_odr constant %rtti.ClassHierarchyDescriptor { i32 0, i32 1, i32 4, i32 trunc (i64 sub nuw nsw (i64 ptrtoint ([5 x i32]* @"\01??_R2D@@8" to i64), i64 ptrtoint (i8* @__ImageBase to i64)) to i32) }, comdat
@"\01??_R2D@@8" = linkonce_odr constant [5 x i32] [i32 trunc (i64 sub nuw nsw (i64 ptrtoint (%rtti.BaseClassDescriptor* @"\01??_R1A@?0A@EA@D@@8" to i64), i64 ptrtoint (i8* @__ImageBase to i64)) to i32), i32 trunc (i64 sub nuw nsw (i64 ptrtoint (%rtti.BaseClassDescriptor* @"\01??_R1A@?0A@EA@C@@8" to i64), i64 ptrtoint (i8* @__ImageBase to i64)) to i32), i32 trunc (i64 sub nuw nsw (i64 ptrtoint (%rtti.BaseClassDescriptor* @"\01??_R1A@?0A@EA@A@@8" to i64), i64 ptrtoint (i8* @__ImageBase to i64)) to i32), i32 trunc (i64 sub nuw nsw (i64 ptrtoint (%rtti.BaseClassDescriptor* @"\01??_R1BA@?0A@EA@B@@8" to i64), i64 ptrtoint (i8* @__ImageBase to i64)) to i32), i32 0], comdat
@"\01??_R1A@?0A@EA@D@@8" = linkonce_odr constant %rtti.BaseClassDescriptor { i32 trunc (i64 sub nuw nsw (i64 ptrtoint (%rtti.TypeDescriptor7* @"\01??_R0?AUD@@@8" to i64), i64 ptrtoint (i8* @__ImageBase to i64)) to i32), i32 3, i32 0, i32 -1, i32 0, i32 64, i32 trunc (i64 sub nuw nsw (i64 ptrtoint (%rtti.ClassHierarchyDescriptor* @"\01??_R3D@@8" to i64), i64 ptrtoint (i8* @__ImageBase to i64)) to i32) }, comdat
@"\01??_R1A@?0A@EA@C@@8" = linkonce_odr constant %rtti.BaseClassDescriptor { i32 trunc (i64 sub nuw nsw (i64 ptrtoint (%rtti.TypeDescriptor7* @"\01??_R0?AUC@@@8" to i64), i64 ptrtoint (i8* @__ImageBase to i64)) to i32), i32 2, i32 0, i32 -1, i32 0, i32 64, i32 trunc (i64 sub nuw nsw (i64 ptrtoint (%rtti.ClassHierarchyDescriptor* @"\01??_R3C@@8" to i64), i64 ptrtoint (i8* @__ImageBase to i64)) to i32) }, comdat
@"\01??_R0?AUC@@@8" = linkonce_odr global %rtti.TypeDescriptor7 { i8** @"\01??_7type_info@@6B@", i8* null, [8 x i8] c".?AUC@@\00" }, comdat
@"\01??_R3C@@8" = linkonce_odr constant %rtti.ClassHierarchyDescriptor { i32 0, i32 1, i32 3, i32 trunc (i64 sub nuw nsw (i64 ptrtoint ([4 x i32]* @"\01??_R2C@@8" to i64), i64 ptrtoint (i8* @__ImageBase to i64)) to i32) }, comdat
@"\01??_R2C@@8" = linkonce_odr constant [4 x i32] [i32 trunc (i64 sub nuw nsw (i64 ptrtoint (%rtti.BaseClassDescriptor* @"\01??_R1A@?0A@EA@C@@8" to i64), i64 ptrtoint (i8* @__ImageBase to i64)) to i32), i32 trunc (i64 sub nuw nsw (i64 ptrtoint (%rtti.BaseClassDescriptor* @"\01??_R1A@?0A@EA@A@@8" to i64), i64 ptrtoint (i8* @__ImageBase to i64)) to i32), i32 trunc (i64 sub nuw nsw (i64 ptrtoint (%rtti.BaseClassDescriptor* @"\01??_R1BA@?0A@EA@B@@8" to i64), i64 ptrtoint (i8* @__ImageBase to i64)) to i32), i32 0], comdat
@"\01??_R1A@?0A@EA@A@@8" = linkonce_odr constant %rtti.BaseClassDescriptor { i32 trunc (i64 sub nuw nsw (i64 ptrtoint (%rtti.TypeDescriptor7* @"\01??_R0?AUA@@@8" to i64), i64 ptrtoint (i8* @__ImageBase to i64)) to i32), i32 0, i32 0, i32 -1, i32 0, i32 64, i32 trunc (i64 sub nuw nsw (i64 ptrtoint (%rtti.ClassHierarchyDescriptor* @"\01??_R3A@@8" to i64), i64 ptrtoint (i8* @__ImageBase to i64)) to i32) }, comdat
@"\01??_R0?AUA@@@8" = linkonce_odr global %rtti.TypeDescriptor7 { i8** @"\01??_7type_info@@6B@", i8* null, [8 x i8] c".?AUA@@\00" }, comdat
@"\01??_R3A@@8" = linkonce_odr constant %rtti.ClassHierarchyDescriptor { i32 0, i32 0, i32 1, i32 trunc (i64 sub nuw nsw (i64 ptrtoint ([2 x i32]* @"\01??_R2A@@8" to i64), i64 ptrtoint (i8* @__ImageBase to i64)) to i32) }, comdat
@"\01??_R2A@@8" = linkonce_odr constant [2 x i32] [i32 trunc (i64 sub nuw nsw (i64 ptrtoint (%rtti.BaseClassDescriptor* @"\01??_R1A@?0A@EA@A@@8" to i64), i64 ptrtoint (i8* @__ImageBase to i64)) to i32), i32 0], comdat
@"\01??_R1BA@?0A@EA@B@@8" = linkonce_odr constant %rtti.BaseClassDescriptor { i32 trunc (i64 sub nuw nsw (i64 ptrtoint (%rtti.TypeDescriptor7* @"\01??_R0?AUB@@@8" to i64), i64 ptrtoint (i8* @__ImageBase to i64)) to i32), i32 0, i32 16, i32 -1, i32 0, i32 64, i32 trunc (i64 sub nuw nsw (i64 ptrtoint (%rtti.ClassHierarchyDescriptor* @"\01??_R3B@@8" to i64), i64 ptrtoint (i8* @__ImageBase to i64)) to i32) }, comdat
@"\01??_R0?AUB@@@8" = linkonce_odr global %rtti.TypeDescriptor7 { i8** @"\01??_7type_info@@6B@", i8* null, [8 x i8] c".?AUB@@\00" }, comdat
@"\01??_R3B@@8" = linkonce_odr constant %rtti.ClassHierarchyDescriptor { i32 0, i32 0, i32 1, i32 trunc (i64 sub nuw nsw (i64 ptrtoint ([2 x i32]* @"\01??_R2B@@8" to i64), i64 ptrtoint (i8* @__ImageBase to i64)) to i32) }, comdat
@"\01??_R2B@@8" = linkonce_odr constant [2 x i32] [i32 trunc (i64 sub nuw nsw (i64 ptrtoint (%rtti.BaseClassDescriptor* @"\01??_R1A@?0A@EA@B@@8" to i64), i64 ptrtoint (i8* @__ImageBase to i64)) to i32), i32 0], comdat
@"\01??_R1A@?0A@EA@B@@8" = linkonce_odr constant %rtti.BaseClassDescriptor { i32 trunc (i64 sub nuw nsw (i64 ptrtoint (%rtti.TypeDescriptor7* @"\01??_R0?AUB@@@8" to i64), i64 ptrtoint (i8* @__ImageBase to i64)) to i32), i32 0, i32 0, i32 -1, i32 0, i32 64, i32 trunc (i64 sub nuw nsw (i64 ptrtoint (%rtti.ClassHierarchyDescriptor* @"\01??_R3B@@8" to i64), i64 ptrtoint (i8* @__ImageBase to i64)) to i32) }, comdat
@"\01??_R4D@@6BB@@@" = linkonce_odr constant %rtti.CompleteObjectLocator { i32 1, i32 16, i32 0, i32 trunc (i64 sub nuw nsw (i64 ptrtoint (%rtti.TypeDescriptor7* @"\01??_R0?AUD@@@8" to i64), i64 ptrtoint (i8* @__ImageBase to i64)) to i32), i32 trunc (i64 sub nuw nsw (i64 ptrtoint (%rtti.ClassHierarchyDescriptor* @"\01??_R3D@@8" to i64), i64 ptrtoint (i8* @__ImageBase to i64)) to i32), i32 trunc (i64 sub nuw nsw (i64 ptrtoint (%rtti.CompleteObjectLocator* @"\01??_R4D@@6BB@@@" to i64), i64 ptrtoint (i8* @__ImageBase to i64)) to i32) }, comdat
@2 = private unnamed_addr constant [3 x i8*] [i8* bitcast (%rtti.CompleteObjectLocator* @"\01??_R4C@@6BA@@@" to i8*), i8* bitcast (void (%struct.C*)* @"\01?f@C@@UEAAXXZ" to i8*), i8* bitcast (void (%struct.C*)* @"\01?g@C@@UEAAXXZ" to i8*)], comdat($"\01??_7C@@6BA@@@")
@3 = private unnamed_addr constant [3 x i8*] [i8* bitcast (%rtti.CompleteObjectLocator* @"\01??_R4C@@6BB@@@" to i8*), i8* bitcast (void (%struct.C*)* @"\01?g@C@@WBA@EAAXXZ" to i8*), i8* bitcast (void (%struct.C*)* @"\01?f@C@@WBA@EAAXXZ" to i8*)], comdat($"\01??_7C@@6BB@@@")
@"\01??_R4C@@6BA@@@" = linkonce_odr constant %rtti.CompleteObjectLocator { i32 1, i32 0, i32 0, i32 trunc (i64 sub nuw nsw (i64 ptrtoint (%rtti.TypeDescriptor7* @"\01??_R0?AUC@@@8" to i64), i64 ptrtoint (i8* @__ImageBase to i64)) to i32), i32 trunc (i64 sub nuw nsw (i64 ptrtoint (%rtti.ClassHierarchyDescriptor* @"\01??_R3C@@8" to i64), i64 ptrtoint (i8* @__ImageBase to i64)) to i32), i32 trunc (i64 sub nuw nsw (i64 ptrtoint (%rtti.CompleteObjectLocator* @"\01??_R4C@@6BA@@@" to i64), i64 ptrtoint (i8* @__ImageBase to i64)) to i32) }, comdat
@"\01??_R4C@@6BB@@@" = linkonce_odr constant %rtti.CompleteObjectLocator { i32 1, i32 16, i32 0, i32 trunc (i64 sub nuw nsw (i64 ptrtoint (%rtti.TypeDescriptor7* @"\01??_R0?AUC@@@8" to i64), i64 ptrtoint (i8* @__ImageBase to i64)) to i32), i32 trunc (i64 sub nuw nsw (i64 ptrtoint (%rtti.ClassHierarchyDescriptor* @"\01??_R3C@@8" to i64), i64 ptrtoint (i8* @__ImageBase to i64)) to i32), i32 trunc (i64 sub nuw nsw (i64 ptrtoint (%rtti.CompleteObjectLocator* @"\01??_R4C@@6BB@@@" to i64), i64 ptrtoint (i8* @__ImageBase to i64)) to i32) }, comdat
@4 = private unnamed_addr constant [3 x i8*] [i8* bitcast (%rtti.CompleteObjectLocator* @"\01??_R4A@@6B@" to i8*), i8* bitcast (void (%struct.A*)* @"\01?f@A@@UEAAXXZ" to i8*), i8* bitcast (void (%struct.A*)* @"\01?g@A@@UEAAXXZ" to i8*)], comdat($"\01??_7A@@6B@")
@"\01??_R4A@@6B@" = linkonce_odr constant %rtti.CompleteObjectLocator { i32 1, i32 0, i32 0, i32 trunc (i64 sub nuw nsw (i64 ptrtoint (%rtti.TypeDescriptor7* @"\01??_R0?AUA@@@8" to i64), i64 ptrtoint (i8* @__ImageBase to i64)) to i32), i32 trunc (i64 sub nuw nsw (i64 ptrtoint (%rtti.ClassHierarchyDescriptor* @"\01??_R3A@@8" to i64), i64 ptrtoint (i8* @__ImageBase to i64)) to i32), i32 trunc (i64 sub nuw nsw (i64 ptrtoint (%rtti.CompleteObjectLocator* @"\01??_R4A@@6B@" to i64), i64 ptrtoint (i8* @__ImageBase to i64)) to i32) }, comdat
@5 = private unnamed_addr constant [3 x i8*] [i8* bitcast (%rtti.CompleteObjectLocator* @"\01??_R4B@@6B@" to i8*), i8* bitcast (void (%struct.B*)* @"\01?g@B@@UEAAXXZ" to i8*), i8* bitcast (void (%struct.B*)* @"\01?f@B@@UEAAXXZ" to i8*)], comdat($"\01??_7B@@6B@")
@"\01??_R4B@@6B@" = linkonce_odr constant %rtti.CompleteObjectLocator { i32 1, i32 0, i32 0, i32 trunc (i64 sub nuw nsw (i64 ptrtoint (%rtti.TypeDescriptor7* @"\01??_R0?AUB@@@8" to i64), i64 ptrtoint (i8* @__ImageBase to i64)) to i32), i32 trunc (i64 sub nuw nsw (i64 ptrtoint (%rtti.ClassHierarchyDescriptor* @"\01??_R3B@@8" to i64), i64 ptrtoint (i8* @__ImageBase to i64)) to i32), i32 trunc (i64 sub nuw nsw (i64 ptrtoint (%rtti.CompleteObjectLocator* @"\01??_R4B@@6B@" to i64), i64 ptrtoint (i8* @__ImageBase to i64)) to i32) }, comdat

@"\01??_7D@@6BA@@@" = unnamed_addr alias i8*, getelementptr inbounds ([3 x i8*], [3 x i8*]* @0, i32 0, i32 1)
@"\01??_7D@@6BB@@@" = unnamed_addr alias i8*, getelementptr inbounds ([3 x i8*], [3 x i8*]* @1, i32 0, i32 1)
@"\01??_7C@@6BA@@@" = unnamed_addr alias i8*, getelementptr inbounds ([3 x i8*], [3 x i8*]* @2, i32 0, i32 1)
@"\01??_7C@@6BB@@@" = unnamed_addr alias i8*, getelementptr inbounds ([3 x i8*], [3 x i8*]* @3, i32 0, i32 1)
@"\01??_7A@@6B@" = unnamed_addr alias i8*, getelementptr inbounds ([3 x i8*], [3 x i8*]* @4, i32 0, i32 1)
@"\01??_7B@@6B@" = unnamed_addr alias i8*, getelementptr inbounds ([3 x i8*], [3 x i8*]* @5, i32 0, i32 1)

; Function Attrs: nounwind
define void @"\01?h@@YAXXZ"() #0 !dbg !6 {
entry:
  %d = alloca %struct.D, align 8
  call void @llvm.dbg.declare(metadata %struct.D* %d, metadata !10, metadata !65), !dbg !66
  %call = call %struct.D* @"\01??0D@@QEAA@XZ"(%struct.D* %d) #4, !dbg !66
  ret void, !dbg !66
}

; Function Attrs: nounwind readnone
declare void @llvm.dbg.declare(metadata, metadata, metadata) #1

; Function Attrs: inlinehint nounwind
define linkonce_odr %struct.D* @"\01??0D@@QEAA@XZ"(%struct.D* returned %this) unnamed_addr #2 comdat align 2 !dbg !67 {
entry:
  %this.addr = alloca %struct.D*, align 8
  store %struct.D* %this, %struct.D** %this.addr, align 8
  call void @llvm.dbg.declare(metadata %struct.D** %this.addr, metadata !69, metadata !65), !dbg !71
  %this1 = load %struct.D*, %struct.D** %this.addr, align 8
  %0 = bitcast %struct.D* %this1 to %struct.C*, !dbg !72
  %call = call %struct.C* @"\01??0C@@QEAA@XZ"(%struct.C* %0) #4, !dbg !72
  %1 = bitcast %struct.D* %this1 to i32 (...)***, !dbg !72
  store i32 (...)** bitcast (i8** @"\01??_7D@@6BA@@@" to i32 (...)**), i32 (...)*** %1, align 8, !dbg !72
  %2 = bitcast %struct.D* %this1 to i8*, !dbg !72
  %add.ptr = getelementptr inbounds i8, i8* %2, i64 16, !dbg !72
  %3 = bitcast i8* %add.ptr to i32 (...)***, !dbg !72
  store i32 (...)** bitcast (i8** @"\01??_7D@@6BB@@@" to i32 (...)**), i32 (...)*** %3, align 8, !dbg !72
  %d = getelementptr inbounds %struct.D, %struct.D* %this1, i32 0, i32 1, !dbg !73
  store i32 0, i32* %d, align 8, !dbg !73
  ret %struct.D* %this1, !dbg !72
}

; Function Attrs: inlinehint nounwind
define linkonce_odr %struct.C* @"\01??0C@@QEAA@XZ"(%struct.C* returned %this) unnamed_addr #2 comdat align 2 !dbg !74 {
entry:
  %this.addr = alloca %struct.C*, align 8
  store %struct.C* %this, %struct.C** %this.addr, align 8
  call void @llvm.dbg.declare(metadata %struct.C** %this.addr, metadata !76, metadata !65), !dbg !78
  %this1 = load %struct.C*, %struct.C** %this.addr, align 8
  %0 = bitcast %struct.C* %this1 to %struct.A*, !dbg !79
  %call = call %struct.A* @"\01??0A@@QEAA@XZ"(%struct.A* %0) #4, !dbg !79
  %1 = bitcast %struct.C* %this1 to i8*, !dbg !79
  %2 = getelementptr inbounds i8, i8* %1, i64 16, !dbg !79
  %3 = bitcast i8* %2 to %struct.B*, !dbg !79
  %call2 = call %struct.B* @"\01??0B@@QEAA@XZ"(%struct.B* %3) #4, !dbg !79
  %4 = bitcast %struct.C* %this1 to i32 (...)***, !dbg !79
  store i32 (...)** bitcast (i8** @"\01??_7C@@6BA@@@" to i32 (...)**), i32 (...)*** %4, align 8, !dbg !79
  %5 = bitcast %struct.C* %this1 to i8*, !dbg !79
  %add.ptr = getelementptr inbounds i8, i8* %5, i64 16, !dbg !79
  %6 = bitcast i8* %add.ptr to i32 (...)***, !dbg !79
  store i32 (...)** bitcast (i8** @"\01??_7C@@6BB@@@" to i32 (...)**), i32 (...)*** %6, align 8, !dbg !79
  %c = getelementptr inbounds %struct.C, %struct.C* %this1, i32 0, i32 2, !dbg !80
  store i32 0, i32* %c, align 8, !dbg !80
  ret %struct.C* %this1, !dbg !79
}

declare void @"\01?f@D@@UEAAXXZ"(%struct.D*) unnamed_addr #3

declare void @"\01?g@D@@UEAAXXZ"(%struct.D*) unnamed_addr #3

; Function Attrs: nounwind
define linkonce_odr void @"\01?g@D@@WBA@EAAXXZ"(%struct.D* %this) unnamed_addr #0 comdat align 2 !dbg !81 {
entry:
  %this.addr = alloca %struct.D*, align 8
  store %struct.D* %this, %struct.D** %this.addr, align 8
  call void @llvm.dbg.declare(metadata %struct.D** %this.addr, metadata !83, metadata !65), !dbg !84
  %this1 = load %struct.D*, %struct.D** %this.addr, align 8, !dbg !85
  %0 = bitcast %struct.D* %this1 to i8*, !dbg !85
  %1 = getelementptr i8, i8* %0, i32 -16, !dbg !85
  %2 = bitcast i8* %1 to %struct.D*, !dbg !85
  tail call void @"\01?g@D@@UEAAXXZ"(%struct.D* %2), !dbg !85
  ret void, !dbg !85
}

; Function Attrs: nounwind
define linkonce_odr void @"\01?f@D@@WBA@EAAXXZ"(%struct.D* %this) unnamed_addr #0 comdat align 2 !dbg !86 {
entry:
  %this.addr = alloca %struct.D*, align 8
  store %struct.D* %this, %struct.D** %this.addr, align 8
  call void @llvm.dbg.declare(metadata %struct.D** %this.addr, metadata !87, metadata !65), !dbg !88
  %this1 = load %struct.D*, %struct.D** %this.addr, align 8, !dbg !89
  %0 = bitcast %struct.D* %this1 to i8*, !dbg !89
  %1 = getelementptr i8, i8* %0, i32 -16, !dbg !89
  %2 = bitcast i8* %1 to %struct.D*, !dbg !89
  tail call void @"\01?f@D@@UEAAXXZ"(%struct.D* %2), !dbg !89
  ret void, !dbg !89
}

; Function Attrs: inlinehint nounwind
define linkonce_odr %struct.A* @"\01??0A@@QEAA@XZ"(%struct.A* returned %this) unnamed_addr #2 comdat align 2 !dbg !90 {
entry:
  %this.addr = alloca %struct.A*, align 8
  store %struct.A* %this, %struct.A** %this.addr, align 8
  call void @llvm.dbg.declare(metadata %struct.A** %this.addr, metadata !92, metadata !65), !dbg !94
  %this1 = load %struct.A*, %struct.A** %this.addr, align 8
  %0 = bitcast %struct.A* %this1 to i32 (...)***, !dbg !95
  store i32 (...)** bitcast (i8** @"\01??_7A@@6B@" to i32 (...)**), i32 (...)*** %0, align 8, !dbg !95
  %a = getelementptr inbounds %struct.A, %struct.A* %this1, i32 0, i32 1, !dbg !96
  store i32 0, i32* %a, align 8, !dbg !96
  ret %struct.A* %this1, !dbg !95
}

; Function Attrs: inlinehint nounwind
define linkonce_odr %struct.B* @"\01??0B@@QEAA@XZ"(%struct.B* returned %this) unnamed_addr #2 comdat align 2 !dbg !97 {
entry:
  %this.addr = alloca %struct.B*, align 8
  store %struct.B* %this, %struct.B** %this.addr, align 8
  call void @llvm.dbg.declare(metadata %struct.B** %this.addr, metadata !99, metadata !65), !dbg !101
  %this1 = load %struct.B*, %struct.B** %this.addr, align 8
  %0 = bitcast %struct.B* %this1 to i32 (...)***, !dbg !102
  store i32 (...)** bitcast (i8** @"\01??_7B@@6B@" to i32 (...)**), i32 (...)*** %0, align 8, !dbg !102
  %b = getelementptr inbounds %struct.B, %struct.B* %this1, i32 0, i32 1, !dbg !103
  store i32 0, i32* %b, align 8, !dbg !103
  ret %struct.B* %this1, !dbg !102
}

declare void @"\01?f@C@@UEAAXXZ"(%struct.C*) unnamed_addr #3

declare void @"\01?g@C@@UEAAXXZ"(%struct.C*) unnamed_addr #3

; Function Attrs: nounwind
define linkonce_odr void @"\01?g@C@@WBA@EAAXXZ"(%struct.C* %this) unnamed_addr #0 comdat align 2 !dbg !104 {
entry:
  %this.addr = alloca %struct.C*, align 8
  store %struct.C* %this, %struct.C** %this.addr, align 8
  call void @llvm.dbg.declare(metadata %struct.C** %this.addr, metadata !105, metadata !65), !dbg !106
  %this1 = load %struct.C*, %struct.C** %this.addr, align 8, !dbg !107
  %0 = bitcast %struct.C* %this1 to i8*, !dbg !107
  %1 = getelementptr i8, i8* %0, i32 -16, !dbg !107
  %2 = bitcast i8* %1 to %struct.C*, !dbg !107
  tail call void @"\01?g@C@@UEAAXXZ"(%struct.C* %2), !dbg !107
  ret void, !dbg !107
}

; Function Attrs: nounwind
define linkonce_odr void @"\01?f@C@@WBA@EAAXXZ"(%struct.C* %this) unnamed_addr #0 comdat align 2 !dbg !108 {
entry:
  %this.addr = alloca %struct.C*, align 8
  store %struct.C* %this, %struct.C** %this.addr, align 8
  call void @llvm.dbg.declare(metadata %struct.C** %this.addr, metadata !109, metadata !65), !dbg !110
  %this1 = load %struct.C*, %struct.C** %this.addr, align 8, !dbg !111
  %0 = bitcast %struct.C* %this1 to i8*, !dbg !111
  %1 = getelementptr i8, i8* %0, i32 -16, !dbg !111
  %2 = bitcast i8* %1 to %struct.C*, !dbg !111
  tail call void @"\01?f@C@@UEAAXXZ"(%struct.C* %2), !dbg !111
  ret void, !dbg !111
}

declare void @"\01?f@A@@UEAAXXZ"(%struct.A*) unnamed_addr #3

declare void @"\01?g@A@@UEAAXXZ"(%struct.A*) unnamed_addr #3

declare void @"\01?g@B@@UEAAXXZ"(%struct.B*) unnamed_addr #3

declare void @"\01?f@B@@UEAAXXZ"(%struct.B*) unnamed_addr #3

attributes #0 = { nounwind "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="false" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "stack-protector-buffer-size"="8" "target-features"="+mmx,+sse,+sse2,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #1 = { nounwind readnone }
attributes #2 = { inlinehint nounwind "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="false" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "stack-protector-buffer-size"="8" "target-features"="+mmx,+sse,+sse2,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #3 = { "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="false" "no-infs-fp-math"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "stack-protector-buffer-size"="8" "target-features"="+mmx,+sse,+sse2,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #4 = { nounwind }

!llvm.dbg.cu = !{!0}
!llvm.module.flags = !{!3, !4}
!llvm.ident = !{!5}

!0 = distinct !DICompileUnit(language: DW_LANG_C_plus_plus, file: !1, producer: "clang version 4.0.0 ", isOptimized: false, runtimeVersion: 0, emissionKind: FullDebug, enums: !2)
!1 = !DIFile(filename: "<stdin>", directory: "D:\5Csrc\5Cllvm\5Cbuild")
!2 = !{}
!3 = !{i32 2, !"CodeView", i32 1}
!4 = !{i32 2, !"Debug Info Version", i32 3}
!5 = !{!"clang version 4.0.0 "}
!6 = distinct !DISubprogram(name: "h", linkageName: "\01?h@@YAXXZ", scope: !7, file: !7, line: 21, type: !8, isLocal: false, isDefinition: true, scopeLine: 21, flags: DIFlagPrototyped, isOptimized: false, unit: !0, variables: !2)
!7 = !DIFile(filename: "t.cpp", directory: "D:\5Csrc\5Cllvm\5Cbuild")
!8 = !DISubroutineType(types: !9)
!9 = !{null}
!10 = !DILocalVariable(name: "d", scope: !6, file: !7, line: 21, type: !11)
!11 = distinct !DICompositeType(tag: DW_TAG_structure_type, name: "D", file: !7, line: 16, size: 384, align: 64, elements: !12, vtableHolder: !17, identifier: ".?AUD@@")
!12 = !{!13, !55, !57, !59, !60, !64}
!13 = !DIDerivedType(tag: DW_TAG_inheritance, scope: !11, baseType: !14)
!14 = distinct !DICompositeType(tag: DW_TAG_structure_type, name: "C", file: !7, line: 11, size: 320, align: 64, elements: !15, vtableHolder: !17, identifier: ".?AUC@@")
!15 = !{!16, !33, !45, !47, !49, !50, !54}
!16 = !DIDerivedType(tag: DW_TAG_inheritance, scope: !14, baseType: !17)
!17 = distinct !DICompositeType(tag: DW_TAG_structure_type, name: "A", file: !7, line: 1, size: 128, align: 64, elements: !18, vtableHolder: !17, identifier: ".?AUA@@")
!18 = !{!19, !21, !27, !28, !32}
!19 = !DIDerivedType(tag: DW_TAG_LLVM_msvftable, name: "??_7A@@6B@", scope: !17, baseType: null, extraData: !20)
!20 = !{!"?f@A@@UEAAXXZ", !"?g@A@@UEAAXXZ"}
!21 = !DIDerivedType(tag: DW_TAG_member, name: "_vptr$A", scope: !7, file: !7, baseType: !22, size: 64, flags: DIFlagArtificial)
!22 = !DIDerivedType(tag: DW_TAG_pointer_type, baseType: !23, size: 64)
!23 = !DIDerivedType(tag: DW_TAG_pointer_type, name: "__vtbl_ptr_type", baseType: !24, size: 64)
!24 = !DISubroutineType(types: !25)
!25 = !{!26}
!26 = !DIBasicType(name: "int", size: 32, align: 32, encoding: DW_ATE_signed)
!27 = !DIDerivedType(tag: DW_TAG_member, name: "a", scope: !17, file: !7, line: 4, baseType: !26, size: 32, align: 32, offset: 64)
!28 = !DISubprogram(name: "f", linkageName: "\01?f@A@@UEAAXXZ", scope: !17, file: !7, line: 2, type: !29, isLocal: false, isDefinition: false, scopeLine: 2, containingType: !17, virtuality: DW_VIRTUALITY_virtual, virtualIndex: 0, flags: DIFlagPrototyped | DIFlagIntroducedVirtual, isOptimized: false)
!29 = !DISubroutineType(types: !30)
!30 = !{null, !31}
!31 = !DIDerivedType(tag: DW_TAG_pointer_type, baseType: !17, size: 64, align: 64, flags: DIFlagArtificial | DIFlagObjectPointer)
!32 = !DISubprogram(name: "g", linkageName: "\01?g@A@@UEAAXXZ", scope: !17, file: !7, line: 3, type: !29, isLocal: false, isDefinition: false, scopeLine: 3, containingType: !17, virtuality: DW_VIRTUALITY_virtual, virtualIndex: 1, flags: DIFlagPrototyped | DIFlagIntroducedVirtual, isOptimized: false)
!33 = !DIDerivedType(tag: DW_TAG_inheritance, scope: !14, baseType: !34, offset: 128)
!34 = distinct !DICompositeType(tag: DW_TAG_structure_type, name: "B", file: !7, line: 6, size: 128, align: 64, elements: !35, vtableHolder: !34, identifier: ".?AUB@@")
!35 = !{!36, !38, !39, !40, !44}
!36 = !DIDerivedType(tag: DW_TAG_LLVM_msvftable, name: "??_7B@@6B@", scope: !34, baseType: null, extraData: !37)
!37 = !{!"?g@B@@UEAAXXZ", !"?f@B@@UEAAXXZ"}
!38 = !DIDerivedType(tag: DW_TAG_member, name: "_vptr$B", scope: !7, file: !7, baseType: !22, size: 64, flags: DIFlagArtificial)
!39 = !DIDerivedType(tag: DW_TAG_member, name: "b", scope: !34, file: !7, line: 9, baseType: !26, size: 32, align: 32, offset: 64)
!40 = !DISubprogram(name: "g", linkageName: "\01?g@B@@UEAAXXZ", scope: !34, file: !7, line: 7, type: !41, isLocal: false, isDefinition: false, scopeLine: 7, containingType: !34, virtuality: DW_VIRTUALITY_virtual, virtualIndex: 0, flags: DIFlagPrototyped | DIFlagIntroducedVirtual, isOptimized: false)
!41 = !DISubroutineType(types: !42)
!42 = !{null, !43}
!43 = !DIDerivedType(tag: DW_TAG_pointer_type, baseType: !34, size: 64, align: 64, flags: DIFlagArtificial | DIFlagObjectPointer)
!44 = !DISubprogram(name: "f", linkageName: "\01?f@B@@UEAAXXZ", scope: !34, file: !7, line: 8, type: !41, isLocal: false, isDefinition: false, scopeLine: 8, containingType: !34, virtuality: DW_VIRTUALITY_virtual, virtualIndex: 1, flags: DIFlagPrototyped | DIFlagIntroducedVirtual, isOptimized: false)
!45 = !DIDerivedType(tag: DW_TAG_LLVM_msvftable, name: "??_7C@@6BA@@@", scope: !14, baseType: !19, extraData: !46)
!46 = !{!"?f@C@@UEAAXXZ", !"?g@C@@UEAAXXZ"}
!47 = !DIDerivedType(tag: DW_TAG_LLVM_msvftable, name: "??_7C@@6BB@@@", scope: !14, baseType: !36, offset: 128, extraData: !48)
!48 = !{!"?g@C@@WBA@EAAXXZ", !"?f@C@@WBA@EAAXXZ"}
!49 = !DIDerivedType(tag: DW_TAG_member, name: "c", scope: !14, file: !7, line: 14, baseType: !26, size: 32, align: 32, offset: 256)
!50 = !DISubprogram(name: "g", linkageName: "\01?g@C@@UEAAXXZ", scope: !14, file: !7, line: 12, type: !51, isLocal: false, isDefinition: false, scopeLine: 12, containingType: !14, virtuality: DW_VIRTUALITY_virtual, virtualIndex: 1, flags: DIFlagPrototyped, isOptimized: false)
!51 = !DISubroutineType(types: !52)
!52 = !{null, !53}
!53 = !DIDerivedType(tag: DW_TAG_pointer_type, baseType: !14, size: 64, align: 64, flags: DIFlagArtificial | DIFlagObjectPointer)
!54 = !DISubprogram(name: "f", linkageName: "\01?f@C@@UEAAXXZ", scope: !14, file: !7, line: 13, type: !51, isLocal: false, isDefinition: false, scopeLine: 13, containingType: !14, virtuality: DW_VIRTUALITY_virtual, virtualIndex: 0, flags: DIFlagPrototyped, isOptimized: false)
!55 = !DIDerivedType(tag: DW_TAG_LLVM_msvftable, name: "??_7D@@6BA@@@", scope: !11, baseType: !45, extraData: !56)
!56 = !{!"?f@D@@UEAAXXZ", !"?g@D@@UEAAXXZ"}
!57 = !DIDerivedType(tag: DW_TAG_LLVM_msvftable, name: "??_7D@@6BB@@@", scope: !11, baseType: !47, offset: 128, extraData: !58)
!58 = !{!"?g@D@@WBA@EAAXXZ", !"?f@D@@WBA@EAAXXZ"}
!59 = !DIDerivedType(tag: DW_TAG_member, name: "d", scope: !11, file: !7, line: 19, baseType: !26, size: 32, align: 32, offset: 320)
!60 = !DISubprogram(name: "g", linkageName: "\01?g@D@@UEAAXXZ", scope: !11, file: !7, line: 17, type: !61, isLocal: false, isDefinition: false, scopeLine: 17, containingType: !11, virtuality: DW_VIRTUALITY_virtual, virtualIndex: 1, flags: DIFlagPrototyped, isOptimized: false)
!61 = !DISubroutineType(types: !62)
!62 = !{null, !63}
!63 = !DIDerivedType(tag: DW_TAG_pointer_type, baseType: !11, size: 64, align: 64, flags: DIFlagArtificial | DIFlagObjectPointer)
!64 = !DISubprogram(name: "f", linkageName: "\01?f@D@@UEAAXXZ", scope: !11, file: !7, line: 18, type: !61, isLocal: false, isDefinition: false, scopeLine: 18, containingType: !11, virtuality: DW_VIRTUALITY_virtual, virtualIndex: 0, flags: DIFlagPrototyped, isOptimized: false)
!65 = !DIExpression()
!66 = !DILocation(line: 21, scope: !6)
!67 = distinct !DISubprogram(name: "D", linkageName: "\01??0D@@QEAA@XZ", scope: !11, file: !7, line: 16, type: !61, isLocal: false, isDefinition: true, scopeLine: 16, flags: DIFlagArtificial | DIFlagPrototyped, isOptimized: false, unit: !0, declaration: !68, variables: !2)
!68 = !DISubprogram(name: "D", scope: !11, type: !61, isLocal: false, isDefinition: false, flags: DIFlagArtificial | DIFlagPrototyped, isOptimized: false)
!69 = !DILocalVariable(name: "this", arg: 1, scope: !67, type: !70, flags: DIFlagArtificial | DIFlagObjectPointer)
!70 = !DIDerivedType(tag: DW_TAG_pointer_type, baseType: !11, size: 64, align: 64)
!71 = !DILocation(line: 0, scope: !67)
!72 = !DILocation(line: 16, scope: !67)
!73 = !DILocation(line: 19, scope: !67)
!74 = distinct !DISubprogram(name: "C", linkageName: "\01??0C@@QEAA@XZ", scope: !14, file: !7, line: 11, type: !51, isLocal: false, isDefinition: true, scopeLine: 11, flags: DIFlagArtificial | DIFlagPrototyped, isOptimized: false, unit: !0, declaration: !75, variables: !2)
!75 = !DISubprogram(name: "C", scope: !14, type: !51, isLocal: false, isDefinition: false, flags: DIFlagArtificial | DIFlagPrototyped, isOptimized: false)
!76 = !DILocalVariable(name: "this", arg: 1, scope: !74, type: !77, flags: DIFlagArtificial | DIFlagObjectPointer)
!77 = !DIDerivedType(tag: DW_TAG_pointer_type, baseType: !14, size: 64, align: 64)
!78 = !DILocation(line: 0, scope: !74)
!79 = !DILocation(line: 11, scope: !74)
!80 = !DILocation(line: 14, scope: !74)
!81 = distinct !DISubprogram(linkageName: "\01?g@D@@WBA@EAAXXZ", scope: !7, file: !7, line: 17, type: !82, isLocal: false, isDefinition: true, scopeLine: 17, flags: DIFlagArtificial, isOptimized: false, unit: !0, variables: !2)
!82 = !DISubroutineType(types: !2)
!83 = !DILocalVariable(name: "this", arg: 1, scope: !81, type: !70, flags: DIFlagArtificial | DIFlagObjectPointer)
!84 = !DILocation(line: 0, scope: !81)
!85 = !DILocation(line: 17, scope: !81)
!86 = distinct !DISubprogram(linkageName: "\01?f@D@@WBA@EAAXXZ", scope: !7, file: !7, line: 18, type: !82, isLocal: false, isDefinition: true, scopeLine: 18, flags: DIFlagArtificial, isOptimized: false, unit: !0, variables: !2)
!87 = !DILocalVariable(name: "this", arg: 1, scope: !86, type: !70, flags: DIFlagArtificial | DIFlagObjectPointer)
!88 = !DILocation(line: 0, scope: !86)
!89 = !DILocation(line: 18, scope: !86)
!90 = distinct !DISubprogram(name: "A", linkageName: "\01??0A@@QEAA@XZ", scope: !17, file: !7, line: 1, type: !29, isLocal: false, isDefinition: true, scopeLine: 1, flags: DIFlagArtificial | DIFlagPrototyped, isOptimized: false, unit: !0, declaration: !91, variables: !2)
!91 = !DISubprogram(name: "A", scope: !17, type: !29, isLocal: false, isDefinition: false, flags: DIFlagArtificial | DIFlagPrototyped, isOptimized: false)
!92 = !DILocalVariable(name: "this", arg: 1, scope: !90, type: !93, flags: DIFlagArtificial | DIFlagObjectPointer)
!93 = !DIDerivedType(tag: DW_TAG_pointer_type, baseType: !17, size: 64, align: 64)
!94 = !DILocation(line: 0, scope: !90)
!95 = !DILocation(line: 1, scope: !90)
!96 = !DILocation(line: 4, scope: !90)
!97 = distinct !DISubprogram(name: "B", linkageName: "\01??0B@@QEAA@XZ", scope: !34, file: !7, line: 6, type: !41, isLocal: false, isDefinition: true, scopeLine: 6, flags: DIFlagArtificial | DIFlagPrototyped, isOptimized: false, unit: !0, declaration: !98, variables: !2)
!98 = !DISubprogram(name: "B", scope: !34, type: !41, isLocal: false, isDefinition: false, flags: DIFlagArtificial | DIFlagPrototyped, isOptimized: false)
!99 = !DILocalVariable(name: "this", arg: 1, scope: !97, type: !100, flags: DIFlagArtificial | DIFlagObjectPointer)
!100 = !DIDerivedType(tag: DW_TAG_pointer_type, baseType: !34, size: 64, align: 64)
!101 = !DILocation(line: 0, scope: !97)
!102 = !DILocation(line: 6, scope: !97)
!103 = !DILocation(line: 9, scope: !97)
!104 = distinct !DISubprogram(linkageName: "\01?g@C@@WBA@EAAXXZ", scope: !7, file: !7, line: 12, type: !82, isLocal: false, isDefinition: true, scopeLine: 12, flags: DIFlagArtificial, isOptimized: false, unit: !0, variables: !2)
!105 = !DILocalVariable(name: "this", arg: 1, scope: !104, type: !77, flags: DIFlagArtificial | DIFlagObjectPointer)
!106 = !DILocation(line: 0, scope: !104)
!107 = !DILocation(line: 12, scope: !104)
!108 = distinct !DISubprogram(linkageName: "\01?f@C@@WBA@EAAXXZ", scope: !7, file: !7, line: 13, type: !82, isLocal: false, isDefinition: true, scopeLine: 13, flags: DIFlagArtificial, isOptimized: false, unit: !0, variables: !2)
!109 = !DILocalVariable(name: "this", arg: 1, scope: !108, type: !77, flags: DIFlagArtificial | DIFlagObjectPointer)
!110 = !DILocation(line: 0, scope: !108)
!111 = !DILocation(line: 13, scope: !108)
