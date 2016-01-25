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

#ifndef LLVM_MC_MCCODEVIEW_H
#define LLVM_MC_MCCODEVIEW_H

#include "llvm/ADT/StringRef.h"
#include "llvm/MC/MCSection.h"

namespace llvm {
class MCContext;
class MCObjectStreamer;
class MCStreamer;

/// \brief Instances of this class represent the information from a
/// .cv_loc directive.
class MCCVLoc {
  uint32_t FunctionId;
  uint32_t FileNum;
  uint32_t Line;
  uint16_t Column;
  uint16_t PrologueEnd : 1;
  uint16_t IsStmt : 1;

private: // MCContext manages these
  friend class MCContext;
  MCCVLoc(unsigned functionid, unsigned fileNum, unsigned line, unsigned column,
          bool prologueend, bool isstmt)
      : FunctionId(functionid), FileNum(fileNum), Line(line), Column(column),
        PrologueEnd(prologueend), IsStmt(isstmt) {}

  // Allow the default copy constructor and assignment operator to be used
  // for an MCCVLoc object.

public:
  unsigned getFunctionId() const { return FunctionId; }

  /// \brief Get the FileNum of this MCCVLoc.
  unsigned getFileNum() const { return FileNum; }

  /// \brief Get the Line of this MCCVLoc.
  unsigned getLine() const { return Line; }

  /// \brief Get the Column of this MCCVLoc.
  unsigned getColumn() const { return Column; }

  bool isPrologueEnd() const { return PrologueEnd; }
  bool isStmt() const { return IsStmt; }

  void setFunctionId(unsigned FID) { FunctionId = FID; }

  /// \brief Set the FileNum of this MCCVLoc.
  void setFileNum(unsigned fileNum) { FileNum = fileNum; }

  /// \brief Set the Line of this MCCVLoc.
  void setLine(unsigned line) { Line = line; }

  /// \brief Set the Column of this MCCVLoc.
  void setColumn(unsigned column) {
    assert(column <= UINT16_MAX);
    Column = column;
  }

  void setPrologueEnd(bool PE) { PrologueEnd = PE; }
  void setIsStmt(bool IS) { IsStmt = IS; }
};

/// \brief Instances of this class represent the line information for
/// the CodeView line table entries.  Which is created after a machine
/// instruction is assembled and uses an address from a temporary label
/// created at the current address in the current section and the info from
/// the last .cv_loc directive seen as stored in the context.
class MCCVLineEntry : public MCCVLoc {
  MCSymbol *Label;

private:
  // Allow the default copy constructor and assignment operator to be used
  // for an MCCVLineEntry object.

public:
  // Constructor to create an MCCVLineEntry given a symbol and the dwarf loc.
  MCCVLineEntry(MCSymbol *label, const MCCVLoc loc)
      : MCCVLoc(loc), Label(label) {}

  MCSymbol *getLabel() const { return Label; }

  // This is called when an instruction is assembled into the specified
  // section and if there is information from the last .loc directive that
  // has yet to have a line entry made for it is made.
  static void Make(MCObjectStreamer *MCOS, MCSection *Section);
};

/// Holds state from .cv_file and .cv_loc directives for later emission.
class CodeViewContext {
public:
  CodeViewContext();

  /// An array of absolute paths. Eventually this may include the file checksum.
  SmallVector<StringRef, 4> Filenames;

  bool isValidFileNumber(unsigned FileNumber) const;
  bool addFile(unsigned FileNumber, StringRef Filename);
  ArrayRef<StringRef> getFilenames() { return Filenames; }
};

} // end namespace llvm
#endif
