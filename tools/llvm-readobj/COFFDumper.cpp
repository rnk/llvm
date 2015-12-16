//===-- COFFDumper.cpp - COFF-specific dumper -------------------*- C++ -*-===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
///
/// \file
/// \brief This file implements the COFF-specific dumper for llvm-readobj.
///
//===----------------------------------------------------------------------===//

#include "llvm-readobj.h"
#include "ARMWinEHPrinter.h"
#include "CodeView.h"
#include "Error.h"
#include "ObjDumper.h"
#include "StackMapPrinter.h"
#include "StreamWriter.h"
#include "Win64EHDumper.h"
#include "llvm/ADT/DenseMap.h"
#include "llvm/ADT/SmallString.h"
#include "llvm/ADT/StringExtras.h"
#include "llvm/Object/COFF.h"
#include "llvm/Object/ObjectFile.h"
#include "llvm/Support/COFF.h"
#include "llvm/Support/Casting.h"
#include "llvm/Support/Compiler.h"
#include "llvm/Support/DataExtractor.h"
#include "llvm/Support/Format.h"
#include "llvm/Support/SourceMgr.h"
#include "llvm/Support/Win64EH.h"
#include "llvm/Support/raw_ostream.h"
#include <algorithm>
#include <cstring>
#include <system_error>
#include <time.h>

using namespace llvm;
using namespace llvm::object;
using namespace llvm::codeview;
using namespace llvm::Win64EH;

namespace {

class COFFDumper : public ObjDumper {
public:
  COFFDumper(const llvm::object::COFFObjectFile *Obj, StreamWriter& Writer)
    : ObjDumper(Writer)
    , Obj(Obj) {
  }

  void printFileHeaders() override;
  void printSections() override;
  void printRelocations() override;
  void printSymbols() override;
  void printDynamicSymbols() override;
  void printUnwindInfo() override;
  void printCOFFImports() override;
  void printCOFFExports() override;
  void printCOFFDirectives() override;
  void printCOFFBaseReloc() override;
  void printCodeViewDebugInfo() override;
  void printStackMap() const override;
private:
  void printSymbol(const SymbolRef &Sym);
  void printRelocation(const SectionRef &Section, const RelocationRef &Reloc);
  void printDataDirectory(uint32_t Index, const std::string &FieldName);

  void printDOSHeader(const dos_header *DH);
  template <class PEHeader> void printPEHeader(const PEHeader *Hdr);
  void printBaseOfDataField(const pe32_header *Hdr);
  void printBaseOfDataField(const pe32plus_header *Hdr);

  void printCodeViewSymbolSection(StringRef SectionName, const SectionRef &Section);
  void printCodeViewTypeSection(StringRef SectionName, const SectionRef &Section);

  void printCodeViewSymbolsSubsection(StringRef Subsection,
                                      const SectionRef &Section,
                                      uint32_t Offset);

  void cacheRelocations();

  std::error_code resolveSymbol(const coff_section *Section, uint64_t Offset,
                                SymbolRef &Sym);
  std::error_code resolveSymbolName(const coff_section *Section,
                                    uint64_t Offset, StringRef &Name);
  void printImportedSymbols(iterator_range<imported_symbol_iterator> Range);
  void printDelayImportedSymbols(
      const DelayImportDirectoryEntryRef &I,
      iterator_range<imported_symbol_iterator> Range);

  typedef DenseMap<const coff_section*, std::vector<RelocationRef> > RelocMapTy;

  const llvm::object::COFFObjectFile *Obj;
  bool RelocCached = false;
  RelocMapTy RelocMap;
  StringRef CVFileIndexToStringOffsetTable;
  StringRef CVStringTable;
};

} // namespace


namespace llvm {

std::error_code createCOFFDumper(const object::ObjectFile *Obj,
                                 StreamWriter &Writer,
                                 std::unique_ptr<ObjDumper> &Result) {
  const COFFObjectFile *COFFObj = dyn_cast<COFFObjectFile>(Obj);
  if (!COFFObj)
    return readobj_error::unsupported_obj_file_format;

  Result.reset(new COFFDumper(COFFObj, Writer));
  return readobj_error::success;
}

} // namespace llvm

// Given a a section and an offset into this section the function returns the
// symbol used for the relocation at the offset.
std::error_code COFFDumper::resolveSymbol(const coff_section *Section,
                                          uint64_t Offset, SymbolRef &Sym) {
  cacheRelocations();
  const auto &Relocations = RelocMap[Section];
  for (const auto &Relocation : Relocations) {
    uint64_t RelocationOffset = Relocation.getOffset();

    if (RelocationOffset == Offset) {
      Sym = *Relocation.getSymbol();
      return readobj_error::success;
    }
  }
  return readobj_error::unknown_symbol;
}

// Given a section and an offset into this section the function returns the name
// of the symbol used for the relocation at the offset.
std::error_code COFFDumper::resolveSymbolName(const coff_section *Section,
                                              uint64_t Offset,
                                              StringRef &Name) {
  SymbolRef Symbol;
  if (std::error_code EC = resolveSymbol(Section, Offset, Symbol))
    return EC;
  ErrorOr<StringRef> NameOrErr = Symbol.getName();
  if (std::error_code EC = NameOrErr.getError())
    return EC;
  Name = *NameOrErr;
  return std::error_code();
}

static const EnumEntry<COFF::MachineTypes> ImageFileMachineType[] = {
  LLVM_READOBJ_ENUM_ENT(COFF, IMAGE_FILE_MACHINE_UNKNOWN  ),
  LLVM_READOBJ_ENUM_ENT(COFF, IMAGE_FILE_MACHINE_AM33     ),
  LLVM_READOBJ_ENUM_ENT(COFF, IMAGE_FILE_MACHINE_AMD64    ),
  LLVM_READOBJ_ENUM_ENT(COFF, IMAGE_FILE_MACHINE_ARM      ),
  LLVM_READOBJ_ENUM_ENT(COFF, IMAGE_FILE_MACHINE_ARMNT    ),
  LLVM_READOBJ_ENUM_ENT(COFF, IMAGE_FILE_MACHINE_EBC      ),
  LLVM_READOBJ_ENUM_ENT(COFF, IMAGE_FILE_MACHINE_I386     ),
  LLVM_READOBJ_ENUM_ENT(COFF, IMAGE_FILE_MACHINE_IA64     ),
  LLVM_READOBJ_ENUM_ENT(COFF, IMAGE_FILE_MACHINE_M32R     ),
  LLVM_READOBJ_ENUM_ENT(COFF, IMAGE_FILE_MACHINE_MIPS16   ),
  LLVM_READOBJ_ENUM_ENT(COFF, IMAGE_FILE_MACHINE_MIPSFPU  ),
  LLVM_READOBJ_ENUM_ENT(COFF, IMAGE_FILE_MACHINE_MIPSFPU16),
  LLVM_READOBJ_ENUM_ENT(COFF, IMAGE_FILE_MACHINE_POWERPC  ),
  LLVM_READOBJ_ENUM_ENT(COFF, IMAGE_FILE_MACHINE_POWERPCFP),
  LLVM_READOBJ_ENUM_ENT(COFF, IMAGE_FILE_MACHINE_R4000    ),
  LLVM_READOBJ_ENUM_ENT(COFF, IMAGE_FILE_MACHINE_SH3      ),
  LLVM_READOBJ_ENUM_ENT(COFF, IMAGE_FILE_MACHINE_SH3DSP   ),
  LLVM_READOBJ_ENUM_ENT(COFF, IMAGE_FILE_MACHINE_SH4      ),
  LLVM_READOBJ_ENUM_ENT(COFF, IMAGE_FILE_MACHINE_SH5      ),
  LLVM_READOBJ_ENUM_ENT(COFF, IMAGE_FILE_MACHINE_THUMB    ),
  LLVM_READOBJ_ENUM_ENT(COFF, IMAGE_FILE_MACHINE_WCEMIPSV2)
};

static const EnumEntry<COFF::Characteristics> ImageFileCharacteristics[] = {
  LLVM_READOBJ_ENUM_ENT(COFF, IMAGE_FILE_RELOCS_STRIPPED        ),
  LLVM_READOBJ_ENUM_ENT(COFF, IMAGE_FILE_EXECUTABLE_IMAGE       ),
  LLVM_READOBJ_ENUM_ENT(COFF, IMAGE_FILE_LINE_NUMS_STRIPPED     ),
  LLVM_READOBJ_ENUM_ENT(COFF, IMAGE_FILE_LOCAL_SYMS_STRIPPED    ),
  LLVM_READOBJ_ENUM_ENT(COFF, IMAGE_FILE_AGGRESSIVE_WS_TRIM     ),
  LLVM_READOBJ_ENUM_ENT(COFF, IMAGE_FILE_LARGE_ADDRESS_AWARE    ),
  LLVM_READOBJ_ENUM_ENT(COFF, IMAGE_FILE_BYTES_REVERSED_LO      ),
  LLVM_READOBJ_ENUM_ENT(COFF, IMAGE_FILE_32BIT_MACHINE          ),
  LLVM_READOBJ_ENUM_ENT(COFF, IMAGE_FILE_DEBUG_STRIPPED         ),
  LLVM_READOBJ_ENUM_ENT(COFF, IMAGE_FILE_REMOVABLE_RUN_FROM_SWAP),
  LLVM_READOBJ_ENUM_ENT(COFF, IMAGE_FILE_NET_RUN_FROM_SWAP      ),
  LLVM_READOBJ_ENUM_ENT(COFF, IMAGE_FILE_SYSTEM                 ),
  LLVM_READOBJ_ENUM_ENT(COFF, IMAGE_FILE_DLL                    ),
  LLVM_READOBJ_ENUM_ENT(COFF, IMAGE_FILE_UP_SYSTEM_ONLY         ),
  LLVM_READOBJ_ENUM_ENT(COFF, IMAGE_FILE_BYTES_REVERSED_HI      )
};

