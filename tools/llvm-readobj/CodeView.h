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

// FIXME: Maybe make this a wrapper struct type.
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
struct SymRecord {
  ulittle16_t reclen; // Record length, starting from the next field
  ulittle16_t rectyp; // Record type
  // char data[];
};

struct ProcSym {
  SymRecord Base;       // S_GPROC32, S_LPROC32, S_GPROC32_ID, S_LPROC32_ID,
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
#define LEAF_TYPE(name, val) name = val,
#include "CVLeafTypes.def"
#undef LEAF_TYPE
};

/// Builtin type indexes.
enum BuiltinType : unsigned {
#define BUILTIN_TYPE(name, val) name = val,
#include "CVBuiltinTypes.def"
#undef BUILTIN_TYPE
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
  PointerMode getPtrMode() const { return PointerMode((Attrs & 0x07) >> 5); }
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

// LF_ENUM
struct EnumType {
  ulittle16_t NumEnumerators; // Number of enumerators
  ulittle16_t Properties;
  TypeIndex UnderlyingType;
  TypeIndex FieldListType;
  // Name: The null-terminated name follows.
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
  //   VFTableOffset: LF_NUMERIC encoded byte offset in vftable
  // Name: Null-terminated string

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
