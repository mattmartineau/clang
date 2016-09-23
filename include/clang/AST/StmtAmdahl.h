//===- StmtAmdahl.h - Classes for Amdahl directives  ------------*- C++ -*-===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
/// \file
/// \brief This file defines Amdahl AST classes for executable directives and
/// clauses.
///
//===----------------------------------------------------------------------===//

#ifndef LLVM_CLANG_AST_STMTAMDAHL_H
#define LLVM_CLANG_AST_STMTAMDAHL_H

#include "clang/AST/Stmt.h"

namespace clang {

class AmdahlForParallelStmt : public ForStmt {
  AmdahlForParallelStmt(ForStmt Parent) : ForStmt(Parent) { }
  explicit AmdahlForParallelStmt(EmptyShell Empty) : ForStmt(Empty) { }
};

class AmdahlForCollapseStmt : public ForStmt {
  AmdahlForCollapseStmt(ForStmt Parent) : ForStmt(Parent) { }
  explicit AmdahlForCollapseStmt(EmptyShell Empty) : ForStmt(Empty) { }
};

}

#endif