static const EnumEntry<COFF::WindowsSubsystem> PEWindowsSubsystem[] = {
  LLVM_READOBJ_ENUM_ENT(COFF, IMAGE_SUBSYSTEM_UNKNOWN                ),
  LLVM_READOBJ_ENUM_ENT(COFF, IMAGE_SUBSYSTEM_NATIVE                 ),
  LLVM_READOBJ_ENUM_ENT(COFF, IMAGE_SUBSYSTEM_WINDOWS_GUI            ),
  LLVM_READOBJ_ENUM_ENT(COFF, IMAGE_SUBSYSTEM_WINDOWS_CUI            ),
  LLVM_READOBJ_ENUM_ENT(COFF, IMAGE_SUBSYSTEM_POSIX_CUI              ),
  LLVM_READOBJ_ENUM_ENT(COFF, IMAGE_SUBSYSTEM_WINDOWS_CE_GUI         ),
  LLVM_READOBJ_ENUM_ENT(COFF, IMAGE_SUBSYSTEM_EFI_APPLICATION        ),
  LLVM_READOBJ_ENUM_ENT(COFF, IMAGE_SUBSYSTEM_EFI_BOOT_SERVICE_DRIVER),
  LLVM_READOBJ_ENUM_ENT(COFF, IMAGE_SUBSYSTEM_EFI_RUNTIME_DRIVER     ),
  LLVM_READOBJ_ENUM_ENT(COFF, IMAGE_SUBSYSTEM_EFI_ROM                ),
  LLVM_READOBJ_ENUM_ENT(COFF, IMAGE_SUBSYSTEM_XBOX                   ),
};

static const EnumEntry<COFF::DLLCharacteristics> PEDLLCharacteristics[] = {
  LLVM_READOBJ_ENUM_ENT(COFF, IMAGE_DLL_CHARACTERISTICS_HIGH_ENTROPY_VA      ),
  LLVM_READOBJ_ENUM_ENT(COFF, IMAGE_DLL_CHARACTERISTICS_DYNAMIC_BASE         ),
  LLVM_READOBJ_ENUM_ENT(COFF, IMAGE_DLL_CHARACTERISTICS_FORCE_INTEGRITY      ),
  LLVM_READOBJ_ENUM_ENT(COFF, IMAGE_DLL_CHARACTERISTICS_NX_COMPAT            ),
  LLVM_READOBJ_ENUM_ENT(COFF, IMAGE_DLL_CHARACTERISTICS_NO_ISOLATION         ),
  LLVM_READOBJ_ENUM_ENT(COFF, IMAGE_DLL_CHARACTERISTICS_NO_SEH               ),
  LLVM_READOBJ_ENUM_ENT(COFF, IMAGE_DLL_CHARACTERISTICS_NO_BIND              ),
  LLVM_READOBJ_ENUM_ENT(COFF, IMAGE_DLL_CHARACTERISTICS_APPCONTAINER         ),
  LLVM_READOBJ_ENUM_ENT(COFF, IMAGE_DLL_CHARACTERISTICS_WDM_DRIVER           ),
  LLVM_READOBJ_ENUM_ENT(COFF, IMAGE_DLL_CHARACTERISTICS_GUARD_CF             ),
  LLVM_READOBJ_ENUM_ENT(COFF, IMAGE_DLL_CHARACTERISTICS_TERMINAL_SERVER_AWARE),
};

static const EnumEntry<COFF::SectionCharacteristics>
ImageSectionCharacteristics[] = {
  LLVM_READOBJ_ENUM_ENT(COFF, IMAGE_SCN_TYPE_NOLOAD           ),
  LLVM_READOBJ_ENUM_ENT(COFF, IMAGE_SCN_TYPE_NO_PAD           ),
  LLVM_READOBJ_ENUM_ENT(COFF, IMAGE_SCN_CNT_CODE              ),
  LLVM_READOBJ_ENUM_ENT(COFF, IMAGE_SCN_CNT_INITIALIZED_DATA  ),
  LLVM_READOBJ_ENUM_ENT(COFF, IMAGE_SCN_CNT_UNINITIALIZED_DATA),
  LLVM_READOBJ_ENUM_ENT(COFF, IMAGE_SCN_LNK_OTHER             ),
  LLVM_READOBJ_ENUM_ENT(COFF, IMAGE_SCN_LNK_INFO              ),
  LLVM_READOBJ_ENUM_ENT(COFF, IMAGE_SCN_LNK_REMOVE            ),
  LLVM_READOBJ_ENUM_ENT(COFF, IMAGE_SCN_LNK_COMDAT            ),
  LLVM_READOBJ_ENUM_ENT(COFF, IMAGE_SCN_GPREL                 ),
  LLVM_READOBJ_ENUM_ENT(COFF, IMAGE_SCN_MEM_PURGEABLE         ),
  LLVM_READOBJ_ENUM_ENT(COFF, IMAGE_SCN_MEM_16BIT             ),
  LLVM_READOBJ_ENUM_ENT(COFF, IMAGE_SCN_MEM_LOCKED            ),
  LLVM_READOBJ_ENUM_ENT(COFF, IMAGE_SCN_MEM_PRELOAD           ),
  LLVM_READOBJ_ENUM_ENT(COFF, IMAGE_SCN_ALIGN_1BYTES          ),
  LLVM_READOBJ_ENUM_ENT(COFF, IMAGE_SCN_ALIGN_2BYTES          ),
  LLVM_READOBJ_ENUM_ENT(COFF, IMAGE_SCN_ALIGN_4BYTES          ),
  LLVM_READOBJ_ENUM_ENT(COFF, IMAGE_SCN_ALIGN_8BYTES          ),
  LLVM_READOBJ_ENUM_ENT(COFF, IMAGE_SCN_ALIGN_16BYTES         ),
  LLVM_READOBJ_ENUM_ENT(COFF, IMAGE_SCN_ALIGN_32BYTES         ),
  LLVM_READOBJ_ENUM_ENT(COFF, IMAGE_SCN_ALIGN_64BYTES         ),
  LLVM_READOBJ_ENUM_ENT(COFF, IMAGE_SCN_ALIGN_128BYTES        ),
  LLVM_READOBJ_ENUM_ENT(COFF, IMAGE_SCN_ALIGN_256BYTES        ),
  LLVM_READOBJ_ENUM_ENT(COFF, IMAGE_SCN_ALIGN_512BYTES        ),
  LLVM_READOBJ_ENUM_ENT(COFF, IMAGE_SCN_ALIGN_1024BYTES       ),
  LLVM_READOBJ_ENUM_ENT(COFF, IMAGE_SCN_ALIGN_2048BYTES       ),
  LLVM_READOBJ_ENUM_ENT(COFF, IMAGE_SCN_ALIGN_4096BYTES       ),
  LLVM_READOBJ_ENUM_ENT(COFF, IMAGE_SCN_ALIGN_8192BYTES       ),
  LLVM_READOBJ_ENUM_ENT(COFF, IMAGE_SCN_LNK_NRELOC_OVFL       ),
  LLVM_READOBJ_ENUM_ENT(COFF, IMAGE_SCN_MEM_DISCARDABLE       ),
  LLVM_READOBJ_ENUM_ENT(COFF, IMAGE_SCN_MEM_NOT_CACHED        ),
  LLVM_READOBJ_ENUM_ENT(COFF, IMAGE_SCN_MEM_NOT_PAGED         ),
  LLVM_READOBJ_ENUM_ENT(COFF, IMAGE_SCN_MEM_SHARED            ),
  LLVM_READOBJ_ENUM_ENT(COFF, IMAGE_SCN_MEM_EXECUTE           ),
  LLVM_READOBJ_ENUM_ENT(COFF, IMAGE_SCN_MEM_READ              ),
  LLVM_READOBJ_ENUM_ENT(COFF, IMAGE_SCN_MEM_WRITE             )
};

static const EnumEntry<COFF::SymbolBaseType> ImageSymType[] = {
  { "Null"  , COFF::IMAGE_SYM_TYPE_NULL   },
  { "Void"  , COFF::IMAGE_SYM_TYPE_VOID   },
  { "Char"  , COFF::IMAGE_SYM_TYPE_CHAR   },
  { "Short" , COFF::IMAGE_SYM_TYPE_SHORT  },
  { "Int"   , COFF::IMAGE_SYM_TYPE_INT    },
  { "Long"  , COFF::IMAGE_SYM_TYPE_LONG   },
  { "Float" , COFF::IMAGE_SYM_TYPE_FLOAT  },
  { "Double", COFF::IMAGE_SYM_TYPE_DOUBLE },
  { "Struct", COFF::IMAGE_SYM_TYPE_STRUCT },
  { "Union" , COFF::IMAGE_SYM_TYPE_UNION  },
  { "Enum"  , COFF::IMAGE_SYM_TYPE_ENUM   },
  { "MOE"   , COFF::IMAGE_SYM_TYPE_MOE    },
  { "Byte"  , COFF::IMAGE_SYM_TYPE_BYTE   },
  { "Word"  , COFF::IMAGE_SYM_TYPE_WORD   },
  { "UInt"  , COFF::IMAGE_SYM_TYPE_UINT   },
  { "DWord" , COFF::IMAGE_SYM_TYPE_DWORD  }
};

static const EnumEntry<COFF::SymbolComplexType> ImageSymDType[] = {
  { "Null"    , COFF::IMAGE_SYM_DTYPE_NULL     },
  { "Pointer" , COFF::IMAGE_SYM_DTYPE_POINTER  },
  { "Function", COFF::IMAGE_SYM_DTYPE_FUNCTION },
  { "Array"   , COFF::IMAGE_SYM_DTYPE_ARRAY    }
};

