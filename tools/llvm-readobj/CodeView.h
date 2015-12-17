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

#include "llvm/Support/Compiler.h"
#include "llvm/Support/Endian.h"

namespace llvm {
namespace codeview {

LLVM_PACKED_START

// FIXME: Maybe make this a wrapper struct type.
typedef ulittle32_t TypeIndex;

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

enum SymType : unsigned short {
  S_COMPILE       =  0x0001,  // Compile flags symbol
  S_REGISTER_16t  =  0x0002,  // Register variable
  S_CONSTANT_16t  =  0x0003,  // constant symbol
  S_UDT_16t       =  0x0004,  // User defined type
  S_SSEARCH       =  0x0005,  // Start Search
  S_END           =  0x0006,  // Block, procedure, "with" or thunk end
  S_SKIP          =  0x0007,  // Reserve symbol space in $$Symbols table
  S_CVRESERVE     =  0x0008,  // Reserved symbol for CV internal use
  S_OBJNAME_ST    =  0x0009,  // path to object file name
  S_ENDARG        =  0x000a,  // end of argument/return list
  S_COBOLUDT_16t  =  0x000b,  // special UDT for cobol that does not symbol pack
  S_MANYREG_16t   =  0x000c,  // multiple register variable
  S_RETURN        =  0x000d,  // return description symbol
  S_ENTRYTHIS     =  0x000e,  // description of this pointer on entry

  S_BPREL16       =  0x0100,  // BP-relative
  S_LDATA16       =  0x0101,  // Module-local symbol
  S_GDATA16       =  0x0102,  // Global data symbol
  S_PUB16         =  0x0103,  // a public symbol
  S_LPROC16       =  0x0104,  // Local procedure start
  S_GPROC16       =  0x0105,  // Global procedure start
  S_THUNK16       =  0x0106,  // Thunk Start
  S_BLOCK16       =  0x0107,  // block start
  S_WITH16        =  0x0108,  // with start
  S_LABEL16       =  0x0109,  // code label
  S_CEXMODEL16    =  0x010a,  // change execution model
  S_VFTABLE16     =  0x010b,  // address of virtual function table
  S_REGREL16      =  0x010c,  // register relative address

  S_BPREL32_16t   =  0x0200,  // BP-relative
  S_LDATA32_16t   =  0x0201,  // Module-local symbol
  S_GDATA32_16t   =  0x0202,  // Global data symbol
  S_PUB32_16t     =  0x0203,  // a public symbol (CV internal reserved)
  S_LPROC32_16t   =  0x0204,  // Local procedure start
  S_GPROC32_16t   =  0x0205,  // Global procedure start
  S_THUNK32_ST    =  0x0206,  // Thunk Start
  S_BLOCK32_ST    =  0x0207,  // block start
  S_WITH32_ST     =  0x0208,  // with start
  S_LABEL32_ST    =  0x0209,  // code label
  S_CEXMODEL32    =  0x020a,  // change execution model
  S_VFTABLE32_16t =  0x020b,  // address of virtual function table
  S_REGREL32_16t  =  0x020c,  // register relative address
  S_LTHREAD32_16t =  0x020d,  // local thread storage
  S_GTHREAD32_16t =  0x020e,  // global thread storage
  S_SLINK32       =  0x020f,  // static link for MIPS EH implementation

  S_LPROCMIPS_16t =  0x0300,  // Local procedure start
  S_GPROCMIPS_16t =  0x0301,  // Global procedure start

  // if these ref symbols have names following then the names are in ST format
  S_PROCREF_ST    =  0x0400,  // Reference to a procedure
  S_DATAREF_ST    =  0x0401,  // Reference to data
  S_ALIGN         =  0x0402,  // Used for page alignment of symbols

  S_LPROCREF_ST   =  0x0403,  // Local Reference to a procedure
  S_OEM           =  0x0404,  // OEM defined symbol

  // sym records with 32-bit types embedded instead of 16-bit
  // all have 0x1000 bit set for easy identification
  // only do the 32-bit target versions since we don't really
  // care about 16-bit ones anymore.
  S_TI16_MAX          =  0x1000,

  S_REGISTER_ST   =  0x1001,  // Register variable
  S_CONSTANT_ST   =  0x1002,  // constant symbol
  S_UDT_ST        =  0x1003,  // User defined type
  S_COBOLUDT_ST   =  0x1004,  // special UDT for cobol that does not symbol pack
  S_MANYREG_ST    =  0x1005,  // multiple register variable
  S_BPREL32_ST    =  0x1006,  // BP-relative
  S_LDATA32_ST    =  0x1007,  // Module-local symbol
  S_GDATA32_ST    =  0x1008,  // Global data symbol
  S_PUB32_ST      =  0x1009,  // a public symbol (CV internal reserved)
  S_LPROC32_ST    =  0x100a,  // Local procedure start
  S_GPROC32_ST    =  0x100b,  // Global procedure start
  S_VFTABLE32     =  0x100c,  // address of virtual function table
  S_REGREL32_ST   =  0x100d,  // register relative address
  S_LTHREAD32_ST  =  0x100e,  // local thread storage
  S_GTHREAD32_ST  =  0x100f,  // global thread storage

