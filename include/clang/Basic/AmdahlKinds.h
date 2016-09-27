//===--- AmdahlKinds.h - Amdahl enums ---------------------------*- C++ -*-===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
///
/// \file
/// \brief Defines some Amdahl-specific enums and functions.
///
//===----------------------------------------------------------------------===//

#ifndef LLVM_CLANG_BASIC_AMDAHLKINDS_H
#define LLVM_CLANG_BASIC_AMDAHLKINDS_H

namespace clang {
  enum AmdahlForKind {
    AmdahlParallelFor,
    AmdahlCollapseFor,
    AmdahlForSequential
  };
}

#endif