static const EnumEntry<COFF::SymbolStorageClass> ImageSymClass[] = {
  { "EndOfFunction"  , COFF::IMAGE_SYM_CLASS_END_OF_FUNCTION  },
  { "Null"           , COFF::IMAGE_SYM_CLASS_NULL             },
  { "Automatic"      , COFF::IMAGE_SYM_CLASS_AUTOMATIC        },
  { "External"       , COFF::IMAGE_SYM_CLASS_EXTERNAL         },
  { "Static"         , COFF::IMAGE_SYM_CLASS_STATIC           },
  { "Register"       , COFF::IMAGE_SYM_CLASS_REGISTER         },
  { "ExternalDef"    , COFF::IMAGE_SYM_CLASS_EXTERNAL_DEF     },
  { "Label"          , COFF::IMAGE_SYM_CLASS_LABEL            },
  { "UndefinedLabel" , COFF::IMAGE_SYM_CLASS_UNDEFINED_LABEL  },
  { "MemberOfStruct" , COFF::IMAGE_SYM_CLASS_MEMBER_OF_STRUCT },
  { "Argument"       , COFF::IMAGE_SYM_CLASS_ARGUMENT         },
  { "StructTag"      , COFF::IMAGE_SYM_CLASS_STRUCT_TAG       },
  { "MemberOfUnion"  , COFF::IMAGE_SYM_CLASS_MEMBER_OF_UNION  },
  { "UnionTag"       , COFF::IMAGE_SYM_CLASS_UNION_TAG        },
  { "TypeDefinition" , COFF::IMAGE_SYM_CLASS_TYPE_DEFINITION  },
  { "UndefinedStatic", COFF::IMAGE_SYM_CLASS_UNDEFINED_STATIC },
  { "EnumTag"        , COFF::IMAGE_SYM_CLASS_ENUM_TAG         },
  { "MemberOfEnum"   , COFF::IMAGE_SYM_CLASS_MEMBER_OF_ENUM   },
  { "RegisterParam"  , COFF::IMAGE_SYM_CLASS_REGISTER_PARAM   },
  { "BitField"       , COFF::IMAGE_SYM_CLASS_BIT_FIELD        },
  { "Block"          , COFF::IMAGE_SYM_CLASS_BLOCK            },
  { "Function"       , COFF::IMAGE_SYM_CLASS_FUNCTION         },
  { "EndOfStruct"    , COFF::IMAGE_SYM_CLASS_END_OF_STRUCT    },
  { "File"           , COFF::IMAGE_SYM_CLASS_FILE             },
  { "Section"        , COFF::IMAGE_SYM_CLASS_SECTION          },
  { "WeakExternal"   , COFF::IMAGE_SYM_CLASS_WEAK_EXTERNAL    },
  { "CLRToken"       , COFF::IMAGE_SYM_CLASS_CLR_TOKEN        }
};

static const EnumEntry<COFF::COMDATType> ImageCOMDATSelect[] = {
  { "NoDuplicates", COFF::IMAGE_COMDAT_SELECT_NODUPLICATES },
  { "Any"         , COFF::IMAGE_COMDAT_SELECT_ANY          },
  { "SameSize"    , COFF::IMAGE_COMDAT_SELECT_SAME_SIZE    },
  { "ExactMatch"  , COFF::IMAGE_COMDAT_SELECT_EXACT_MATCH  },
  { "Associative" , COFF::IMAGE_COMDAT_SELECT_ASSOCIATIVE  },
  { "Largest"     , COFF::IMAGE_COMDAT_SELECT_LARGEST      },
  { "Newest"      , COFF::IMAGE_COMDAT_SELECT_NEWEST       }
};

static const EnumEntry<COFF::WeakExternalCharacteristics>
WeakExternalCharacteristics[] = {
  { "NoLibrary", COFF::IMAGE_WEAK_EXTERN_SEARCH_NOLIBRARY },
  { "Library"  , COFF::IMAGE_WEAK_EXTERN_SEARCH_LIBRARY   },
  { "Alias"    , COFF::IMAGE_WEAK_EXTERN_SEARCH_ALIAS     }
};

static const EnumEntry<CompileSym3::Flags> CompileSym3Flags[] = {
    LLVM_READOBJ_ENUM_ENT(CompileSym3, EC),
    LLVM_READOBJ_ENUM_ENT(CompileSym3, NoDbgInfo),
    LLVM_READOBJ_ENUM_ENT(CompileSym3, LTCG),
    LLVM_READOBJ_ENUM_ENT(CompileSym3, NoDataAlign),
    LLVM_READOBJ_ENUM_ENT(CompileSym3, ManagedPresent),
    LLVM_READOBJ_ENUM_ENT(CompileSym3, SecurityChecks),
    LLVM_READOBJ_ENUM_ENT(CompileSym3, HotPatch),
    LLVM_READOBJ_ENUM_ENT(CompileSym3, CVTCIL),
    LLVM_READOBJ_ENUM_ENT(CompileSym3, MSILModule),
    LLVM_READOBJ_ENUM_ENT(CompileSym3, Sdl),
    LLVM_READOBJ_ENUM_ENT(CompileSym3, PGO),
    LLVM_READOBJ_ENUM_ENT(CompileSym3, Exp),
};

static const EnumEntry<codeview::SourceLanguage> SourceLanguages[] = {
    LLVM_READOBJ_ENUM_ENT(SourceLanguage, C),
    LLVM_READOBJ_ENUM_ENT(SourceLanguage, CXX),
    LLVM_READOBJ_ENUM_ENT(SourceLanguage, FORTRAN),
    LLVM_READOBJ_ENUM_ENT(SourceLanguage, MASM),
    LLVM_READOBJ_ENUM_ENT(SourceLanguage, PASCAL),
    LLVM_READOBJ_ENUM_ENT(SourceLanguage, BASIC),
    LLVM_READOBJ_ENUM_ENT(SourceLanguage, COBOL),
    LLVM_READOBJ_ENUM_ENT(SourceLanguage, LINK),
    LLVM_READOBJ_ENUM_ENT(SourceLanguage, CVTRES),
    LLVM_READOBJ_ENUM_ENT(SourceLanguage, CVTPGD),
    LLVM_READOBJ_ENUM_ENT(SourceLanguage, CSHARP),
    LLVM_READOBJ_ENUM_ENT(SourceLanguage, VP),
    LLVM_READOBJ_ENUM_ENT(SourceLanguage, ILASM),
    LLVM_READOBJ_ENUM_ENT(SourceLanguage, JAVA),
    LLVM_READOBJ_ENUM_ENT(SourceLanguage, JSCRIPT),
    LLVM_READOBJ_ENUM_ENT(SourceLanguage, MSIL),
    LLVM_READOBJ_ENUM_ENT(SourceLanguage, HLSL),
};

static const EnumEntry<codeview::SubSectionType> SubSectionTypes[] = {
    LLVM_READOBJ_ENUM_ENT(SubSectionType, SUBSEC_SYMBOLS),
    LLVM_READOBJ_ENUM_ENT(SubSectionType, SUBSEC_LINES),
    LLVM_READOBJ_ENUM_ENT(SubSectionType, SUBSEC_STRINGTABLE),
    LLVM_READOBJ_ENUM_ENT(SubSectionType, SUBSEC_FILECHKSMS),
    LLVM_READOBJ_ENUM_ENT(SubSectionType, SUBSEC_FRAMEDATA),
    LLVM_READOBJ_ENUM_ENT(SubSectionType, SUBSEC_INLINEELINES),
    LLVM_READOBJ_ENUM_ENT(SubSectionType, SUBSEC_CROSSSCOPEIMPORTS),
    LLVM_READOBJ_ENUM_ENT(SubSectionType, SUBSEC_CROSSSCOPEEXPORTS),
    LLVM_READOBJ_ENUM_ENT(SubSectionType, SUBSEC_IL_LINES),
    LLVM_READOBJ_ENUM_ENT(SubSectionType, SUBSEC_FUNC_MDTOKEN_MAP),
    LLVM_READOBJ_ENUM_ENT(SubSectionType, SUBSEC_TYPE_MDTOKEN_MAP),
    LLVM_READOBJ_ENUM_ENT(SubSectionType, SUBSEC_MERGED_ASSEMBLYINPUT),
    LLVM_READOBJ_ENUM_ENT(SubSectionType, SUBSEC_COFF_SYMBOL_RVA),
};

