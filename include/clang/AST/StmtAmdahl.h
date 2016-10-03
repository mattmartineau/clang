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

class AmdahlParallelForStmt : public Stmt {
  friend class ASTStmtReader;
  bool IsAmdahlMaster;
  int AmdahlNestLevel;

  Stmt* ChildStmt;
public:
  AmdahlParallelForStmt(Stmt* ChildStmt, const int AmdahlNestLevel)
    : Stmt(AmdahlParallelForStmtClass) { 
    this->IsAmdahlMaster = (AmdahlNestLevel == 0); 
    this->ChildStmt = ChildStmt; 
    this->AmdahlNestLevel = AmdahlNestLevel;
  }

  bool IsMaster() { return IsAmdahlMaster; } 
  Stmt* getChildStmt() const { return ChildStmt; }

  /// \brief Build an empty for statement.
  explicit AmdahlParallelForStmt(EmptyShell Empty) 
    : Stmt(AmdahlParallelForStmtClass, Empty) { }

  SourceLocation getLocStart() const LLVM_READONLY { 
    return getChildStmt()->getLocStart();
  }

  SourceLocation getLocEnd() const LLVM_READONLY {
    return getChildStmt()->getLocEnd();
  }

  static bool classof(const Stmt *T) {
    return T->getStmtClass() == AmdahlParallelForStmtClass;
  }

  child_range children() {
    return child_range(&ChildStmt, &ChildStmt + 1);
  }
};

class AmdahlCollapseForStmt : public Stmt {
  friend class ASTStmtReader;

  Stmt* ChildStmt;
  int AmdahlNestLevel;
public:
  AmdahlCollapseForStmt(Stmt* ChildStmt, const int AmdahlNestLevel)
    : Stmt(AmdahlCollapseForStmtClass) { 
    this->ChildStmt = ChildStmt; 
    this->AmdahlNestLevel = AmdahlNestLevel;
  }


  Stmt* getChildStmt() const { return ChildStmt; }

  /// \brief Build an empty for statement.
  explicit AmdahlCollapseForStmt(EmptyShell Empty) 
    : Stmt(AmdahlCollapseForStmtClass, Empty) { }

  SourceLocation getLocStart() const LLVM_READONLY { 
    return getChildStmt()->getLocStart();
  }

  SourceLocation getLocEnd() const LLVM_READONLY {
    return getChildStmt()->getLocEnd();
  }

  static bool classof(const Stmt *T) {
    return T->getStmtClass() == AmdahlCollapseForStmtClass;
  }

  child_range children() {
    return child_range(&ChildStmt, &ChildStmt + 1);
  }
};
}

#endif

