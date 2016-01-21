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

/// Holds state from .cv_file and .cv_loc directives for later emission.
class CodeViewContext {
public:
  CodeViewContext();

  /// An array of absolute paths. Eventually this may include the file checksum.
  SmallVector<StringRef, 4> Filenames;

  bool isValidFileNumer(unsigned FileNumber);
  bool addFile(unsigned FileNumber, StringRef Filename);
  ArrayRef<StringRef> getFilenames() { return Filenames; }
};

} // end namespace llvm
#endif