static const EnumEntry<codeview::CPUType> CPUTypes[] = {
    LLVM_READOBJ_ENUM_ENT(CPUType, CPU_8080),
    LLVM_READOBJ_ENUM_ENT(CPUType, CPU_8086),
    LLVM_READOBJ_ENUM_ENT(CPUType, CPU_80286),
    LLVM_READOBJ_ENUM_ENT(CPUType, CPU_80386),
    LLVM_READOBJ_ENUM_ENT(CPUType, CPU_80486),
    LLVM_READOBJ_ENUM_ENT(CPUType, CPU_PENTIUM),
    LLVM_READOBJ_ENUM_ENT(CPUType, CPU_PENTIUMPRO),
    LLVM_READOBJ_ENUM_ENT(CPUType, CPU_PENTIUMIII),
    LLVM_READOBJ_ENUM_ENT(CPUType, CPU_MIPS),
    LLVM_READOBJ_ENUM_ENT(CPUType, CPU_MIPS16),
    LLVM_READOBJ_ENUM_ENT(CPUType, CPU_MIPS32),
    LLVM_READOBJ_ENUM_ENT(CPUType, CPU_MIPS64),
    LLVM_READOBJ_ENUM_ENT(CPUType, CPU_MIPSI),
    LLVM_READOBJ_ENUM_ENT(CPUType, CPU_MIPSII),
    LLVM_READOBJ_ENUM_ENT(CPUType, CPU_MIPSIII),
    LLVM_READOBJ_ENUM_ENT(CPUType, CPU_MIPSIV),
    LLVM_READOBJ_ENUM_ENT(CPUType, CPU_MIPSV),
    LLVM_READOBJ_ENUM_ENT(CPUType, CPU_M68000),
    LLVM_READOBJ_ENUM_ENT(CPUType, CPU_M68010),
    LLVM_READOBJ_ENUM_ENT(CPUType, CPU_M68020),
    LLVM_READOBJ_ENUM_ENT(CPUType, CPU_M68030),
    LLVM_READOBJ_ENUM_ENT(CPUType, CPU_M68040),
    LLVM_READOBJ_ENUM_ENT(CPUType, CPU_ALPHA),
    LLVM_READOBJ_ENUM_ENT(CPUType, CPU_ALPHA_21064),
    LLVM_READOBJ_ENUM_ENT(CPUType, CPU_ALPHA_21164),
    LLVM_READOBJ_ENUM_ENT(CPUType, CPU_ALPHA_21164A),
    LLVM_READOBJ_ENUM_ENT(CPUType, CPU_ALPHA_21264),
    LLVM_READOBJ_ENUM_ENT(CPUType, CPU_ALPHA_21364),
    LLVM_READOBJ_ENUM_ENT(CPUType, CPU_PPC601),
    LLVM_READOBJ_ENUM_ENT(CPUType, CPU_PPC603),
    LLVM_READOBJ_ENUM_ENT(CPUType, CPU_PPC604),
    LLVM_READOBJ_ENUM_ENT(CPUType, CPU_PPC620),
    LLVM_READOBJ_ENUM_ENT(CPUType, CPU_PPCFP),
    LLVM_READOBJ_ENUM_ENT(CPUType, CPU_PPCBE),
    LLVM_READOBJ_ENUM_ENT(CPUType, CPU_SH3),
    LLVM_READOBJ_ENUM_ENT(CPUType, CPU_SH3E),
    LLVM_READOBJ_ENUM_ENT(CPUType, CPU_SH3DSP),
    LLVM_READOBJ_ENUM_ENT(CPUType, CPU_SH4),
    LLVM_READOBJ_ENUM_ENT(CPUType, CPU_SHMEDIA),
    LLVM_READOBJ_ENUM_ENT(CPUType, CPU_ARM3),
    LLVM_READOBJ_ENUM_ENT(CPUType, CPU_ARM4),
    LLVM_READOBJ_ENUM_ENT(CPUType, CPU_ARM4T),
    LLVM_READOBJ_ENUM_ENT(CPUType, CPU_ARM5),
    LLVM_READOBJ_ENUM_ENT(CPUType, CPU_ARM5T),
    LLVM_READOBJ_ENUM_ENT(CPUType, CPU_ARM6),
    LLVM_READOBJ_ENUM_ENT(CPUType, CPU_ARM_XMAC),
    LLVM_READOBJ_ENUM_ENT(CPUType, CPU_ARM_WMMX),
    LLVM_READOBJ_ENUM_ENT(CPUType, CPU_ARM7),
    LLVM_READOBJ_ENUM_ENT(CPUType, CPU_OMNI),
    LLVM_READOBJ_ENUM_ENT(CPUType, CPU_IA64),
    LLVM_READOBJ_ENUM_ENT(CPUType, CPU_IA64_1),
    LLVM_READOBJ_ENUM_ENT(CPUType, CPU_IA64_2),
    LLVM_READOBJ_ENUM_ENT(CPUType, CPU_CEE),
    LLVM_READOBJ_ENUM_ENT(CPUType, CPU_AM33),
    LLVM_READOBJ_ENUM_ENT(CPUType, CPU_M32R),
    LLVM_READOBJ_ENUM_ENT(CPUType, CPU_TRICORE),
    LLVM_READOBJ_ENUM_ENT(CPUType, CPU_X64),
    LLVM_READOBJ_ENUM_ENT(CPUType, CPU_EBC),
    LLVM_READOBJ_ENUM_ENT(CPUType, CPU_THUMB),
    LLVM_READOBJ_ENUM_ENT(CPUType, CPU_ARMNT),
    LLVM_READOBJ_ENUM_ENT(CPUType, CPU_ARM64),
    LLVM_READOBJ_ENUM_ENT(CPUType, CPU_D3D11_SHADER),
};


static const EnumEntry<FrameProcSym::Flags> FrameProcSymFlags[] = {
    LLVM_READOBJ_ENUM_ENT(FrameProcSym, HasAlloca),
    LLVM_READOBJ_ENUM_ENT(FrameProcSym, HasSetJmp),
    LLVM_READOBJ_ENUM_ENT(FrameProcSym, HasLongJmp),
    LLVM_READOBJ_ENUM_ENT(FrameProcSym, HasInlAsm),
    LLVM_READOBJ_ENUM_ENT(FrameProcSym, HasEH),
    LLVM_READOBJ_ENUM_ENT(FrameProcSym, InlSpec),
    LLVM_READOBJ_ENUM_ENT(FrameProcSym, HasSEH),
    LLVM_READOBJ_ENUM_ENT(FrameProcSym, Naked),
    LLVM_READOBJ_ENUM_ENT(FrameProcSym, SecurityChecks),
    LLVM_READOBJ_ENUM_ENT(FrameProcSym, AsyncEH),
    LLVM_READOBJ_ENUM_ENT(FrameProcSym, GSNoStackOrdering),
    LLVM_READOBJ_ENUM_ENT(FrameProcSym, WasInlined),
    LLVM_READOBJ_ENUM_ENT(FrameProcSym, GSCheck),
    LLVM_READOBJ_ENUM_ENT(FrameProcSym, SafeBuffers),
    LLVM_READOBJ_ENUM_ENT(FrameProcSym, encodedLocalBasePointer1),
    LLVM_READOBJ_ENUM_ENT(FrameProcSym, encodedLocalBasePointer2),
    LLVM_READOBJ_ENUM_ENT(FrameProcSym, encodedParamBasePointer1),
    LLVM_READOBJ_ENUM_ENT(FrameProcSym, encodedParamBasePointer2),
    LLVM_READOBJ_ENUM_ENT(FrameProcSym, PogoOn),
    LLVM_READOBJ_ENUM_ENT(FrameProcSym, ValidCounts),
    LLVM_READOBJ_ENUM_ENT(FrameProcSym, OptSpeed),
    LLVM_READOBJ_ENUM_ENT(FrameProcSym, GuardCF),
    LLVM_READOBJ_ENUM_ENT(FrameProcSym, GuardCFW),
};

static const EnumEntry<uint16_t> TagPropertyFlags[] = {
    LLVM_READOBJ_ENUM_ENT(TagProperties, packed),
    LLVM_READOBJ_ENUM_ENT(TagProperties, ctor),
    LLVM_READOBJ_ENUM_ENT(TagProperties, ovlops),
    LLVM_READOBJ_ENUM_ENT(TagProperties, isnested),
    LLVM_READOBJ_ENUM_ENT(TagProperties, cnested),
    LLVM_READOBJ_ENUM_ENT(TagProperties, opassign),
    LLVM_READOBJ_ENUM_ENT(TagProperties, opcast),
    LLVM_READOBJ_ENUM_ENT(TagProperties, fwdref),
    LLVM_READOBJ_ENUM_ENT(TagProperties, scoped),
    LLVM_READOBJ_ENUM_ENT(TagProperties, asuniquename),
    LLVM_READOBJ_ENUM_ENT(TagProperties, sealed),
    LLVM_READOBJ_ENUM_ENT(TagProperties, hfa0),
    LLVM_READOBJ_ENUM_ENT(TagProperties, hfa1),
    LLVM_READOBJ_ENUM_ENT(TagProperties, intrinsic),
    LLVM_READOBJ_ENUM_ENT(TagProperties, mocom0),
    LLVM_READOBJ_ENUM_ENT(TagProperties, mocom1),
};

template <typename T>
static std::error_code getSymbolAuxData(const COFFObjectFile *Obj,
                                        COFFSymbolRef Symbol,
                                        uint8_t AuxSymbolIdx, const T *&Aux) {
  ArrayRef<uint8_t> AuxData = Obj->getSymbolAuxData(Symbol);
  AuxData = AuxData.slice(AuxSymbolIdx * Obj->getSymbolTableEntrySize());
  Aux = reinterpret_cast<const T*>(AuxData.data());
  return readobj_error::success;
}

void COFFDumper::cacheRelocations() {
  if (RelocCached)
    return;
  RelocCached = true;

  for (const SectionRef &S : Obj->sections()) {
    const coff_section *Section = Obj->getCOFFSection(S);

    for (const RelocationRef &Reloc : S.relocations())
      RelocMap[Section].push_back(Reloc);

    // Sort relocations by address.
    std::sort(RelocMap[Section].begin(), RelocMap[Section].end(),
              relocAddressLess);
  }
}

void COFFDumper::printDataDirectory(uint32_t Index, const std::string &FieldName) {
  const data_directory *Data;
  if (Obj->getDataDirectory(Index, Data))
    return;
  W.printHex(FieldName + "RVA", Data->RelativeVirtualAddress);
  W.printHex(FieldName + "Size", Data->Size);
}