  S_LPROCMIPS_ST  =  0x1010,  // Local procedure start
  S_GPROCMIPS_ST  =  0x1011,  // Global procedure start

  S_FRAMEPROC     =  0x1012,  // extra frame and proc information
  S_COMPILE2_ST   =  0x1013,  // extended compile flags and info

  // new symbols necessary for 16-bit enumerates of IA64 registers
  // and IA64 specific symbols

  S_MANYREG2_ST   =  0x1014,  // multiple register variable
  S_LPROCIA64_ST  =  0x1015,  // Local procedure start (IA64)
  S_GPROCIA64_ST  =  0x1016,  // Global procedure start (IA64)

  // Local symbols for IL
  S_LOCALSLOT_ST  =  0x1017,  // local IL sym with field for local slot index
  S_PARAMSLOT_ST  =  0x1018,  // local IL sym with field for parameter slot index

  S_ANNOTATION    =  0x1019,  // Annotation string literals

  // symbols to support managed code debugging
  S_GMANPROC_ST   =  0x101a,  // Global proc
  S_LMANPROC_ST   =  0x101b,  // Local proc
  S_RESERVED1     =  0x101c,  // reserved
  S_RESERVED2     =  0x101d,  // reserved
  S_RESERVED3     =  0x101e,  // reserved
  S_RESERVED4     =  0x101f,  // reserved
  S_LMANDATA_ST   =  0x1020,
  S_GMANDATA_ST   =  0x1021,
  S_MANFRAMEREL_ST=  0x1022,
  S_MANREGISTER_ST=  0x1023,
  S_MANSLOT_ST    =  0x1024,
  S_MANMANYREG_ST =  0x1025,
  S_MANREGREL_ST  =  0x1026,
  S_MANMANYREG2_ST=  0x1027,
  S_MANTYPREF     =  0x1028,  // Index for type referenced by name from metadata
  S_UNAMESPACE_ST =  0x1029,  // Using namespace

  // Symbols w/ SZ name fields. All name fields contain utf8 encoded strings.
  S_ST_MAX        =  0x1100,  // starting point for SZ name symbols

  S_OBJNAME       =  0x1101,  // path to object file name
  S_THUNK32       =  0x1102,  // Thunk Start
  S_BLOCK32       =  0x1103,  // block start
  S_WITH32        =  0x1104,  // with start
  S_LABEL32       =  0x1105,  // code label
  S_REGISTER      =  0x1106,  // Register variable
  S_CONSTANT      =  0x1107,  // constant symbol
  S_UDT           =  0x1108,  // User defined type
  S_COBOLUDT      =  0x1109,  // special UDT for cobol that does not symbol pack
  S_MANYREG       =  0x110a,  // multiple register variable
  S_BPREL32       =  0x110b,  // BP-relative
  S_LDATA32       =  0x110c,  // Module-local symbol
  S_GDATA32       =  0x110d,  // Global data symbol
  S_PUB32         =  0x110e,  // a public symbol (CV internal reserved)
  S_LPROC32       =  0x110f,  // Local procedure start
  S_GPROC32       =  0x1110,  // Global procedure start
  S_REGREL32      =  0x1111,  // register relative address
  S_LTHREAD32     =  0x1112,  // local thread storage
  S_GTHREAD32     =  0x1113,  // global thread storage

  S_LPROCMIPS     =  0x1114,  // Local procedure start
  S_GPROCMIPS     =  0x1115,  // Global procedure start
  S_COMPILE2      =  0x1116,  // extended compile flags and info
  S_MANYREG2      =  0x1117,  // multiple register variable
  S_LPROCIA64     =  0x1118,  // Local procedure start (IA64)
  S_GPROCIA64     =  0x1119,  // Global procedure start (IA64)
  S_LOCALSLOT     =  0x111a,  // local IL sym with field for local slot index
  S_SLOT          = S_LOCALSLOT,  // alias for LOCALSLOT
  S_PARAMSLOT     =  0x111b,  // local IL sym with field for parameter slot index

