//===- MCCodeView.h - Machine Code CodeView support -------------*- C++ -*-===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
// Holds state from .cv_file and .cv_loc directives for later emission.
//
//===----------------------------------------------------------------------===//

#include "llvm/MC/MCCodeView.h"
#include "llvm/DebugInfo/CodeView/CodeView.h"
#include "llvm/MC/MCContext.h"
#include "llvm/MC/MCObjectStreamer.h"
#include "llvm/Support/COFF.h"

using namespace llvm;
using namespace llvm::codeview;

CodeViewContext::CodeViewContext() : StringTable(StringTableBuilder::ELF) {}

/// This is a valid number for use with .cv_loc if we've already seen a .cv_file
/// for it.
bool CodeViewContext::isValidFileNumber(unsigned FileNumber) const {
  unsigned Idx = FileNumber - 1;
  if (Idx < Filenames.size())
    return !Filenames[Idx].empty();
  return false;
}

bool CodeViewContext::addFile(unsigned FileNumber, StringRef Filename) {
  assert(FileNumber > 0);
  Filename = saveString(Filename);
  unsigned Idx = FileNumber - 1;
  if (Idx >= Filenames.size())
    Filenames.resize(Idx + 1);

  if (Filename.empty())
    Filename = "<stdin>";

  if (!Filenames[Idx].empty())
    return false;

  StringTable.add(Filename);

  Filenames[Idx] = Filename;
  return true;
}

void CodeViewContext::emitStringTable(MCObjectStreamer &OS) {
  MCContext &Ctx = OS.getContext();
  MCSymbol *StringBegin = Ctx.createTempSymbol("strtab_begin"),
           *StringEnd = Ctx.createTempSymbol("strtab_end");

  OS.EmitIntValue(unsigned(ModuleSubstreamKind::StringTable), 4);
  OS.emitAbsoluteSymbolDiff(StringEnd, StringBegin, 4);
  OS.EmitLabel(StringBegin);

  StringTable.finalizeInOrder();
  OS.EmitBytes(StringTable.data());

  OS.EmitValueToAlignment(4, 0);

  OS.EmitLabel(StringEnd);
}

void CodeViewContext::emitFileChecksums(MCObjectStreamer &OS) {
  MCContext &Ctx = OS.getContext();
  MCSymbol *FileBegin = Ctx.createTempSymbol("filechecksums_begin"),
           *FileEnd = Ctx.createTempSymbol("filechecksums_end");

  OS.EmitIntValue(unsigned(ModuleSubstreamKind::FileChecksums), 4);
  OS.emitAbsoluteSymbolDiff(FileEnd, FileBegin, 4);
  OS.EmitLabel(FileBegin);

  // Emit an array of FileChecksum entries. We index into this table using the
  // user-provided file number. Each entry is currently 8 bytes, as we don't
  // emit checksums.
  for (StringRef Filename : Filenames) {
    // A string table offset of zero is always the empty string.
    OS.EmitIntValue(Filename.empty() ? 0 : StringTable.getOffset(Filename), 4);
    // Zero the next two fields and align back to 4 bytes. This indicates that
    // no checksum is present.
    OS.EmitIntValue(0, 4);
  }

  OS.EmitLabel(FileEnd);
}

void CodeViewContext::emitLineTableForFunction(MCObjectStreamer &OS,
                                               unsigned FuncId,
                                               const MCSymbol *FuncBegin,
                                               const MCSymbol *FuncEnd) {
  MCContext &Ctx = OS.getContext();
  MCSymbol *LineBegin = Ctx.createTempSymbol("linetable_begin"),
           *LineEnd = Ctx.createTempSymbol("linetable_end");

  OS.EmitIntValue(unsigned(ModuleSubstreamKind::Lines), 4);
  OS.emitAbsoluteSymbolDiff(LineEnd, LineBegin, 4);
  OS.EmitLabel(LineBegin);
  OS.EmitCOFFSecRel32(FuncBegin);
  OS.EmitCOFFSectionIndex(FuncBegin);
  // FIXME: Emit colum records and set
  // COFF::DEBUG_LINE_TABLES_HAVE_COLUMN_RECORDS.
  OS.EmitIntValue(0, 2);
  OS.emitAbsoluteSymbolDiff(FuncEnd, FuncBegin, 4);

  // Actual line info.
  ArrayRef<MCCVLineEntry> Locs = getFunctionLineEntries(FuncId);
  for (auto I = Locs.begin(), E = Locs.end(); I != E;) {
    // Emit a file segment for the run of locations that share a file id.
    unsigned CurFileNum = I->getFileNum();
    auto FileSegEnd =
        std::find_if(I, E, [CurFileNum](const MCCVLineEntry &Loc) {
          return Loc.getFileNum() != CurFileNum;
        });
    unsigned EntryCount = FileSegEnd - I;
    OS.AddComment("Segment for file '" + Twine(Filenames[CurFileNum - 1]) +
                  "' begins");
    OS.EmitIntValue(8 * (CurFileNum - 1), 4);
    OS.EmitIntValue(EntryCount, 4);
    OS.EmitIntValue(12 + 8 * EntryCount, 4);

    for (; I != FileSegEnd; ++I) {
      OS.emitAbsoluteSymbolDiff(I->getLabel(), FuncBegin, 4);
      unsigned LineData = I->getLine();
      if (I->isStmt())
        LineData |= COFF::CVL_IsStatement;
      OS.EmitIntValue(LineData, 4);
    }
  }
  OS.EmitLabel(LineEnd);
}

//
// This is called when an instruction is assembled into the specified section
// and if there is information from the last .cv_loc directive that has yet to have
// a line entry made for it is made.
//
void MCCVLineEntry::Make(MCObjectStreamer *MCOS) {
  if (!MCOS->getContext().getCVLocSeen())
    return;

  // Create a symbol at in the current section for use in the line entry.
  MCSymbol *LineSym = MCOS->getContext().createTempSymbol();
  // Set the value of the symbol to use for the MCCVLineEntry.
  MCOS->EmitLabel(LineSym);

  // Get the current .loc info saved in the context.
  const MCCVLoc &CVLoc = MCOS->getContext().getCurrentCVLoc();

  // Create a (local) line entry with the symbol and the current .loc info.
  MCCVLineEntry LineEntry(LineSym, CVLoc);

  // clear CVLocSeen saying the current .loc info is now used.
  MCOS->getContext().clearCVLocSeen();

  // Add the line entry to this section's entries.
  MCOS->getContext().getCVContext().addLineEntry(LineEntry);
}