void COFFDumper::printFileHeaders() {
  time_t TDS = Obj->getTimeDateStamp();
  char FormattedTime[20] = { };
  strftime(FormattedTime, 20, "%Y-%m-%d %H:%M:%S", gmtime(&TDS));

  {
    DictScope D(W, "ImageFileHeader");
    W.printEnum  ("Machine", Obj->getMachine(),
                    makeArrayRef(ImageFileMachineType));
    W.printNumber("SectionCount", Obj->getNumberOfSections());
    W.printHex   ("TimeDateStamp", FormattedTime, Obj->getTimeDateStamp());
    W.printHex   ("PointerToSymbolTable", Obj->getPointerToSymbolTable());
    W.printNumber("SymbolCount", Obj->getNumberOfSymbols());
    W.printNumber("OptionalHeaderSize", Obj->getSizeOfOptionalHeader());
    W.printFlags ("Characteristics", Obj->getCharacteristics(),
                    makeArrayRef(ImageFileCharacteristics));
  }

  // Print PE header. This header does not exist if this is an object file and
  // not an executable.
  const pe32_header *PEHeader = nullptr;
  error(Obj->getPE32Header(PEHeader));
  if (PEHeader)
    printPEHeader<pe32_header>(PEHeader);

  const pe32plus_header *PEPlusHeader = nullptr;
  error(Obj->getPE32PlusHeader(PEPlusHeader));
  if (PEPlusHeader)
    printPEHeader<pe32plus_header>(PEPlusHeader);

  if (const dos_header *DH = Obj->getDOSHeader())
    printDOSHeader(DH);
}

void COFFDumper::printDOSHeader(const dos_header *DH) {
  DictScope D(W, "DOSHeader");
  W.printString("Magic", StringRef(DH->Magic, sizeof(DH->Magic)));
  W.printNumber("UsedBytesInTheLastPage", DH->UsedBytesInTheLastPage);
  W.printNumber("FileSizeInPages", DH->FileSizeInPages);
  W.printNumber("NumberOfRelocationItems", DH->NumberOfRelocationItems);
  W.printNumber("HeaderSizeInParagraphs", DH->HeaderSizeInParagraphs);
  W.printNumber("MinimumExtraParagraphs", DH->MinimumExtraParagraphs);
  W.printNumber("MaximumExtraParagraphs", DH->MaximumExtraParagraphs);
  W.printNumber("InitialRelativeSS", DH->InitialRelativeSS);
  W.printNumber("InitialSP", DH->InitialSP);
  W.printNumber("Checksum", DH->Checksum);
  W.printNumber("InitialIP", DH->InitialIP);
  W.printNumber("InitialRelativeCS", DH->InitialRelativeCS);
  W.printNumber("AddressOfRelocationTable", DH->AddressOfRelocationTable);
  W.printNumber("OverlayNumber", DH->OverlayNumber);
  W.printNumber("OEMid", DH->OEMid);
  W.printNumber("OEMinfo", DH->OEMinfo);
  W.printNumber("AddressOfNewExeHeader", DH->AddressOfNewExeHeader);
}

template <class PEHeader>
void COFFDumper::printPEHeader(const PEHeader *Hdr) {
  DictScope D(W, "ImageOptionalHeader");
  W.printNumber("MajorLinkerVersion", Hdr->MajorLinkerVersion);
  W.printNumber("MinorLinkerVersion", Hdr->MinorLinkerVersion);
  W.printNumber("SizeOfCode", Hdr->SizeOfCode);
  W.printNumber("SizeOfInitializedData", Hdr->SizeOfInitializedData);
  W.printNumber("SizeOfUninitializedData", Hdr->SizeOfUninitializedData);
  W.printHex   ("AddressOfEntryPoint", Hdr->AddressOfEntryPoint);
  W.printHex   ("BaseOfCode", Hdr->BaseOfCode);
  printBaseOfDataField(Hdr);
  W.printHex   ("ImageBase", Hdr->ImageBase);
  W.printNumber("SectionAlignment", Hdr->SectionAlignment);
  W.printNumber("FileAlignment", Hdr->FileAlignment);
  W.printNumber("MajorOperatingSystemVersion",
                Hdr->MajorOperatingSystemVersion);
  W.printNumber("MinorOperatingSystemVersion",
                Hdr->MinorOperatingSystemVersion);
  W.printNumber("MajorImageVersion", Hdr->MajorImageVersion);
  W.printNumber("MinorImageVersion", Hdr->MinorImageVersion);
  W.printNumber("MajorSubsystemVersion", Hdr->MajorSubsystemVersion);
  W.printNumber("MinorSubsystemVersion", Hdr->MinorSubsystemVersion);
  W.printNumber("SizeOfImage", Hdr->SizeOfImage);
  W.printNumber("SizeOfHeaders", Hdr->SizeOfHeaders);
  W.printEnum  ("Subsystem", Hdr->Subsystem, makeArrayRef(PEWindowsSubsystem));
  W.printFlags ("Characteristics", Hdr->DLLCharacteristics,
                makeArrayRef(PEDLLCharacteristics));
  W.printNumber("SizeOfStackReserve", Hdr->SizeOfStackReserve);
  W.printNumber("SizeOfStackCommit", Hdr->SizeOfStackCommit);
  W.printNumber("SizeOfHeapReserve", Hdr->SizeOfHeapReserve);
  W.printNumber("SizeOfHeapCommit", Hdr->SizeOfHeapCommit);
  W.printNumber("NumberOfRvaAndSize", Hdr->NumberOfRvaAndSize);

  if (Hdr->NumberOfRvaAndSize > 0) {
    DictScope D(W, "DataDirectory");
    static const char * const directory[] = {
      "ExportTable", "ImportTable", "ResourceTable", "ExceptionTable",
      "CertificateTable", "BaseRelocationTable", "Debug", "Architecture",
      "GlobalPtr", "TLSTable", "LoadConfigTable", "BoundImport", "IAT",
      "DelayImportDescriptor", "CLRRuntimeHeader", "Reserved"
    };

    for (uint32_t i = 0; i < Hdr->NumberOfRvaAndSize; ++i) {
      printDataDirectory(i, directory[i]);
    }
  }
}

void COFFDumper::printBaseOfDataField(const pe32_header *Hdr) {
  W.printHex("BaseOfData", Hdr->BaseOfData);
}

void COFFDumper::printBaseOfDataField(const pe32plus_header *) {}

void COFFDumper::printCodeViewDebugInfo() {
  for (const SectionRef &S : Obj->sections()) {
    StringRef SectionName;
    error(S.getName(SectionName));
    if (SectionName == ".debug$S")
      printCodeViewSymbolSection(SectionName, S);
    else if (SectionName == ".debug$T")
      printCodeViewTypeSection(SectionName, S);
  }
}

void COFFDumper::printCodeViewSymbolSection(StringRef SectionName,
                                            const SectionRef &Section) {
  StringRef Data;
  error(Section.getContents(Data));

  SmallVector<StringRef, 10> FunctionNames;
  StringMap<StringRef> FunctionLineTables;

  ListScope D(W, "CodeViewDebugInfo");
  // Print the section to allow correlation with printSections.
  W.printNumber("Section", SectionName, Obj->getSectionID(Section));

  {
    // FIXME: Add more offset correctness checks.
    DataExtractor DE(Data, true, 4);
    uint32_t Offset = 0,
             Magic = DE.getU32(&Offset);
    W.printHex("Magic", Magic);
    if (Magic != COFF::DEBUG_SECTION_MAGIC)
      return error(object_error::parse_failed);

    bool Finished = false;
    while (DE.isValidOffset(Offset) && !Finished) {
      // The section consists of a number of subsection in the following format:
      // |SubSectionType|SubSectionSize|Contents...|
      uint32_t SubType = DE.getU32(&Offset),
               SubSectionSize = DE.getU32(&Offset);
      ListScope S(W, "Subsection");
      W.printEnum("SubSectionType", SubType,
                  makeArrayRef(SubSectionTypes));
      W.printHex("SubSectionSize", SubSectionSize);

      // Get the contents of the subsection.
      if (SubSectionSize > Data.size() - Offset)
        return error(object_error::parse_failed);
      StringRef Contents = Data.substr(Offset, SubSectionSize);

      // Optionally print the subsection bytes in case our parsing gets confused
      // later.
      if (opts::CodeViewSubsectionBytes)
        W.printBinaryBlock("SubSectionContents", Contents);

      switch (SubType) {
      case SUBSEC_SYMBOLS:
        printCodeViewSymbolsSubsection(Contents, Section, Offset);
        break;
      case SUBSEC_LINES: {
        // Holds a PC to file:line table.  Some data to parse this subsection is
        // stored in the other subsections, so just check sanity and store the
        // pointers for deferred processing.

        if (SubSectionSize < 12) {
          // There should be at least three words to store two function
          // relocations and size of the code.
          error(object_error::parse_failed);
          return;
        }

        StringRef LinkageName;
        error(resolveSymbolName(Obj->getCOFFSection(Section), Offset,
                                LinkageName));
        W.printString("LinkageName", LinkageName);
        if (FunctionLineTables.count(LinkageName) != 0) {
          // Saw debug info for this function already?
          error(object_error::parse_failed);
          return;
        }

        FunctionLineTables[LinkageName] = Contents;
        FunctionNames.push_back(LinkageName);
        break;
      }
      case SUBSEC_STRINGTABLE:
        if (SubSectionSize == 0 || CVStringTable.data() != nullptr ||
            Contents.back() != '\0') {
          // Empty or duplicate or non-null-terminated subsection.
          error(object_error::parse_failed);
          return;
        }
        CVStringTable = Contents;
        break;
      case SUBSEC_FILECHKSMS:
        // Holds the translation table from file indices
        // to offsets in the string table.

        if (SubSectionSize == 0 ||
            CVFileIndexToStringOffsetTable.data() != nullptr) {
          // Empty or duplicate subsection.
          error(object_error::parse_failed);
          return;
        }
        CVFileIndexToStringOffsetTable = Contents;
        break;
      }

      Offset += SubSectionSize;

      // Align the reading pointer by 4.
      Offset += (-Offset) % 4;
    }
  }

  // Dump the line tables now that we've read all the subsections and know all
  // the required information.
  for (unsigned I = 0, E = FunctionNames.size(); I != E; ++I) {
    StringRef Name = FunctionNames[I];
    ListScope S(W, "FunctionLineTable");
    W.printString("LinkageName", Name);

    DataExtractor DE(FunctionLineTables[Name], true, 4);
    uint32_t Offset = 6;  // Skip relocations.
    uint16_t Flags = DE.getU16(&Offset);
    W.printHex("Flags", Flags);
    bool HasColumnInformation =
        Flags & COFF::DEBUG_LINE_TABLES_HAVE_COLUMN_RECORDS;
    uint32_t FunctionSize = DE.getU32(&Offset);
    W.printHex("CodeSize", FunctionSize);
    while (DE.isValidOffset(Offset)) {
      // For each range of lines with the same filename, we have a segment
      // in the line table.  The filename string is accessed using double
      // indirection to the string table subsection using the index subsection.
      uint32_t OffsetInIndex = DE.getU32(&Offset),
               SegmentLength = DE.getU32(&Offset),
               FullSegmentSize = DE.getU32(&Offset);

      if (FullSegmentSize !=
          12 + 8 * SegmentLength +
              (HasColumnInformation ? 4 * SegmentLength : 0)) {
        error(object_error::parse_failed);
        return;
      }

      uint32_t FilenameOffset;
      {
        DataExtractor SDE(CVFileIndexToStringOffsetTable, true, 4);
        uint32_t OffsetInSDE = OffsetInIndex;
        if (!SDE.isValidOffset(OffsetInSDE)) {
          error(object_error::parse_failed);
          return;
        }
        FilenameOffset = SDE.getU32(&OffsetInSDE);
      }

      if (FilenameOffset == 0 || FilenameOffset + 1 >= CVStringTable.size() ||
          CVStringTable.data()[FilenameOffset - 1] != '\0') {
        // Each string in an F3 subsection should be preceded by a null
        // character.
        error(object_error::parse_failed);
        return;
      }

      StringRef Filename(CVStringTable.data() + FilenameOffset);
      ListScope S(W, "FilenameSegment");
      W.printString("Filename", Filename);
      for (unsigned J = 0; J != SegmentLength && DE.isValidOffset(Offset);
           ++J) {
        // Then go the (PC, LineNumber) pairs.  The line number is stored in the
        // least significant 31 bits of the respective word in the table.
        uint32_t PC = DE.getU32(&Offset),
                 LineNumber = DE.getU32(&Offset) & 0x7fffffff;
        if (PC >= FunctionSize) {
          error(object_error::parse_failed);
          return;
        }
        char Buffer[32];
        format("+0x%X", PC).snprint(Buffer, 32);
        W.printNumber(Buffer, LineNumber);
      }
      if (HasColumnInformation) {
        for (unsigned J = 0; J != SegmentLength && DE.isValidOffset(Offset);
             ++J) {
          uint16_t ColStart = DE.getU16(&Offset);
          W.printNumber("ColStart", ColStart);
          uint16_t ColEnd = DE.getU16(&Offset);
          W.printNumber("ColEnd", ColEnd);
        }
      }
    }
  }
}