  // symbols to support managed code debugging
  S_LMANDATA      =  0x111c,
  S_GMANDATA      =  0x111d,
  S_MANFRAMEREL   =  0x111e,
  S_MANREGISTER   =  0x111f,
  S_MANSLOT       =  0x1120,
  S_MANMANYREG    =  0x1121,
  S_MANREGREL     =  0x1122,
  S_MANMANYREG2   =  0x1123,
  S_UNAMESPACE    =  0x1124,  // Using namespace

  // ref symbols with name fields
  S_PROCREF       =  0x1125,  // Reference to a procedure
  S_DATAREF       =  0x1126,  // Reference to data
  S_LPROCREF      =  0x1127,  // Local Reference to a procedure
  S_ANNOTATIONREF =  0x1128,  // Reference to an S_ANNOTATION symbol
  S_TOKENREF      =  0x1129,  // Reference to one of the many MANPROCSYM's

  // continuation of managed symbols
  S_GMANPROC      =  0x112a,  // Global proc
  S_LMANPROC      =  0x112b,  // Local proc

  // short, light-weight thunks
  S_TRAMPOLINE    =  0x112c,  // trampoline thunks
  S_MANCONSTANT   =  0x112d,  // constants with metadata type info

  // native attributed local/parms
  S_ATTR_FRAMEREL =  0x112e,  // relative to virtual frame ptr
  S_ATTR_REGISTER =  0x112f,  // stored in a register
  S_ATTR_REGREL   =  0x1130,  // relative to register (alternate frame ptr)
  S_ATTR_MANYREG  =  0x1131,  // stored in >1 register

  // Separated code (from the compiler) support
  S_SEPCODE       =  0x1132,

  S_LOCAL_2005    =  0x1133,  // defines a local symbol in optimized code
  S_DEFRANGE_2005 =  0x1134,  // defines a single range of addresses in which symbol can be evaluated
  S_DEFRANGE2_2005 =  0x1135,  // defines ranges of addresses in which symbol can be evaluated

  S_SECTION       =  0x1136,  // A COFF section in a PE executable
  S_COFFGROUP     =  0x1137,  // A COFF group
  S_EXPORT        =  0x1138,  // A export

  S_CALLSITEINFO  =  0x1139,  // Indirect call site information
  S_FRAMECOOKIE   =  0x113a,  // Security cookie information

  S_DISCARDED     =  0x113b,  // Discarded by LINK /OPT:REF (experimental, see richards)

  S_COMPILE3      =  0x113c,  // Replacement for S_COMPILE2
  S_ENVBLOCK      =  0x113d,  // Environment block split off from S_COMPILE2

  S_LOCAL         =  0x113e,  // defines a local symbol in optimized code
  S_DEFRANGE      =  0x113f,  // defines a single range of addresses in which symbol can be evaluated
  S_DEFRANGE_SUBFIELD =  0x1140,           // ranges for a subfield

  S_DEFRANGE_REGISTER =  0x1141,           // ranges for en-registered symbol
  S_DEFRANGE_FRAMEPOINTER_REL =  0x1142,   // range for stack symbol.
  S_DEFRANGE_SUBFIELD_REGISTER =  0x1143,  // ranges for en-registered field of symbol
  S_DEFRANGE_FRAMEPOINTER_REL_FULL_SCOPE =  0x1144, // range for stack symbol span valid full scope of function body, gap might apply.
  S_DEFRANGE_REGISTER_REL =  0x1145, // range for symbol address as register + offset.

  // S_PROC symbols that reference ID instead of type
  S_LPROC32_ID     =  0x1146,
  S_GPROC32_ID     =  0x1147,
  S_LPROCMIPS_ID   =  0x1148,
  S_GPROCMIPS_ID   =  0x1149,
  S_LPROCIA64_ID   =  0x114a,
  S_GPROCIA64_ID   =  0x114b,

  S_BUILDINFO      = 0x114c, // build information.
  S_INLINESITE     = 0x114d, // inlined function callsite.
  S_INLINESITE_END = 0x114e,
  S_PROC_ID_END    = 0x114f,

  S_DEFRANGE_HLSL  = 0x1150,
  S_GDATA_HLSL     = 0x1151,
  S_LDATA_HLSL     = 0x1152,

  S_FILESTATIC     = 0x1153,

  S_LOCAL_DPC_GROUPSHARED = 0x1154, // DPC groupshared variable
  S_LPROC32_DPC = 0x1155, // DPC local procedure start
  S_LPROC32_DPC_ID =  0x1156,
  S_DEFRANGE_DPC_PTR_TAG =  0x1157, // DPC pointer tag definition range
  S_DPC_SYM_TAG_MAP = 0x1158, // DPC pointer tag value to symbol record map

  S_HEAPALLOCSITE = 0x115e,    // heap allocation site

