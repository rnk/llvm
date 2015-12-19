//===-- CodeView.h - On-disk record types for CodeView ----------*- C++ -*-===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
///
/// \file
/// \brief This file provides data structures useful for consuming on-disk
/// CodeView. It is based on information published by Microsoft at
/// https://github.com/Microsoft/microsoft-pdb/.
///
//===----------------------------------------------------------------------===//

// FIXME: Move this to include/llvm/DebugInfo/CodeView/ when that library is
// added.

#ifndef LLVM_READOBJ_CODEVIEW_H
#define LLVM_READOBJ_CODEVIEW_H

#include "llvm/Support/Endian.h"

namespace llvm {
namespace codeview {

/// A 32-bit type reference. Types are indexed by their order of appearance in
/// .debug$T plus 0x1000. Type indices less than 0x1000 refer to builtin types
/// in the BuiltinType enum.
typedef ulittle32_t TypeIndex;

/// Any .debug$S section can be broken into subsections with these types.
/// Equivalent to DEBUG_S_SUBSECTION_TYPE in cvinfo.h.
enum SubSectionType : unsigned {
  // If this bit is set in a subsection type then ignore the subsection
  // contents.
  SUBSEC_IGNORE = 0x80000000,
  SUBSEC_SYMBOLS = 0xf1,
  SUBSEC_LINES,
  SUBSEC_STRINGTABLE,
  SUBSEC_FILECHKSMS,
  SUBSEC_FRAMEDATA,
  SUBSEC_INLINEELINES,
  SUBSEC_CROSSSCOPEIMPORTS,
  SUBSEC_CROSSSCOPEEXPORTS,
  SUBSEC_IL_LINES,
  SUBSEC_FUNC_MDTOKEN_MAP,
  SUBSEC_TYPE_MDTOKEN_MAP,
  SUBSEC_MERGED_ASSEMBLYINPUT,
  SUBSEC_COFF_SYMBOL_RVA,
};

/// A SUBSEC_SYMBOLS subsection is a sequence of SymRecords. Advancing by 'len'
/// bytes will find the next SymRecord. These are the possible types of a
/// record. Equivalent to SYM_ENUM_e in cvinfo.h.
enum SymType : uint16_t {
#define SYMBOL_TYPE(ename, value) ename = value,
#include "CVSymbolTypes.def"
};

/// Generic record compatible with all symbol records.
struct SymRecord {
  ulittle16_t reclen; // Record length, starting from the next field
  ulittle16_t rectyp; // Record type
  // char data[];
};

// S_GPROC32, S_LPROC32, S_GPROC32_ID, S_LPROC32_ID, S_LPROC32_DPC or
// S_LPROC32_DPC_ID
struct ProcSym {
  ulittle32_t pParent;  // pointer to the parent
  ulittle32_t pEnd;     // pointer to this blocks end
  ulittle32_t pNext;    // pointer to next symbol
  ulittle32_t len;      // Proc length
  ulittle32_t DbgStart; // Debug start offset
  ulittle32_t DbgEnd;   // Debug end offset
  ulittle32_t typind;   // Type index or ID
  ulittle32_t off;
  ulittle16_t seg;
  uint8_t flags; // Proc flags
  //char name[1];  // Length-prefixed name
};

// LF_FUNC_ID
struct FuncId {
  TypeIndex ParentScope;
  TypeIndex FunctionType;
  // Name: The null-terminated name follows.
};

// S_OBJNAME
struct ObjNameSym {
  ulittle32_t signature; // signature
  // Name: The null-terminated name follows.
};

enum SourceLanguage : uint8_t {
  C = 0x00,
  CXX = 0x01,
  FORTRAN = 0x02,
  MASM = 0x03,
  PASCAL = 0x04,
  BASIC = 0x05,
  COBOL = 0x06,
  LINK = 0x07,
  CVTRES = 0x08,
  CVTPGD = 0x09,
  CSHARP = 0x0A,
  VP = 0x0B,
  ILASM = 0x0C,
  JAVA = 0x0D,
  JSCRIPT = 0x0E,
  MSIL = 0x0F,
  HLSL = 0x10,
};

enum CPUType : uint16_t {
  CPU_8080 = 0x00,
  CPU_8086 = 0x01,
  CPU_80286 = 0x02,
  CPU_80386 = 0x03,
  CPU_80486 = 0x04,
  CPU_PENTIUM = 0x05,
  CPU_PENTIUMII = 0x06,
  CPU_PENTIUMPRO = CPU_PENTIUMII,
  CPU_PENTIUMIII = 0x07,
  CPU_MIPS = 0x10,
  CPU_MIPSR4000 = CPU_MIPS,
  CPU_MIPS16 = 0x11,
  CPU_MIPS32 = 0x12,
  CPU_MIPS64 = 0x13,
  CPU_MIPSI = 0x14,
  CPU_MIPSII = 0x15,
  CPU_MIPSIII = 0x16,
  CPU_MIPSIV = 0x17,
  CPU_MIPSV = 0x18,
  CPU_M68000 = 0x20,
  CPU_M68010 = 0x21,
  CPU_M68020 = 0x22,
  CPU_M68030 = 0x23,
  CPU_M68040 = 0x24,
  CPU_ALPHA = 0x30,
  CPU_ALPHA_21064 = 0x30,
  CPU_ALPHA_21164 = 0x31,
  CPU_ALPHA_21164A = 0x32,
  CPU_ALPHA_21264 = 0x33,
  CPU_ALPHA_21364 = 0x34,
  CPU_PPC601 = 0x40,
  CPU_PPC603 = 0x41,
  CPU_PPC604 = 0x42,
  CPU_PPC620 = 0x43,
  CPU_PPCFP = 0x44,
  CPU_PPCBE = 0x45,
  CPU_SH3 = 0x50,
  CPU_SH3E = 0x51,
  CPU_SH3DSP = 0x52,
  CPU_SH4 = 0x53,
  CPU_SHMEDIA = 0x54,
  CPU_ARM3 = 0x60,
  CPU_ARM4 = 0x61,
  CPU_ARM4T = 0x62,
  CPU_ARM5 = 0x63,
  CPU_ARM5T = 0x64,
  CPU_ARM6 = 0x65,
  CPU_ARM_XMAC = 0x66,
  CPU_ARM_WMMX = 0x67,
  CPU_ARM7 = 0x68,
  CPU_OMNI = 0x70,
  CPU_IA64 = 0x80,
  CPU_IA64_1 = 0x80,
  CPU_IA64_2 = 0x81,
  CPU_CEE = 0x90,
  CPU_AM33 = 0xA0,
  CPU_M32R = 0xB0,
  CPU_TRICORE = 0xC0,
  CPU_X64 = 0xD0,
  CPU_AMD64 = CPU_X64,
  CPU_EBC = 0xE0,
  CPU_THUMB = 0xF0,
  CPU_ARMNT = 0xF4,
  CPU_ARM64 = 0xF6,
  CPU_D3D11_SHADER = 0x100,
};

// S_COMPILE3
struct CompileSym3 {
  ulittle32_t flags;
  uint8_t getLanguage() const { return flags & 0xff; }
  enum Flags : uint32_t {
    EC = 1 << 8,              // compiled for E/C
    NoDbgInfo = 1 << 9,       // not compiled with debug info
    LTCG = 1 << 10,           // compiled with LTCG
    NoDataAlign = 1 << 11,    // compiled with -Bzalign
    ManagedPresent = 1 << 12, // managed code/data present
    SecurityChecks = 1 << 13, // compiled with /GS
    HotPatch = 1 << 14,       // compiled with /hotpatch
    CVTCIL = 1 << 15,         // converted with CVTCIL
    MSILModule = 1 << 16,     // MSIL netmodule
    Sdl = 1 << 17,            // compiled with /sdl
    PGO = 1 << 18,            // compiled with /ltcg:pgo or pgu
    Exp = 1 << 19,            // .exp module
  };
  ulittle16_t machine;    // target processor (CV_CPU_TYPE_e)
  ulittle16_t verFEMajor; // front end major version #
  ulittle16_t verFEMinor; // front end minor version #
  ulittle16_t verFEBuild; // front end build version #
  ulittle16_t verFEQFE;   // front end QFE version #
  ulittle16_t verMajor;   // back end major version #
  ulittle16_t verMinor;   // back end minor version #
  ulittle16_t verBuild;   // back end build version #
  ulittle16_t verQFE;     // back end QFE version #
  //char verSz[1];          // Zero terminated compiler version string
};

// S_FRAMEPROC
struct FrameProcSym {
  ulittle32_t cbFrame;    // count of bytes of total frame of procedure
  ulittle32_t cbPad;      // count of bytes of padding in the frame
  ulittle32_t offPad;     // offset (relative to frame poniter) to where
                          //  padding starts
  ulittle32_t cbSaveRegs; // count of bytes of callee save registers
  ulittle32_t offExHdlr;  // offset of exception handler
  ulittle16_t sectExHdlr; // section id of exception handler
  enum Flags : uint32_t {
    HasAlloca = 1 << 0,  // function uses _alloca()
    HasSetJmp = 1 << 1,  // function uses setjmp()
    HasLongJmp = 1 << 2, // function uses longjmp()
    HasInlAsm = 1 << 3,  // function uses inline asm
    HasEH = 1 << 4,      // function has EH states
    InlSpec = 1 << 5,    // function was speced as inline
    HasSEH = 1 << 6,     // function has SEH
    Naked = 1 << 7,      // function is __declspec(naked)
    SecurityChecks =
        1 << 8,       // function has buffer security check introduced by /GS.
    AsyncEH = 1 << 9, // function compiled with /EHa
    GSNoStackOrdering = 1 << 10, // function has /GS buffer checks, but
                                 // stack ordering couldn't be done
    WasInlined = 1 << 11,        // function was inlined within another function
    GSCheck = 1 << 12,           // function is __declspec(strict_gs_check)
    SafeBuffers = 1 << 13,       // function is __declspec(safebuffers)
    // record function's local pointer explicitly.
    encodedLocalBasePointer1 = 1 << 14,
    encodedLocalBasePointer2 = 1 << 15,
    // record function's parameter pointer explicitly.
    encodedParamBasePointer1 = 1 << 16,
    encodedParamBasePointer2 = 1 << 17,
    PogoOn = 1 << 18,      // function was compiled with PGO/PGU
    ValidCounts = 1 << 19, // Do we have valid Pogo counts?
    OptSpeed = 1 << 20,    // Did we optimize for speed?
    GuardCF = 1 << 21,     // function contains CFG checks (and no write checks)
    GuardCFW = 1 << 22, // function contains CFW checks and/or instrumentation
  };
  ulittle32_t flags;
};

// S_UDT | S_COBOLUDT
struct UDTSym {
  ulittle32_t typind; // Type index
  char name[1];       // Length-prefixed name
};

// S_BUILDINFO
struct BuildInfoSym {
  ulittle32_t id;     // CV_ItemId of Build Info.
};

// S_BPREL32
struct BPRelativeSym {
  ulittle32_t off;    // BP-relative offset
  ulittle32_t typind; // Type index or Metadata token
  //char name[1];       // Length-prefixed name
};

// S_REGREL32
struct RegRelativeSym {
  ulittle32_t off;    // BP-relative offset
  ulittle32_t typind; // Type index or Metadata token
  ulittle16_t reg;    // register index for symbol
  //char name[1];       // Length-prefixed name
};

struct FrameData {
  ulittle32_t ulRvaStart;
  ulittle32_t cbBlock;
  ulittle32_t cbLocals;
  ulittle32_t cbParams;
  ulittle32_t cbStkMax;
  ulittle32_t frameFunc;
  ulittle16_t cbProlog;
  ulittle16_t cbSavedRegs;
  ulittle32_t flags;
  enum Flags : uint32_t {
    HasSEH = 1 << 0,
    HasEH = 1 << 1,
    IsFunctionStart = 1 << 2,
  };
};

//===----------------------------------------------------------------------===//
// On-disk representation of type information

/// Indicates the kind of TypeRecord we're dealing with here. The documentation
/// and headers talk about this as the "leaf" type.
enum LeafType : uint16_t {
#define LEAF_TYPE(name, val) name = val,
#include "CVLeafTypes.def"
};

/// Builtin type indexes.
enum BuiltinType : unsigned {
#define BUILTIN_TYPE(name, val) name = val,
#include "CVBuiltinTypes.def"
};

/// Equvalent to CV_prop_t in cvinfo.h.
enum ClassOptions : uint16_t {
  Packed = 0x0001,
  HasConstructorOrDestructor = 0x0002,
  HasOverloadedOperator = 0x0004,
  Nested = 0x0008,
  ContainsNestedClass = 0x0010,
  HasOverloadedAssignmentOperator = 0x0020,
  HasConversionOperator = 0x0040,
  ForwardReference = 0x0080,
  Scoped = 0x0100,
  HasUniqueName = 0x0200,
  Sealed = 0x0400,
  // Two bits for HFA classification.
  Intrinsic = 0x2000,
  // Two bits for MOCOM classification.
};

// A CodeView type stream is a sequence of TypeRecords. Records larger than
// 65536 must chain on to a second record. Each TypeRecord is followed by one of
// the leaf types described below.
struct TypeRecord {
  ulittle16_t Len;  // Type record length, starting from &Leaf.
  ulittle16_t Leaf; // Type record kind (LeafType)
};

// LF_TYPESERVER2
struct TypeServer2 {
  char Signature[16];  // GUID
  ulittle32_t Age;
  // Name: Name of the PDB as a null-terminated string
};

// LF_STRING_ID
struct StringId {
  TypeIndex id;
};

// LF_CLASS, LF_STRUCT, LF_INTERFACE
struct ClassType {
  ulittle16_t MemberCount; // Number of members in FieldList.
  ulittle16_t Properties;  // ClassOptions bitset
  TypeIndex FieldList;     // LF_FIELDLIST: List of all kinds of members
  TypeIndex DerivedFrom;   // LF_DERIVED: List of known derived classes
  TypeIndex VShape;        // LF_VTSHAPE: Shape of the vftable
  // SizeOf: The 'sizeof' the UDT in bytes is encoded as an LF_NUMERIC integer.
  // Name: The null-terminated name follows.
};

// LF_POINTER
struct PointerType {
  TypeIndex PointeeType;
  ulittle32_t Attrs; // pointer attributes
  // if pointer to member:
  //   PointerToMemberTail