/// Get the next symbol record. Returns null on reaching the end of Data, or if
/// the next record extends beyond the end of Data.
static const SymRecord *nextRecord(const SymRecord *Rec, StringRef Data) {
  const char *Next =
      reinterpret_cast<const char *>(Rec) + sizeof(Rec->reclen) + Rec->reclen;
  ptrdiff_t Diff = Next - Data.data();
  if (Diff < 0)
    return nullptr;
  if (size_t(Diff) + sizeof(SymRecord) > Data.size())
    return nullptr;
  Rec = reinterpret_cast<const SymRecord *>(Next);
  if (size_t(Diff) + Rec->reclen > Data.size())
    return nullptr;
  return Rec;
}

template <typename T>
static const T *castSymRec(const SymRecord *Rec) {
  if (sizeof(T) > Rec->reclen + sizeof(Rec->reclen))
    return nullptr;
  return reinterpret_cast<const T*>(Rec);
}

void COFFDumper::printCodeViewSymbolsSubsection(StringRef Subsection,
                                                const SectionRef &Section,
                                                uint32_t OffsetInSection) {
  if (Subsection.size() < sizeof(SymRecord))
    return error(object_error::parse_failed);

  bool InFunctionScope = false;
  for (const SymRecord *Rec =
           reinterpret_cast<const SymRecord *>(Subsection.data());
       Rec != nullptr; Rec = nextRecord(Rec, Subsection)) {
    SymType Type = static_cast<SymType>(uint16_t(Rec->rectyp));
    switch (Type) {
    case S_LPROC32_ID:
    case S_GPROC32_ID: {
      DictScope S(W, "ProcStart");
      const ProcSym *Proc = castSymRec<ProcSym>(Rec);
      if (!Proc || InFunctionScope)
        return error(object_error::parse_failed);
      InFunctionScope = true;

      // Find the relocation that will be applied to the 'off' field, and get
      // the symbol associated with it.
      ptrdiff_t OffsetOfOff =
          reinterpret_cast<const char *>(&Proc->off) - Subsection.data();
      StringRef LinkageName;
      error(resolveSymbolName(Obj->getCOFFSection(Section),
                              OffsetInSection + OffsetOfOff, LinkageName));

      W.printHex("CodeSize", Proc->len);
      size_t DisplayNameLen =
          (Proc->reclen + sizeof(Proc->reclen)) - sizeof(*Proc);
      if (DisplayNameLen) {
        StringRef DisplayName = StringRef(Proc->name, DisplayNameLen);
        W.printString("DisplayName", DisplayName);
      }
      W.printString("LinkageName", LinkageName);
      break;
    }
    case S_PROC_ID_END: {
      if (!InFunctionScope || Rec->reclen != sizeof(Rec->rectyp))
        return error(object_error::parse_failed);
      W.startLine() << "ProcEnd\n";
      InFunctionScope = false;
      break;
    }
    case S_OBJNAME: {
      DictScope S(W, "ObjectName");
      const auto *ObjName = castSymRec<ObjNameSym>(Rec);
      W.printHex("Signature", ObjName->signature);
      size_t ObjectNameLen =
          (ObjName->reclen + sizeof(ObjName->reclen)) - sizeof(*ObjName);
      if (ObjectNameLen) {
        StringRef ObjectName = StringRef(ObjName->name, ObjectNameLen);
        W.printString("ObjectName", ObjectName);
      }
      break;
    }
    case S_COMPILE3: {
      DictScope S(W, "CompilerFlags");
      const auto *CompFlags = castSymRec<CompileSym3>(Rec);
      W.printEnum("Language", CompFlags->getLanguage(),
                  makeArrayRef(SourceLanguages));
      W.printFlags("Flags", CompFlags->flags & ~0xff,
                   makeArrayRef(CompileSym3Flags));
      W.printEnum("Machine", CompFlags->machine, makeArrayRef(CPUTypes));
      std::string FrontendVersion;
      {
        raw_string_ostream Out(FrontendVersion);
        Out << CompFlags->verFEMajor << '.' << CompFlags->verFEMinor << '.'
            << CompFlags->verFEBuild << '.' << CompFlags->verFEQFE;
      }
      std::string BackendVersion;
      {
        raw_string_ostream Out(BackendVersion);
        Out << CompFlags->verMajor << '.' << CompFlags->verMinor << '.'
            << CompFlags->verBuild << '.' << CompFlags->verQFE;
      }
      W.printString("FrontendVersion", FrontendVersion);
      W.printString("BackendVersion", BackendVersion);
      size_t VersionNameLen =
          (CompFlags->reclen + sizeof(CompFlags->reclen)) - sizeof(*CompFlags);
      if (VersionNameLen) {
        StringRef VersionName = StringRef(CompFlags->verSz, VersionNameLen);
        W.printString("VersionName", VersionName);
      }
      break;
    }
    case S_FRAMEPROC: {
      DictScope S(W, "FrameProc");
      const auto *FrameProc = castSymRec<FrameProcSym>(Rec);
      W.printHex("TotalFrameBytes", FrameProc->cbFrame);
      W.printHex("PaddingFrameBytes", FrameProc->cbPad);
      W.printHex("OffsetToPadding", FrameProc->offPad);
      W.printHex("BytesOfCalleeSavedRegister", FrameProc->cbSaveRegs);
      W.printHex("OffsetOfExceptionHandler", FrameProc->offExHdlr);
      W.printHex("SectionIdOfExceptionHandler", FrameProc->sectExHdlr);
      W.printFlags("Flags", FrameProc->flags, makeArrayRef(FrameProcSymFlags));
      break;
    }

    case S_UDT: {
      DictScope S(W, "UDT");
      const auto *UDT = castSymRec<UDTSym>(Rec);
      size_t UDTNameLen = (UDT->reclen + sizeof(UDT->reclen)) - sizeof(*UDT);
      if (UDTNameLen) {
        StringRef UDTName = StringRef(UDT->name, UDTNameLen);
        W.printString("Name", UDTName);
      }
      break;
    }

    case S_BPREL32: {
      DictScope S(W, "BPRelativeSym");
      const auto *BPRel = castSymRec<BPRelativeSym>(Rec);
      W.printHex("Offset", BPRel->off);
      W.printHex("TypeIndex", BPRel->typind);
      size_t NameLen = (BPRel->reclen + sizeof(BPRel->reclen)) - sizeof(*BPRel);
      StringRef VarName = StringRef(BPRel->name, NameLen);
      W.printString("VarName", VarName);
      break;
    }

    case S_REGREL32: {
      DictScope S(W, "RegRelativeSym");
      const auto *RegRel = castSymRec<RegRelativeSym>(Rec);
      W.printHex("Offset", RegRel->off);
      W.printHex("TypeIndex", RegRel->typind);
      W.printHex("Register", RegRel->reg);
      size_t NameLen =
          (RegRel->reclen + sizeof(RegRel->reclen)) - sizeof(*RegRel);
      StringRef VarName = StringRef(RegRel->name, NameLen);
      W.printString("VarName", VarName);
      break;
    }

    case S_BUILDINFO: {
      DictScope S(W, "BuildInfo");
      const auto *BuildInfo = castSymRec<BuildInfoSym>(Rec);
      W.printNumber("Id", BuildInfo->id);
      break;
    }

    default: {
      if (opts::CodeViewSubsectionBytes) {
        ListScope S(W, "Record");
        W.printHex("Type", Rec->rectyp);
        W.printHex("Size", Rec->reclen);

        StringRef Contents =
            StringRef(reinterpret_cast<const char *>(Rec + 1), Rec->reclen - 2);
        W.printBinaryBlock("Contents", Contents);
      }
      break;
    }
    }
  }
}