  S_MOD_TYPEREF = 0x115f,      // only generated at link time

  S_REF_MINIPDB = 0x1160,      // only generated at link time for mini PDB
  S_PDBMAP      = 0x1161,      // only generated at link time for mini PDB

  S_GDATA_HLSL32 = 0x1162,
  S_LDATA_HLSL32 = 0x1163,

  S_GDATA_HLSL32_EX = 0x1164,
  S_LDATA_HLSL32_EX = 0x1165,

  S_RECTYPE_MAX,               // one greater than last
  S_RECTYPE_LAST  = S_RECTYPE_MAX - 1,
  S_RECTYPE_PAD   = S_RECTYPE_MAX + 0x100 // Used *only* to verify symbol record types so that current PDB code can potentially read
                              // future PDBs (assuming no format change, etc).

};

/// Generic record compatible with all symbol records.
///
/// TODO: Extend this with LLVM-style RTTI.
struct SymRecord {
  ulittle16_t reclen; // Record length, starting from the next field
  ulittle16_t rectyp; // Record type
  // char data[];
};

struct ProcSym {
  ulittle16_t reclen;   // Record length
  ulittle16_t rectyp;   // S_GPROC32, S_LPROC32, S_GPROC32_ID, S_LPROC32_ID,
                        // S_LPROC32_DPC or S_LPROC32_DPC_ID
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
  char name[1];  // Length-prefixed name
};

struct ObjNameSym {
  ulittle16_t reclen;    // Record length
  ulittle16_t rectyp;    // S_OBJNAME
  ulittle32_t signature; // signature
  char name[1];          // Length-prefixed name
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

struct CompileSym3 {
  ulittle16_t reclen; // Record length
  ulittle16_t rectyp; // S_COMPILE3
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
  char verSz[1];          // Zero terminated compiler version string
};

struct FrameProcSym {
  ulittle16_t reclen;     // Record length
  ulittle16_t rectyp;     // S_FRAMEPROC
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

struct UDTSym {
  ulittle16_t reclen; // Record length
  ulittle16_t rectyp; // S_UDT | S_COBOLUDT
  ulittle32_t typind; // Type index
  char name[1];       // Length-prefixed name
};

struct BuildInfoSym {
  ulittle16_t reclen; // Record length
  ulittle16_t rectyp; // S_BUILDINFO
  ulittle32_t id;     // CV_ItemId of Build Info.
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

struct BPRelativeSym {
  ulittle16_t reclen; // Record length
  ulittle16_t rectyp; // S_BPREL32
  ulittle32_t off;    // BP-relative offset
  ulittle32_t typind; // Type index or Metadata token
  char name[1];       // Length-prefixed name
};

struct RegRelativeSym {
  ulittle16_t reclen; // Record length
  ulittle16_t rectyp; // S_BPREL32
  ulittle32_t off;    // BP-relative offset
  ulittle32_t typind; // Type index or Metadata token
  ulittle16_t reg;    // register index for symbol
  char name[1];       // Length-prefixed name
};

//===----------------------------------------------------------------------===//
// On-disk representation of type information

/// Indicates the kind of TypeRecord we're dealing with here. The documentation
/// and headers talk about this as the "leaf" type.
enum LeafType : uint16_t {
  LF_MODIFIER_16t = 0x0001,
  LF_POINTER_16t = 0x0002,
  LF_ARRAY_16t = 0x0003,
  LF_CLASS_16t = 0x0004,
  LF_STRUCTURE_16t = 0x0005,
  LF_UNION_16t = 0x0006,
  LF_ENUM_16t = 0x0007,
  LF_PROCEDURE_16t = 0x0008,
  LF_MFUNCTION_16t = 0x0009,
  LF_VTSHAPE = 0x000a,
  LF_COBOL0_16t = 0x000b,
  LF_COBOL1 = 0x000c,
  LF_BARRAY_16t = 0x000d,
  LF_LABEL = 0x000e,
  LF_NULL = 0x000f,
  LF_NOTTRAN = 0x0010,
  LF_DIMARRAY_16t = 0x0011,
  LF_VFTPATH_16t = 0x0012,
  LF_PRECOMP_16t = 0x0013,   // not referenced from symbol
  LF_ENDPRECOMP = 0x0014,    // not referenced from symbol
  LF_OEM_16t = 0x0015,       // oem definable type string
  LF_TYPESERVER_ST = 0x0016, // not referenced from symbol

  // leaf indices starting records but referenced only from type records