  enum PointerKind : uint8_t {
    Near16 = 0x00,                // 16 bit pointer
    Far16 = 0x01,                 // 16:16 far pointer
    Huge16 = 0x02,                // 16:16 huge pointer
    BasedOnSegment = 0x03,        // based on segment
    BasedOnValue = 0x04,          // based on value of base
    BasedOnSegmentValue = 0x05,   // based on segment value of base
    BasedOnAddress = 0x06,        // based on address of base
    BasedOnSegmentAddress = 0x07, // based on segment address of base
    BasedOnType = 0x08,           // based on type
    BasedOnSelf = 0x09,           // based on self
    Near32 = 0x0a,                // 32 bit pointer
    Far32 = 0x0b,                 // 16:32 pointer
    Near64 = 0x0c                 // 64 bit pointer
  };

  enum PointerMode : uint8_t {
    Pointer = 0x00,                 // "normal" pointer
    LValueReference = 0x01,         // "old" reference
    PointerToDataMember = 0x02,     // pointer to data member
    PointerToMemberFunction = 0x03, // pointer to member function
    RValueReference = 0x04          // r-value reference
  };

  PointerKind getPtrKind() const { return PointerKind(Attrs & 0x1f); }
  PointerMode getPtrMode() const { return PointerMode((Attrs >> 5) & 0x07); }
  bool isFlat() const { return Attrs & (1 << 8); }
  bool isVolatile() const { return Attrs & (1 << 9); }
  bool isConst() const { return Attrs & (1 << 10); }
  bool isUnaligned() const { return Attrs & (1 << 11); }

