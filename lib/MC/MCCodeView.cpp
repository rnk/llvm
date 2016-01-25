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

using namespace llvm;

CodeViewContext::CodeViewContext() {}

/// This is a valid number for use with .cv_file if it has not yet been used.
bool CodeViewContext::isValidFileNumber(unsigned FileNumber) const {
  return FileNumber > 0 &&
         (FileNumber - 1 >= Filenames.size() ||
          (FileNumber - 1 < Filenames.size() && Filenames[FileNumber - 1].empty()));
}

bool CodeViewContext::addFile(unsigned FileNumber, StringRef Filename) {
  unsigned Idx = FileNumber - 1;
  if (Idx >= Filenames.size())
    Filenames.resize(Idx + 1);
  if (!Filenames[Idx].empty())
    return false;
  Filenames[Idx] = Filename;
  return true;
}

//
// This is called when an instruction is assembled into the specified section
// and if there is information from the last .cv_loc directive that has yet to have
// a line entry made for it is made.
//
void MCCVLineEntry::Make(MCObjectStreamer *MCOS, MCSection *Section) {
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
#if 0
  MCOS->getContext()
      .getMCCVLineTable(MCOS->getContext().getCVCompileUnitID())
      .getMCLineSections()
      .addLineEntry(LineEntry, Section);
#endif
}
