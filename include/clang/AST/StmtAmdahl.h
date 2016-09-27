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

class AmdahlForParallelStmt : public ForStmt {
  friend class ASTStmtReader;

public:
  AmdahlForParallelStmt(const ASTContext &C, Stmt *Init, Expr *Cond, 
      VarDecl *CondVar, Expr *Inc, Stmt *Body, SourceLocation FL, 
      SourceLocation LP, SourceLocation RP)
    : ForStmt(C, Init, Cond, CondVar, Inc, Body, FL, LP, RP, AmdahlForParallelStmtClass) 
  { 
    *child_begin() = Body;
  }

  /// \brief Build an empty for statement.
  explicit AmdahlForParallelStmt(EmptyShell Empty) : ForStmt(Empty) { }

  /// \brief Returns body associated with the Amdahl for.
  Stmt *getBody() const {
    return const_cast<Stmt *>(*child_begin());
  }

  static bool classof(const Stmt *T) {
    return T->getStmtClass() == AmdahlForParallelStmtClass;
  }
};

class AmdahlForCollapseStmt : public ForStmt {
  friend class ASTStmtReader;

public:
  AmdahlForCollapseStmt(const ASTContext &C, Stmt *Init, Expr *Cond, 
      VarDecl *CondVar, Expr *Inc, Stmt *Body, SourceLocation FL, 
      SourceLocation LP, SourceLocation RP)
    : ForStmt(C, Init, Cond, CondVar, Inc, Body, FL, LP, RP, AmdahlForCollapseStmtClass) 
  {
  }

  /// \brief Build an empty for statement.
  explicit AmdahlForCollapseStmt(EmptyShell Empty) : ForStmt(Empty) { }

  static bool classof(const Stmt *T) 
  {
    return T->getStmtClass() == AmdahlForCollapseStmtClass;
  }
};

}

#endif