  bool isPointerToDataMember() const {
    return getPtrMode() == PointerToDataMember;
  }
  bool isPointerToMemberFunction() const {
    return getPtrMode() == PointerToMemberFunction;
  }
  bool isPointerToMember() const {
    return isPointerToMemberFunction() || isPointerToDataMember();
  }
};

struct PointerToMemberTail {
  TypeIndex ClassType;
  ulittle16_t Representation;

  /// Equivalent to CV_pmtype_e.
  enum PointerToMemberRepresentation : uint16_t {
    Unknown = 0x00,                     // not specified (pre VC8)
    SingleInheritanceData = 0x01,       // member data, single inheritance
    MultipleInheritanceData = 0x02,     // member data, multiple inheritance
    VirtualInheritanceData = 0x03,      // member data, virtual inheritance
    GeneralData = 0x04,                 // member data, most general
    SingleInheritanceFunction = 0x05,   // member function, single inheritance
    MultipleInheritanceFunction = 0x06, // member function, multiple inheritance
    VirtualInheritanceFunction = 0x07,  // member function, virtual inheritance
    GeneralFunction = 0x08              // member function, most general
  };
};

/// In Clang parlance, these are "qualifiers".  LF_MODIFIER
struct TypeModifier {
  TypeIndex ModifiedType;
  ulittle16_t Modifiers;

