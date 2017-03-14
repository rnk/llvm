//===-- DerivedUser.h - Base for non-IR Users -------------------*- C++ -*-===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
// This file declares the DerivedUser class, which is an extension point for the
// Value class hierarchy. Generally speaking, Value is the base of a closed
// class hierarchy that can't be extended by code outside of lib/IR. This class
// creates a loophole that allows classes outside of lib/IR to extend User to
// leverage its use/def list machinery.
//
//===----------------------------------------------------------------------===//

#ifndef LLVM_IR_DERIVEDUSER_H
#define LLVM_IR_DERIVEDUSER_H

#include "llvm/IR/User.h"

namespace llvm {

class ValueCallbacks;

/// Extension point for the Value hierarchy. All classes outside of lib/IR
/// that wish to inherit from User should instead inherit from DerivedUser
/// instead.
class DerivedUser : public User {
  friend Value;
  ValueCallbacks *VCallbacks;

public:
  DerivedUser(Type *Ty, unsigned VK, Use *U, unsigned NumOps,
              ValueCallbacks *VC)
      : User(Ty, VK, U, NumOps), VCallbacks(VC) {}
};

/// Interface implemented for DerivedUser subclasses implemented outside of
/// lib/IR.
class ValueCallbacks {
public:
  virtual ~ValueCallbacks(); // Satisfy -Wweak-vtables and -Wnon-virtual-dtor.
  virtual void deleteValue(Value *DV) = 0;
};

} // namespace llvm

#endif // LLVM_IR_DERIVEDUSER_H
