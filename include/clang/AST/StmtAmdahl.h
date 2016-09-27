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
#include "clang/AST/Expr.h"
#include "clang/AST/OpenMPClause.h"
#include "clang/AST/Stmt.h"
#include "clang/Basic/OpenMPKinds.h"
#include "clang/Basic/SourceLocation.h"

namespace clang {

class AmdahlParallelForStmt : public ForStmt {
  friend class ASTStmtReader;

public:
  AmdahlParallelForStmt(const ForStmt& ChildForStmt, const ASTContext &C)
    : ForStmt(ChildForStmt, C, AmdahlParallelForStmtClass) 
  { 
  }

  /// \brief Build an empty for statement.
  explicit AmdahlParallelForStmt(EmptyShell Empty) : ForStmt(Empty) { }

  static bool classof(const Stmt *T) {
    return T->getStmtClass() == AmdahlParallelForStmtClass;
  }
};

class AmdahlCollapseForStmt : public ForStmt {
  friend class ASTStmtReader;

public:
  AmdahlCollapseForStmt(const ForStmt& ChildForStmt, const ASTContext &C)
    : ForStmt(ChildForStmt, C, AmdahlCollapseForStmtClass) 
  { 
  }

  /// \brief Build an empty for statement.
  explicit AmdahlCollapseForStmt(EmptyShell Empty) : ForStmt(Empty) { }

  static bool classof(const Stmt *T) {
    return T->getStmtClass() == AmdahlCollapseForStmtClass;
  }
};

}

#endif