  /// Equivalent to CV_modifier_t.
  enum QualFlags : uint16_t {
    Const       = (1 << 0),
    Volatile    = (1 << 1),
    Unaligned   = (1 << 2),
  };
};

// LF_VTSHAPE
struct VTableShape {
  // Number of vftable entries. Each method may have more than one entry due to
  // things like covariant return types.
  ulittle16_t VFEntryCount;
  // Descriptors[]: 4-bit virtual method descriptors of type CV_VTS_desc_e.
};

// LF_UDT_SRC_LINE
struct UDTSrcLine {
  TypeIndex UDT;        // The user-defined type
  TypeIndex SourceFile; // StringID containing the source filename
  ulittle32_t LineNumber;
};

// LF_ARGLIST, LF_SUBSTR_LIST
struct ArgList {
  ulittle32_t NumArgs; // Number of arguments
  // ArgTypes[]: Type indicies of arguments
};

// LF_BUILDINFO
struct BuildInfo {
  ulittle16_t NumArgs; // Number of arguments
  // ArgTypes[]: Type indicies of arguments
};

// LF_ENUM
struct EnumType {
  ulittle16_t NumEnumerators; // Number of enumerators
  ulittle16_t Properties;
  TypeIndex UnderlyingType;
  TypeIndex FieldListType;
  // Name: The null-terminated name follows.
};

// LF_ARRAY
struct ArrayType {
  TypeIndex ElementType;
  TypeIndex IndexType;
  // SizeOf: LF_NUMERIC encoded size in bytes. Not element count!
  // Name: The null-terminated name follows.
};

// LF_VFTABLE
struct VFTableType {
  TypeIndex CompleteClass;     // Class that owns this vftable.
  TypeIndex OverriddenVFTable; // VFTable that this overrides.
  ulittle32_t VFPtrOffset;     // VFPtr offset in CompleteClass
  ulittle32_t NamesLen;        // Length of subsequent names array in bytes.
  // Names: A sequence of null-terminated strings. First string is vftable
  // names.
};

// LF_MFUNC_ID
struct MemberFuncId {
  TypeIndex ClassType;
  TypeIndex FunctionType;
  // Name: The null-terminated name follows.
};

enum CallingConvention : uint8_t {
  NearC = 0x00,       // near right to left push, caller pops stack
  FarC = 0x01,        // far right to left push, caller pops stack
  NearPascal = 0x02,  // near left to right push, callee pops stack
  FarPascal = 0x03,   // far left to right push, callee pops stack
  NearFast = 0x04,    // near left to right push with regs, callee pops stack
  FarFast = 0x05,     // far left to right push with regs, callee pops stack
  NearStdCall = 0x07, // near standard call
  FarStdCall = 0x08,  // far standard call
  NearSysCall = 0x09, // near sys call
  FarSysCall = 0x0a,  // far sys call
  ThisCall = 0x0b,    // this call (this passed in register)
  MipsCall = 0x0c,    // Mips call
  Generic = 0x0d,     // Generic call sequence
  AlphaCall = 0x0e,   // Alpha call
  PpcCall = 0x0f,     // PPC call
  SHCall = 0x10,      // Hitachi SuperH call
  ArmCall = 0x11,     // ARM call
  AM33Call = 0x12,    // AM33 call
  TriCall = 0x13,     // TriCore Call
  SH5Call = 0x14,     // Hitachi SuperH-5 call
  M32RCall = 0x15,    // M32R Call
  ClrCall = 0x16,     // clr call
  Inline =
      0x17, // Marker for routines always inlined and thus lacking a convention
  NearVector = 0x18 // near left to right push with regs, callee pops stack
};

enum FunctionOptions : uint8_t {
  None = 0x00,
  CxxReturnUdt = 0x01,
  Constructor = 0x02,
  ConstructorWithVirtualBases = 0x04
};

// LF_PROCEDURE
struct ProcedureType {
  TypeIndex ReturnType;
  CallingConvention CallConv;
  FunctionOptions Options;
  ulittle16_t NumParameters;
  TypeIndex ArgListType;
};

// LF_MFUNCTION
struct MemberFunctionType {
  TypeIndex ReturnType;
  TypeIndex ClassType;
  TypeIndex ThisType;
  CallingConvention CallConv;
  FunctionOptions Options;
  ulittle16_t NumParameters;
  TypeIndex ArgListType;
  int32_t ThisAdjustment;
};

//===----------------------------------------------------------------------===//
// Field list records, which do not include leafs or sizes

/// Equvalent to CV_fldattr_t in cvinfo.h.
struct MemberAttributes {
  ulittle16_t Attrs;