  LF_SKIP_16t = 0x0200,
  LF_ARGLIST_16t = 0x0201,
  LF_DEFARG_16t = 0x0202,
  LF_LIST = 0x0203,
  LF_FIELDLIST_16t = 0x0204,
  LF_DERIVED_16t = 0x0205,
  LF_BITFIELD_16t = 0x0206,
  LF_METHODLIST_16t = 0x0207,
  LF_DIMCONU_16t = 0x0208,
  LF_DIMCONLU_16t = 0x0209,
  LF_DIMVARU_16t = 0x020a,
  LF_DIMVARLU_16t = 0x020b,
  LF_REFSYM = 0x020c,

  LF_BCLASS_16t = 0x0400,
  LF_VBCLASS_16t = 0x0401,
  LF_IVBCLASS_16t = 0x0402,
  LF_ENUMERATE_ST = 0x0403,
  LF_FRIENDFCN_16t = 0x0404,
  LF_INDEX_16t = 0x0405,
  LF_MEMBER_16t = 0x0406,
  LF_STMEMBER_16t = 0x0407,
  LF_METHOD_16t = 0x0408,
  LF_NESTTYPE_16t = 0x0409,
  LF_VFUNCTAB_16t = 0x040a,
  LF_FRIENDCLS_16t = 0x040b,
  LF_ONEMETHOD_16t = 0x040c,
  LF_VFUNCOFF_16t = 0x040d,

  // 32-bit type index versions of leaves, all have the 0x1000 bit set
  //
  LF_TI16_MAX = 0x1000,

  LF_MODIFIER = 0x1001,
  LF_POINTER = 0x1002,
  LF_ARRAY_ST = 0x1003,
  LF_CLASS_ST = 0x1004,
  LF_STRUCTURE_ST = 0x1005,
  LF_UNION_ST = 0x1006,
  LF_ENUM_ST = 0x1007,
  LF_PROCEDURE = 0x1008,
  LF_MFUNCTION = 0x1009,
  LF_COBOL0 = 0x100a,
  LF_BARRAY = 0x100b,
  LF_DIMARRAY_ST = 0x100c,
  LF_VFTPATH = 0x100d,
  LF_PRECOMP_ST = 0x100e, // not referenced from symbol
  LF_OEM = 0x100f,        // oem definable type string
  LF_ALIAS_ST = 0x1010,   // alias (typedef) type
  LF_OEM2 = 0x1011,       // oem definable type string

  // leaf indices starting records but referenced only from type records

  LF_SKIP = 0x1200,
  LF_ARGLIST = 0x1201,
  LF_DEFARG_ST = 0x1202,
  LF_FIELDLIST = 0x1203,
  LF_DERIVED = 0x1204,
  LF_BITFIELD = 0x1205,
  LF_METHODLIST = 0x1206,
  LF_DIMCONU = 0x1207,
  LF_DIMCONLU = 0x1208,
  LF_DIMVARU = 0x1209,
  LF_DIMVARLU = 0x120a,

  LF_BCLASS = 0x1400,
  LF_VBCLASS = 0x1401,
  LF_IVBCLASS = 0x1402,
  LF_FRIENDFCN_ST = 0x1403,
  LF_INDEX = 0x1404,
  LF_MEMBER_ST = 0x1405,
  LF_STMEMBER_ST = 0x1406,
  LF_METHOD_ST = 0x1407,
  LF_NESTTYPE_ST = 0x1408,
  LF_VFUNCTAB = 0x1409,
  LF_FRIENDCLS = 0x140a,
  LF_ONEMETHOD_ST = 0x140b,
  LF_VFUNCOFF = 0x140c,
  LF_NESTTYPEEX_ST = 0x140d,
  LF_MEMBERMODIFY_ST = 0x140e,
  LF_MANAGED_ST = 0x140f,

  // Types w/ SZ names

  LF_ST_MAX = 0x1500,

  LF_TYPESERVER = 0x1501, // not referenced from symbol
  LF_ENUMERATE = 0x1502,
  LF_ARRAY = 0x1503,
  LF_CLASS = 0x1504,
  LF_STRUCTURE = 0x1505,
  LF_UNION = 0x1506,
  LF_ENUM = 0x1507,
  LF_DIMARRAY = 0x1508,
  LF_PRECOMP = 0x1509, // not referenced from symbol
  LF_ALIAS = 0x150a,   // alias (typedef) type
  LF_DEFARG = 0x150b,
  LF_FRIENDFCN = 0x150c,
  LF_MEMBER = 0x150d,
  LF_STMEMBER = 0x150e,
  LF_METHOD = 0x150f,
  LF_NESTTYPE = 0x1510,
  LF_ONEMETHOD = 0x1511,
  LF_NESTTYPEEX = 0x1512,
  LF_MEMBERMODIFY = 0x1513,
  LF_MANAGED = 0x1514,
  LF_TYPESERVER2 = 0x1515,