StringRef nextType(StringRef Data, const TypeRecord *Rec) {
  return Data.drop_front(sizeof(Rec->len) + Rec->len);
}

template <typename T>
static const T *castTypeRec(const TypeRecord *Rec) {
  if (sizeof(T) > Rec->len + sizeof(Rec->len))
    return nullptr;
  return reinterpret_cast<const T*>(Rec);
}


void COFFDumper::printCodeViewTypeSection(StringRef SectionName,
                                          const SectionRef &Section) {
  ListScope D(W, "CodeViewTypes");
  W.printNumber("Section", SectionName, Obj->getSectionID(Section));
  StringRef Data;
  error(Section.getContents(Data));
  W.printBinaryBlock("Data", Data);

  unsigned Magic = *reinterpret_cast<const ulittle32_t *>(Data.data());
  W.printHex("Magic", Magic);

  Data = Data.drop_front(4);

  while (!Data.empty()) {
    if (Data.size() < sizeof(TypeRecord))
      return error(object_error::parse_failed);
    auto *Rec = reinterpret_cast<const TypeRecord *>(Data.data());
    auto Leaf = static_cast<LeafType>(uint16_t(Rec->leaf));

    switch (Leaf) {
    default:
      break;
    case LF_CLASS:
    case LF_STRUCTURE:
      auto *Class = castTypeRec<ClassType>(Rec);
      if (!Class)
        return error(object_error::parse_failed);
      ListScope S(W, "ClassType");
      W.printNumber("MemberCount", Class->count);
      W.printFlags("Properties", uint16_t(Class->property),
                   makeArrayRef(TagPropertyFlags));
      W.printNumber("FieldTypeIndex", Class->field);
      W.printNumber("DerivedFrom", Class->derived);
      W.printNumber("VShape", Class->vshape);
      StringRef NameData(&Class->data[0], Rec->len + 2 - sizeof(*Class));
      W.printBinaryBlock("NameData", NameData);
      break;
    }

    Data = nextType(Data, Rec);

    // 0xF1... alignment
  }
}


void COFFDumper::printSections() {
  ListScope SectionsD(W, "Sections");
  int SectionNumber = 0;
  for (const SectionRef &Sec : Obj->sections()) {
    ++SectionNumber;
    const coff_section *Section = Obj->getCOFFSection(Sec);

    StringRef Name;
    error(Sec.getName(Name));

    DictScope D(W, "Section");
    W.printNumber("Number", SectionNumber);
    W.printBinary("Name", Name, Section->Name);
    W.printHex   ("VirtualSize", Section->VirtualSize);
    W.printHex   ("VirtualAddress", Section->VirtualAddress);
    W.printNumber("RawDataSize", Section->SizeOfRawData);
    W.printHex   ("PointerToRawData", Section->PointerToRawData);
    W.printHex   ("PointerToRelocations", Section->PointerToRelocations);
    W.printHex   ("PointerToLineNumbers", Section->PointerToLinenumbers);
    W.printNumber("RelocationCount", Section->NumberOfRelocations);
    W.printNumber("LineNumberCount", Section->NumberOfLinenumbers);
    W.printFlags ("Characteristics", Section->Characteristics,
                    makeArrayRef(ImageSectionCharacteristics),
                    COFF::SectionCharacteristics(0x00F00000));

    if (opts::SectionRelocations) {
      ListScope D(W, "Relocations");
      for (const RelocationRef &Reloc : Sec.relocations())
        printRelocation(Sec, Reloc);
    }

    if (opts::SectionSymbols) {
      ListScope D(W, "Symbols");
      for (const SymbolRef &Symbol : Obj->symbols()) {
        if (!Sec.containsSymbol(Symbol))
          continue;

        printSymbol(Symbol);
      }
    }

    if (opts::SectionData &&
        !(Section->Characteristics & COFF::IMAGE_SCN_CNT_UNINITIALIZED_DATA)) {
      StringRef Data;
      error(Sec.getContents(Data));

      W.printBinaryBlock("SectionData", Data);
    }
  }
}

void COFFDumper::printRelocations() {
  ListScope D(W, "Relocations");

  int SectionNumber = 0;
  for (const SectionRef &Section : Obj->sections()) {
    ++SectionNumber;
    StringRef Name;
    error(Section.getName(Name));

    bool PrintedGroup = false;
    for (const RelocationRef &Reloc : Section.relocations()) {
      if (!PrintedGroup) {
        W.startLine() << "Section (" << SectionNumber << ") " << Name << " {\n";
        W.indent();
        PrintedGroup = true;
      }

      printRelocation(Section, Reloc);
    }

    if (PrintedGroup) {
      W.unindent();
      W.startLine() << "}\n";
    }
  }
}

void COFFDumper::printRelocation(const SectionRef &Section,
                                 const RelocationRef &Reloc) {
  uint64_t Offset = Reloc.getOffset();
  uint64_t RelocType = Reloc.getType();
  SmallString<32> RelocName;
  StringRef SymbolName;
  Reloc.getTypeName(RelocName);
  symbol_iterator Symbol = Reloc.getSymbol();
  if (Symbol != Obj->symbol_end()) {
    ErrorOr<StringRef> SymbolNameOrErr = Symbol->getName();
    error(SymbolNameOrErr.getError());
    SymbolName = *SymbolNameOrErr;
  }

  if (opts::ExpandRelocs) {
    DictScope Group(W, "Relocation");
    W.printHex("Offset", Offset);
    W.printNumber("Type", RelocName, RelocType);
    W.printString("Symbol", SymbolName.empty() ? "-" : SymbolName);
  } else {
    raw_ostream& OS = W.startLine();
    OS << W.hex(Offset)
       << " " << RelocName
       << " " << (SymbolName.empty() ? "-" : SymbolName)
       << "\n";
  }
}

void COFFDumper::printSymbols() {
  ListScope Group(W, "Symbols");

  for (const SymbolRef &Symbol : Obj->symbols())
    printSymbol(Symbol);
}

void COFFDumper::printDynamicSymbols() { ListScope Group(W, "DynamicSymbols"); }

static ErrorOr<StringRef>
getSectionName(const llvm::object::COFFObjectFile *Obj, int32_t SectionNumber,
               const coff_section *Section) {
  if (Section) {
    StringRef SectionName;
    if (std::error_code EC = Obj->getSectionName(Section, SectionName))
      return EC;
    return SectionName;
  }
  if (SectionNumber == llvm::COFF::IMAGE_SYM_DEBUG)
    return StringRef("IMAGE_SYM_DEBUG");
  if (SectionNumber == llvm::COFF::IMAGE_SYM_ABSOLUTE)
    return StringRef("IMAGE_SYM_ABSOLUTE");
  if (SectionNumber == llvm::COFF::IMAGE_SYM_UNDEFINED)
    return StringRef("IMAGE_SYM_UNDEFINED");
  return StringRef("");
}

