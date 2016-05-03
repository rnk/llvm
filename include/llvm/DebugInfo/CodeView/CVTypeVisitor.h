//===- CVTypeVisitor.h ------------------------------------------*- C++ -*-===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

#ifndef LLVM_DEBUGINFO_CODEVIEW_CVTYPEVISITOR_H
#define LLVM_DEBUGINFO_CODEVIEW_CVTYPEVISITOR_H

#include "llvm/DebugInfo/CodeView/CodeView.h"
#include "llvm/DebugInfo/CodeView/TypeIndex.h"
#include "llvm/DebugInfo/CodeView/TypeRecord.h"

namespace llvm {
namespace codeview {

template <typename Derived>
class CVTypeVisitor {
public:
  CVTypeVisitor() {}

  bool hadError() const { return HadError; }

  template <typename T>
  bool consumeObject(ArrayRef<uint8_t> &Data, const T *&Res) {
    if (Data.size() < sizeof(*Res)) {
      HadError = true;
      return false;
    }
    Res = reinterpret_cast<const T *>(Data.data());
    Data = Data.drop_front(sizeof(*Res));
    return true;
  }

  /// Action to take on known types. By default, does nothing.
#define TYPE_RECORD(ClassName, LeafEnum)                                       \
  void visit##ClassName(TypeLeafKind LeafType, const ClassName *Record,        \
                        StringRef LeafData) {}
#define TYPE_RECORD_ALIAS(ClassName, LeafEnum)
#define MEMBER_RECORD_ALIAS(ClassName, LeafEnum)
#include "TypeRecords.def"

  /// Visits the type records in Data, and returns remaining data. Sets the
  /// error flag on parse failures.
  ArrayRef<uint8_t> visit(ArrayRef<uint8_t> Data) {
    while (!Data.empty()) {
      const TypeRecordPrefix *Prefix;
      if (!consumeObject(Data, Prefix))
        return Data;
      size_t RecLen = Prefix->Len - 2;
      ArrayRef<uint8_t> LeafData = Data.slice(0, RecLen);
      TypeLeafKind Leaf = TypeLeafKind(unsigned(Prefix->Leaf));
      switch (Leaf) {
        // FIXME: Templatize to avoid macros.
#define TYPE_RECORD(ClassName, LeafEnum)                                       \
  case LeafEnum: {                                                             \
    const ClassName *Rec;                                                      \
    if (!consumeObject(LeafData, Rec))                                         \
      return Data;                                                             \
    StringRef RemainingBytes(reinterpret_cast<const char *>(LeafData.data()),  \
                             LeafData.size());                                 \
    static_cast<Derived *>(this)->visit##ClassName(                            \
        Leaf, Rec, RemainingBytes.drop_front(sizeof(*Rec)));                   \
    break;                                                                     \
  }
#include "TypeRecords.def"
      default:
        Derived::visitUnknownType(Prefix);
        break;
      }

      // The next record starts immediately after this one.
      Data = Data.drop_front(RecLen);

      // FIXME: Documentation suggests that one day we may need to ignore LF_PAD
      // bytes here. All tested versions of MSVC include LF_PAD bytes in
      // LeafData, though.
    }
    return Data;
  }

  /// Action to take on unknown types. By default, they are ignored.
  void visitUnknownType(const TypeRecordPrefix *Prefix) {}

private:
  void parseError() { HadError = true; };

  /// Whether a type stream parsing error was encountered.
  bool HadError;
};

} // codeview
} // llvm

#endif // LLVM_DEBUGINFO_CODEVIEW_CVTYPEVISITOR_H