  LF_STRIDED_ARRAY =
      0x1516, // same as LF_ARRAY, but with stride between adjacent elements
  LF_HLSL = 0x1517,
  LF_MODIFIER_EX = 0x1518,
  LF_INTERFACE = 0x1519,
  LF_BINTERFACE = 0x151a,
  LF_VECTOR = 0x151b,
  LF_MATRIX = 0x151c,

  LF_VFTABLE = 0x151d, // a virtual function table
  LF_ENDOFLEAFRECORD = LF_VFTABLE,

  LF_TYPE_LAST, // one greater than the last type record
  LF_TYPE_MAX = LF_TYPE_LAST - 1,

  LF_FUNC_ID = 0x1601,  // global func ID
  LF_MFUNC_ID = 0x1602, // member func ID
  LF_BUILDINFO =
      0x1603, // build info: tool, version, command line, src/pdb file
  LF_SUBSTR_LIST = 0x1604, // similar to LF_ARGLIST, for list of sub strings
  LF_STRING_ID = 0x1605,   // string ID

  LF_UDT_SRC_LINE = 0x1606, // source and line on where an UDT is defined
                            // only generated by compiler

  LF_UDT_MOD_SRC_LINE =
      0x1607, // module, source and line on where an UDT is defined
              // only generated by linker

  LF_ID_LAST, // one greater than the last ID record
  LF_ID_MAX = LF_ID_LAST - 1,

  LF_NUMERIC = 0x8000,
  LF_CHAR = 0x8000,
  LF_SHORT = 0x8001,
  LF_USHORT = 0x8002,
  LF_LONG = 0x8003,
  LF_ULONG = 0x8004,
  LF_REAL32 = 0x8005,
  LF_REAL64 = 0x8006,
  LF_REAL80 = 0x8007,
  LF_REAL128 = 0x8008,
  LF_QUADWORD = 0x8009,
  LF_UQUADWORD = 0x800a,
  LF_REAL48 = 0x800b,
  LF_COMPLEX32 = 0x800c,
  LF_COMPLEX64 = 0x800d,
  LF_COMPLEX80 = 0x800e,
  LF_COMPLEX128 = 0x800f,
  LF_VARSTRING = 0x8010,

  LF_OCTWORD = 0x8017,
  LF_UOCTWORD = 0x8018,

  LF_DECIMAL = 0x8019,
  LF_DATE = 0x801a,
  LF_UTF8STRING = 0x801b,

  LF_REAL16 = 0x801c,

  LF_PAD0 = 0xf0,
  LF_PAD1 = 0xf1,
  LF_PAD2 = 0xf2,
  LF_PAD3 = 0xf3,
  LF_PAD4 = 0xf4,
  LF_PAD5 = 0xf5,
  LF_PAD6 = 0xf6,
  LF_PAD7 = 0xf7,
  LF_PAD8 = 0xf8,
  LF_PAD9 = 0xf9,
  LF_PAD10 = 0xfa,
  LF_PAD11 = 0xfb,
  LF_PAD12 = 0xfc,
  LF_PAD13 = 0xfd,
  LF_PAD14 = 0xfe,
  LF_PAD15 = 0xff,
};

/// Builtin type indexes.
enum BuiltinTypes : unsigned {
#define BUILTIN_TYPE(name, val) name = val,
#include "CVBuiltinTypes.def"
#undef BUILTIN_TYPE
};

// CV_prop_t
enum TagProperties : uint16_t {
  packed = (1 << 0),   // true if structure is packed
  ctor = (1 << 1),     // true if constructors or destructors present
  ovlops = (1 << 2),   // true if overloaded operators present
  isnested = (1 << 3), // true if this is a nested class
  cnested = (1 << 4),  // true if this class contains nested types
  opassign = (1 << 5), // true if overloaded assignment (=)
  opcast = (1 << 6),   // true if casting methods
  fwdref = (1 << 7),   // true if forward reference (incomplete defn)
  scoped = (1 << 8),   // scoped definition
  hasuniquename = (1 << 9), // true if there is a decorated name following the regular name
  sealed = (1 << 10),    // true if class cannot be used as a base class
  hfa0 = (1 << 11),      // CV_HFA_e
  hfa1 = (1 << 12),      // CV_HFA_e
  intrinsic = (1 << 13), // true if class is an intrinsic type (e.g. __m128d)
  mocom0 = (1 << 14),    // CV_MOCOM_UDT_e
  mocom1 = (1 << 15),    // CV_MOCOM_UDT_e
};

/// (CV_fldattr_t)
enum MemberAttributes : uint16_t {
  MA_Access = 0x3,             // access protection CV_access_t
  MA_MProp = (0x7 << 2),       // method properties CV_methodprop_t
  MA_Pseudo = (0x1 << 5),      // compiler generated fcn and does not exist
  MA_NoInherit = (0x1 << 6),   // true if class cannot be inherited
  MA_NoConstruct = (0x1 << 7), // true if class cannot be constructed
  MA_CompilerGenerated = (0x1 << 8), // compiler generated fcn and does exist
  MA_Sealed = (0x1 << 9),            // true if method cannot be overridden
  MA_Unused = (0x3f << 10),          // unused
};

/// Possible values of MA_MProp. (CV_methodprop_t)
enum MethodProperties {
  MP_Vanilla = 0x00,
  MP_Virtual = 0x01,
  MP_Static = 0x02,
  MP_Friend = 0x03,
  MP_IntroVirt = 0x04,
  MP_PureVirt = 0x05,
  MP_PureIntro = 0x06
};

inline bool isVirtualMethodProperty(MethodProperties Prop) {
  switch (Prop) {
  case MP_Vanilla:
  case MP_Static:
  case MP_Friend:
    return false;
  case MP_Virtual:
  case MP_IntroVirt:
  case MP_PureVirt:
  case MP_PureIntro:
    return true;
  }
  return false;
}

struct TypeRecord {
  ulittle16_t len;
  ulittle16_t leaf;
};

struct TypeServer2 {
  TypeRecord Base; // LF_TYPESERVER2

