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
public:
  AmdahlParallelForStmt(Stmt* ChildStmt, bool IsAmdahlMaster)
    : Stmt(AmdahlParallelForStmtClass) 
  { 
    this->IsAmdahlMaster = IsAmdahlMaster; 
    *child_begin() = ChildStmt; 
  }

  bool IsMaster() { return IsAmdahlMaster; } 
  Stmt* getChildStmt() const { return const_cast<Stmt *>(*child_begin()); }

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

  child_range children();
};

class AmdahlCollapseForStmt : public Stmt {
  friend class ASTStmtReader;

public:
  AmdahlCollapseForStmt(Stmt* ChildStmt)
    : Stmt(AmdahlCollapseForStmtClass) 
  { 
    *child_begin() = ChildStmt;
  }

  Stmt* getChildStmt() const { return const_cast<Stmt *>(*child_begin()); }

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

  child_range children();
};
}

#endif