  enum Flags : uint16_t {
    MA_Access = 0x3,
    MA_MProp = (0x7 << 2),
    MA_Pseudo = (0x1 << 5),      // compiler generated fcn and does not exist
    MA_NoInherit = (0x1 << 6),   // true if class cannot be inherited
    MA_NoConstruct = (0x1 << 7), // true if class cannot be constructed
    MA_CompilerGenerated = (0x1 << 8), // compiler generated fcn and does exist
    MA_Sealed = (0x1 << 9),            // true if method cannot be overridden
  };

  /// Get the flags that are not included in access control or method
  /// properties.
  Flags getFlags() const {
    return Flags(unsigned(Attrs) & ~(MA_Access | MA_MProp));
  }

  /// Possible values of MA_MProp. (CV_methodprop_e)
  enum MethodProperties {
    MP_Vanilla = 0x00,
    MP_Virtual = 0x01,
    MP_Static = 0x02,
    MP_Friend = 0x03,
    MP_IntroVirt = 0x04,
    MP_PureVirt = 0x05,
    MP_PureIntro = 0x06
  };

  /// Indicates if a method is defined with friend, virtual, static, etc.
  MethodProperties getMethodProperties() const {
    return MethodProperties((Attrs & MA_MProp) >> 2);
  }

  /// Is this method virtual.
  bool isVirtual() const {
    auto MP = getMethodProperties();
    return MP != MP_Vanilla && MP != MP_Friend && MP_Static;
  }