  char sig70[16];  // guid signature
  ulittle32_t age; // age of database used by this module
  char name[1];    // length prefixed name of PDB
};

struct StringId {
  TypeRecord Base;

  TypeIndex id;
  char data[1];
};

struct ClassType {
  TypeRecord Base; // LF_CLASS, LF_STRUCT, LF_INTERFACE

  ulittle16_t count;    // count of number of elements in class
  ulittle16_t property; // property attribute field (TagProperties)
  TypeIndex field;      // type index of LF_FIELD descriptor list
  TypeIndex derived;    // type index of derived from list if not zero
  TypeIndex vshape;     // type index of vshape table for this class
  char data[1];         // data describing length of structure in
                        // bytes and name
};

struct FieldList {
  TypeRecord Base; // LF_FIELDLIST

  char data[1];     // field list sub lists
};

struct PointerType {
  TypeRecord Base; // LF_POINTER

  TypeIndex utype; // type index of pointee type
  ulittle32_t attr; // pointer attributes
  // if pointer to member:
  //   TypeIndex pmclass;
  //   ulittle16_t pmenum; // CV_pmtype_e
  // else if CV_PTR_BASE_SEG:
  //   ulittle16_t bseg;
  //   char Sym[1]
  // else if CV_PTR_BASE_TYPE:
  //   CV_typ_t index;      // type index if CV_PTR_BASE_TYPE
  //   char name[1];        // name of base type

  enum CV_ptrtype_e : uint8_t {
    CV_PTR_NEAR         = 0x00, // 16 bit pointer
    CV_PTR_FAR          = 0x01, // 16:16 far pointer
    CV_PTR_HUGE         = 0x02, // 16:16 huge pointer
    CV_PTR_BASE_SEG     = 0x03, // based on segment
    CV_PTR_BASE_VAL     = 0x04, // based on value of base
    CV_PTR_BASE_SEGVAL  = 0x05, // based on segment value of base
    CV_PTR_BASE_ADDR    = 0x06, // based on address of base
    CV_PTR_BASE_SEGADDR = 0x07, // based on segment address of base
    CV_PTR_BASE_TYPE    = 0x08, // based on type
    CV_PTR_BASE_SELF    = 0x09, // based on self
    CV_PTR_NEAR32       = 0x0a, // 32 bit pointer
    CV_PTR_FAR32        = 0x0b, // 16:32 pointer
    CV_PTR_64           = 0x0c, // 64 bit pointer
    CV_PTR_UNUSEDPTR    = 0x0d  // first unused pointer type
  };

  enum CV_ptrmode_e : uint8_t {
    CV_PTR_MODE_PTR     = 0x00, // "normal" pointer
    CV_PTR_MODE_REF     = 0x01, // "old" reference
    CV_PTR_MODE_LVREF   = 0x01, // l-value reference
    CV_PTR_MODE_PMEM    = 0x02, // pointer to data member
    CV_PTR_MODE_PMFUNC  = 0x03, // pointer to member function
    CV_PTR_MODE_RVREF   = 0x04, // r-value reference
    CV_PTR_MODE_RESERVED= 0x05  // first unused pointer mode
  };

