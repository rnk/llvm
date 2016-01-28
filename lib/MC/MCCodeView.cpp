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
#include "llvm/MC/MCContext.h"
#include "llvm/MC/MCObjectStreamer.h"
#include "llvm/Support/COFF.h"

using namespace llvm;

CodeViewContext::CodeViewContext() {}

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
  unsigned Idx = FileNumber - 1;
  if (Idx >= Filenames.size())
    Filenames.resize(Idx + 1);

  if (Filename.empty())
    Filename = "<stdin>";

  if (!Filenames[Idx].empty())
    return false;

  Filenames[Idx] = Filename;
  return true;
}

static void EmitLabelDiff(MCStreamer &Streamer, const MCSymbol *From,
                          const MCSymbol *To, unsigned int Size = 4) {
  MCSymbolRefExpr::VariantKind Variant = MCSymbolRefExpr::VK_None;
  MCContext &Context = Streamer.getContext();
  const MCExpr *FromRef = MCSymbolRefExpr::create(From, Variant, Context),
               *ToRef   = MCSymbolRefExpr::create(To, Variant, Context);
  const MCExpr *AddrDelta =
      MCBinaryExpr::create(MCBinaryExpr::Sub, ToRef, FromRef, Context);
  Streamer.EmitValue(AddrDelta, Size);
}

void CodeViewContext::emitLineTableForFunction(MCObjectStreamer &OS,
                                               unsigned FuncId,
                                               const MCSymbol *FuncBegin,
                                               const MCSymbol *FuncEnd) {
  OS.EmitCOFFSecRel32(FuncBegin);
  OS.EmitCOFFSectionIndex(FuncBegin);
  // FIXME: Emit colum records and set COFF::DEBUG_LINE_TABLES_HAVE_COLUMN_RECORDS.
  OS.EmitIntValue(0, 2);
  EmitLabelDiff(OS, FuncBegin, FuncEnd);

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
    OS.EmitIntValue(8 * (CurFileNum - 1), 4); // FIXME: index in file table
    OS.EmitIntValue(EntryCount, 4);
    OS.EmitIntValue(12 + 8 * EntryCount, 4);

    for (; I != FileSegEnd; ++I) {
      EmitLabelDiff(OS, FuncBegin, I->getLabel());
      unsigned LineData = I->getLine();
      if (I->isStmt())
        LineData |= COFF::CVL_IsStatement;
      OS.EmitIntValue(LineData, 4);
    }
  }
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