  /// Does this member introduce a new virtual method.
  bool isIntroducedVirtual() const {
    auto MP = getMethodProperties();
    return MP == MP_IntroVirt || MP == MP_PureIntro;
  }

  /// Source-level access specifier. (CV_access_e)
  enum AccessSpecifier {
    AS_private = 1,
    AS_protected = 2,
    AS_public = 3
  };

  AccessSpecifier getAccess() const {
    return AccessSpecifier(Attrs & MA_Access);
  }
};

// LF_NESTTYPE
struct NestedType {
  ulittle16_t Pad0; // Should be zero
  TypeIndex Type;   // Type index of nested type
  // Name: Null-terminated string
};

// LF_ONEMETHOD
struct OneMethod {
  MemberAttributes Attrs;
  TypeIndex Type;
  // If is introduced virtual method:
  //   VFTableOffset: int32_t offset in vftable
  // Name: Null-terminated string

  MemberAttributes::MethodProperties getMethodProperties() const {
    return Attrs.getMethodProperties();
  }

  bool isVirtual() const { return Attrs.isVirtual(); }
  bool isIntroducedVirtual() const { return Attrs.isIntroducedVirtual(); }
};

struct MethodListEntry {
  MemberAttributes Attrs;
  ulittle16_t Padding;

  TypeIndex Type;
  // If is introduced virtual method:
  //   VFTableOffset: int32_t offset in vftable