  CV_ptrtype_e getPtrType() const { return CV_ptrtype_e(attr & 0x1f); }
  CV_ptrmode_e getPtrMode() const { return CV_ptrmode_e((attr & 0x07) >> 5); }
  bool isFlat()      const { return attr & (1 <<  8); }
  bool isVolatile()  const { return attr & (1 <<  9); }
  bool isConst()     const { return attr & (1 << 10); }
  bool isUnaligned() const { return attr & (1 << 11); }

  bool isPointerToDataMember() const {
    return getPtrMode() == CV_PTR_MODE_PMEM;
  }
  bool isPointerToMemberFunction() const {
    return getPtrMode() == CV_PTR_MODE_PMFUNC;
  }
  bool isPointerToMember() const {
    return isPointerToMemberFunction() || isPointerToDataMember();
  }
};

struct PointerToMemberTail {
  TypeIndex pmclass;
  ulittle16_t pmenum;

  enum CV_pmtype_e {
    CV_PMTYPE_Undef     = 0x00, // not specified (pre VC8)
    CV_PMTYPE_D_Single  = 0x01, // member data, single inheritance
    CV_PMTYPE_D_Multiple= 0x02, // member data, multiple inheritance
    CV_PMTYPE_D_Virtual = 0x03, // member data, virtual inheritance
    CV_PMTYPE_D_General = 0x04, // member data, most general
    CV_PMTYPE_F_Single  = 0x05, // member function, single inheritance
    CV_PMTYPE_F_Multiple= 0x06, // member function, multiple inheritance
    CV_PMTYPE_F_Virtual = 0x07, // member function, virtual inheritance
    CV_PMTYPE_F_General = 0x08, // member function, most general
  };
};

/// In Clang parlance, these are "qualifiers".
struct TypeModifier {
  TypeRecord Base;   // LF_MODIFIER
  TypeIndex type;    // modified type
  ulittle16_t attr;  // modifier attribute (CV_modifier_t)

  enum CV_modifier_t : uint16_t {
    MOD_const       = (1 << 0),
    MOD_volatile    = (1 << 1),
    MOD_unaligned   = (1 << 2),
  };
};

//===----------------------------------------------------------------------===//
// Field list records, which do not include leafs or sizes

struct NestedType {
  // ulittle16_t leaf;   // LF_NESTTYPE
  ulittle16_t pad0;      // internal padding, must be 0
  TypeIndex index;       // index of nested type definition
  // char Name[];        // length prefixed type name
};

struct OneMethod {
  // ulittle16_t leaf; // LF_ONEMETHOD
  ulittle16_t attr;    // method attribute (MemberAttributes)
  TypeIndex index;     // index to type record for procedure
  // offset in vfunctable if intro virtual followed by length prefixed name of
  // method

  MethodProperties getMethodProperties() const {
    return MethodProperties((attr & MA_MProp) >> 2);
  }

  bool isVirtual() const {
    return isVirtualMethodProperty(getMethodProperties());
  }
};

/// For method overload sets.
struct OverloadedMethod {
    // ulittle16_t leaf;            // LF_METHOD
    ulittle16_t count;              // number of occurrences of function
    TypeIndex mList;                // index to LF_METHODLIST record
    // unsigned char   Name[1];     // length prefixed name of method
};

struct VirtualFunctionPointer {
  // ulittle16_t  leaf; // LF_VFUNCTAB
  ulittle16_t pad0; // internal padding, must be 0
  TypeIndex type;   // type index of pointer
};

struct DataMember {
  // ulittle16_t  leaf; // LF_MEMBER
  ulittle16_t attr;     // attribute mask
  TypeIndex index;      // index of type record for field
  // variable length offset of field followed by length prefixed name of field
};

struct Enumerator {
  // ulittle16_t  leaf; // LF_MEMBER
  ulittle16_t attr;     // attribute mask
  // variable length numeric leaf for the enumerator value followed by length
  // prefixed name of the enumerator
};

struct BaseClass {
  // ulittle16_t leaf; // LF_BCLASS, LF_BINTERFACE
  ulittle16_t attr; // attribute
  TypeIndex index;  // type index of base class
  // variable length offset of base within class
  // unsigned char offset[CV_ZEROLEN];
};

struct VirtualBaseClass {
  // ulittle16_t leaf; // LF_VBCLASS | LV_IVBCLASS
  ulittle16_t attr; // attribute
  TypeIndex index;  // type index of direct virtual base class
  TypeIndex vbptr;  // type index of virtual base pointer
  // virtual base pointer offset from address point followed by virtual base
  // offset from vbtable
  // unsigned char vbpoff[CV_ZEROLEN];
};

LLVM_PACKED_END

} // namespace codeview
} // namespace llvm

#endif // LLVM_READOBJ_CODEVIEW_H