void COFFDumper::printSymbol(const SymbolRef &Sym) {
  DictScope D(W, "Symbol");

  COFFSymbolRef Symbol = Obj->getCOFFSymbol(Sym);
  const coff_section *Section;
  if (std::error_code EC = Obj->getSection(Symbol.getSectionNumber(), Section)) {
    W.startLine() << "Invalid section number: " << EC.message() << "\n";
    W.flush();
    return;
  }

  StringRef SymbolName;
  if (Obj->getSymbolName(Symbol, SymbolName))
    SymbolName = "";

  StringRef SectionName = "";
  ErrorOr<StringRef> Res =
      getSectionName(Obj, Symbol.getSectionNumber(), Section);
  if (Res)
    SectionName = *Res;

  W.printString("Name", SymbolName);
  W.printNumber("Value", Symbol.getValue());
  W.printNumber("Section", SectionName, Symbol.getSectionNumber());
  W.printEnum  ("BaseType", Symbol.getBaseType(), makeArrayRef(ImageSymType));
  W.printEnum  ("ComplexType", Symbol.getComplexType(),
                                                   makeArrayRef(ImageSymDType));
  W.printEnum  ("StorageClass", Symbol.getStorageClass(),
                                                   makeArrayRef(ImageSymClass));
  W.printNumber("AuxSymbolCount", Symbol.getNumberOfAuxSymbols());

  for (uint8_t I = 0; I < Symbol.getNumberOfAuxSymbols(); ++I) {
    if (Symbol.isFunctionDefinition()) {
      const coff_aux_function_definition *Aux;
      error(getSymbolAuxData(Obj, Symbol, I, Aux));

      DictScope AS(W, "AuxFunctionDef");
      W.printNumber("TagIndex", Aux->TagIndex);
      W.printNumber("TotalSize", Aux->TotalSize);
      W.printHex("PointerToLineNumber", Aux->PointerToLinenumber);
      W.printHex("PointerToNextFunction", Aux->PointerToNextFunction);

    } else if (Symbol.isAnyUndefined()) {
      const coff_aux_weak_external *Aux;
      error(getSymbolAuxData(Obj, Symbol, I, Aux));

      ErrorOr<COFFSymbolRef> Linked = Obj->getSymbol(Aux->TagIndex);
      StringRef LinkedName;
      std::error_code EC = Linked.getError();
      if (EC || (EC = Obj->getSymbolName(*Linked, LinkedName))) {
        LinkedName = "";
        error(EC);
      }

      DictScope AS(W, "AuxWeakExternal");
      W.printNumber("Linked", LinkedName, Aux->TagIndex);
      W.printEnum  ("Search", Aux->Characteristics,
                    makeArrayRef(WeakExternalCharacteristics));

    } else if (Symbol.isFileRecord()) {
      const char *FileName;
      error(getSymbolAuxData(Obj, Symbol, I, FileName));

      DictScope AS(W, "AuxFileRecord");

      StringRef Name(FileName, Symbol.getNumberOfAuxSymbols() *
                                   Obj->getSymbolTableEntrySize());
      W.printString("FileName", Name.rtrim(StringRef("\0", 1)));
      break;
    } else if (Symbol.isSectionDefinition()) {
      const coff_aux_section_definition *Aux;
      error(getSymbolAuxData(Obj, Symbol, I, Aux));

      int32_t AuxNumber = Aux->getNumber(Symbol.isBigObj());

      DictScope AS(W, "AuxSectionDef");
      W.printNumber("Length", Aux->Length);
      W.printNumber("RelocationCount", Aux->NumberOfRelocations);
      W.printNumber("LineNumberCount", Aux->NumberOfLinenumbers);
      W.printHex("Checksum", Aux->CheckSum);
      W.printNumber("Number", AuxNumber);
      W.printEnum("Selection", Aux->Selection, makeArrayRef(ImageCOMDATSelect));

      if (Section && Section->Characteristics & COFF::IMAGE_SCN_LNK_COMDAT
          && Aux->Selection == COFF::IMAGE_COMDAT_SELECT_ASSOCIATIVE) {
        const coff_section *Assoc;
        StringRef AssocName = "";
        std::error_code EC = Obj->getSection(AuxNumber, Assoc);
        ErrorOr<StringRef> Res = getSectionName(Obj, AuxNumber, Assoc);
        if (Res)
          AssocName = *Res;
        if (!EC)
          EC = Res.getError();
        if (EC) {
          AssocName = "";
          error(EC);
        }

        W.printNumber("AssocSection", AssocName, AuxNumber);
      }
    } else if (Symbol.isCLRToken()) {
      const coff_aux_clr_token *Aux;
      error(getSymbolAuxData(Obj, Symbol, I, Aux));

      ErrorOr<COFFSymbolRef> ReferredSym =
          Obj->getSymbol(Aux->SymbolTableIndex);
      StringRef ReferredName;
      std::error_code EC = ReferredSym.getError();
      if (EC || (EC = Obj->getSymbolName(*ReferredSym, ReferredName))) {
        ReferredName = "";
        error(EC);
      }

      DictScope AS(W, "AuxCLRToken");
      W.printNumber("AuxType", Aux->AuxType);
      W.printNumber("Reserved", Aux->Reserved);
      W.printNumber("SymbolTableIndex", ReferredName, Aux->SymbolTableIndex);

    } else {
      W.startLine() << "<unhandled auxiliary record>\n";
    }
  }
}

void COFFDumper::printUnwindInfo() {
  ListScope D(W, "UnwindInformation");
  switch (Obj->getMachine()) {
  case COFF::IMAGE_FILE_MACHINE_AMD64: {
    Win64EH::Dumper Dumper(W);
    Win64EH::Dumper::SymbolResolver
    Resolver = [](const object::coff_section *Section, uint64_t Offset,
                  SymbolRef &Symbol, void *user_data) -> std::error_code {
      COFFDumper *Dumper = reinterpret_cast<COFFDumper *>(user_data);
      return Dumper->resolveSymbol(Section, Offset, Symbol);
    };
    Win64EH::Dumper::Context Ctx(*Obj, Resolver, this);
    Dumper.printData(Ctx);
    break;
  }
  case COFF::IMAGE_FILE_MACHINE_ARMNT: {
    ARM::WinEH::Decoder Decoder(W);
    Decoder.dumpProcedureData(*Obj);
    break;
  }
  default:
    W.printEnum("unsupported Image Machine", Obj->getMachine(),
                makeArrayRef(ImageFileMachineType));
    break;
  }
}

void COFFDumper::printImportedSymbols(
    iterator_range<imported_symbol_iterator> Range) {
  for (const ImportedSymbolRef &I : Range) {
    StringRef Sym;
    error(I.getSymbolName(Sym));
    uint16_t Ordinal;
    error(I.getOrdinal(Ordinal));
    W.printNumber("Symbol", Sym, Ordinal);
  }
}

void COFFDumper::printDelayImportedSymbols(
    const DelayImportDirectoryEntryRef &I,
    iterator_range<imported_symbol_iterator> Range) {
  int Index = 0;
  for (const ImportedSymbolRef &S : Range) {
    DictScope Import(W, "Import");
    StringRef Sym;
    error(S.getSymbolName(Sym));
    uint16_t Ordinal;
    error(S.getOrdinal(Ordinal));
    W.printNumber("Symbol", Sym, Ordinal);
    uint64_t Addr;
    error(I.getImportAddress(Index++, Addr));
    W.printHex("Address", Addr);
  }
}

void COFFDumper::printCOFFImports() {
  // Regular imports
  for (const ImportDirectoryEntryRef &I : Obj->import_directories()) {
    DictScope Import(W, "Import");
    StringRef Name;
    error(I.getName(Name));
    W.printString("Name", Name);
    uint32_t Addr;
    error(I.getImportLookupTableRVA(Addr));
    W.printHex("ImportLookupTableRVA", Addr);
    error(I.getImportAddressTableRVA(Addr));
    W.printHex("ImportAddressTableRVA", Addr);
    printImportedSymbols(I.imported_symbols());
  }

  // Delay imports
  for (const DelayImportDirectoryEntryRef &I : Obj->delay_import_directories()) {
    DictScope Import(W, "DelayImport");
    StringRef Name;
    error(I.getName(Name));
    W.printString("Name", Name);
    const delay_import_directory_table_entry *Table;
    error(I.getDelayImportTable(Table));
    W.printHex("Attributes", Table->Attributes);
    W.printHex("ModuleHandle", Table->ModuleHandle);
    W.printHex("ImportAddressTable", Table->DelayImportAddressTable);
    W.printHex("ImportNameTable", Table->DelayImportNameTable);
    W.printHex("BoundDelayImportTable", Table->BoundDelayImportTable);
    W.printHex("UnloadDelayImportTable", Table->UnloadDelayImportTable);
    printDelayImportedSymbols(I, I.imported_symbols());
  }
}

void COFFDumper::printCOFFExports() {
  for (const ExportDirectoryEntryRef &E : Obj->export_directories()) {
    DictScope Export(W, "Export");

    StringRef Name;
    uint32_t Ordinal, RVA;

    error(E.getSymbolName(Name));
    error(E.getOrdinal(Ordinal));
    error(E.getExportRVA(RVA));

    W.printNumber("Ordinal", Ordinal);
    W.printString("Name", Name);
    W.printHex("RVA", RVA);
  }
}

void COFFDumper::printCOFFDirectives() {
  for (const SectionRef &Section : Obj->sections()) {
    StringRef Contents;
    StringRef Name;

    error(Section.getName(Name));
    if (Name != ".drectve")
      continue;

    error(Section.getContents(Contents));

    W.printString("Directive(s)", Contents);
  }
}

static StringRef getBaseRelocTypeName(uint8_t Type) {
  switch (Type) {
  case COFF::IMAGE_REL_BASED_ABSOLUTE: return "ABSOLUTE";
  case COFF::IMAGE_REL_BASED_HIGH: return "HIGH";
  case COFF::IMAGE_REL_BASED_LOW: return "LOW";
  case COFF::IMAGE_REL_BASED_HIGHLOW: return "HIGHLOW";
  case COFF::IMAGE_REL_BASED_HIGHADJ: return "HIGHADJ";
  case COFF::IMAGE_REL_BASED_ARM_MOV32T: return "ARM_MOV32(T)";
  case COFF::IMAGE_REL_BASED_DIR64: return "DIR64";
  default: return "unknown (" + llvm::utostr(Type) + ")";
  }
}

void COFFDumper::printCOFFBaseReloc() {
  ListScope D(W, "BaseReloc");
  for (const BaseRelocRef &I : Obj->base_relocs()) {
    uint8_t Type;
    uint32_t RVA;
    error(I.getRVA(RVA));
    error(I.getType(Type));
    DictScope Import(W, "Entry");
    W.printString("Type", getBaseRelocTypeName(Type));
    W.printHex("Address", RVA);
  }
}

void COFFDumper::printStackMap() const {
  object::SectionRef StackMapSection;
  for (auto Sec : Obj->sections()) {
    StringRef Name;
    Sec.getName(Name);
    if (Name == ".llvm_stackmaps") {
      StackMapSection = Sec;
      break;
    }
  }

  if (StackMapSection == object::SectionRef())
    return;

  StringRef StackMapContents;
  StackMapSection.getContents(StackMapContents);
  ArrayRef<uint8_t> StackMapContentsArray(
      reinterpret_cast<const uint8_t*>(StackMapContents.data()),
      StackMapContents.size());

  if (Obj->isLittleEndian())
    prettyPrintStackMap(
                      llvm::outs(),
                      StackMapV1Parser<support::little>(StackMapContentsArray));
  else
    prettyPrintStackMap(llvm::outs(),
                        StackMapV1Parser<support::big>(StackMapContentsArray));
}
