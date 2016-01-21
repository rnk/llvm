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

using namespace llvm;

CodeViewContext::CodeViewContext() {}

/// This is a valid number for use with .cv_file if it has not yet been used.
bool CodeViewContext::isValidFileNumer(unsigned FileNumber) {
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