  MemberAttributes::MethodProperties getMethodProperties() const {
    return Attrs.getMethodProperties();
  }

  bool isVirtual() const { return Attrs.isVirtual(); }
  bool isIntroducedVirtual() const { return Attrs.isIntroducedVirtual(); }
};

/// For method overload sets.  LF_METHOD
struct OverloadedMethod {
  ulittle16_t MethodCount; // Size of overload set
  TypeIndex MethList;      // Type index of methods in overload set
  // Name: Null-terminated string
};

// LF_VFUNCTAB
struct VirtualFunctionPointer {
  ulittle16_t Pad0;
  TypeIndex Type;   // Type of vfptr
};

// LF_MEMBER
struct DataMember {
  MemberAttributes Attrs; // Access control attributes, etc
  TypeIndex Type;
  // FieldOffset: LF_NUMERIC encoded byte offset
  // Name: Null-terminated string
};

// LF_ENUMERATE
struct Enumerator {
  MemberAttributes Attrs; // Access control attributes, etc
  // EnumValue: LF_NUMERIC encoded enumerator value
  // Name: Null-terminated string
};

// LF_BCLASS, LF_BINTERFACE
struct BaseClass {
  MemberAttributes Attrs; // Access control attributes, etc
  TypeIndex BaseType;     // Base class type
  // BaseOffset: LF_NUMERIC encoded byte offset of base from derived.
};

// LF_VBCLASS | LV_IVBCLASS
struct VirtualBaseClass {
  MemberAttributes Attrs; // Access control attributes, etc.
  TypeIndex BaseType;     // Base class type
  TypeIndex vbptr;        // Virtual base pointer type
  // VBPtrOffset: Offset of vbptr from vfptr encoded as LF_NUMERIC.
  // VBTableIndex: Index of vbase within vbtable encoded as LF_NUMERIC.
};

} // namespace codeview
} // namespace llvm

#endif // LLVM_READOBJ_CODEVIEW_H
