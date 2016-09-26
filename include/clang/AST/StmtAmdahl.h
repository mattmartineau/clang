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
  public:
  AmdahlForParallelStmt(const ASTContext &C, Stmt *Init, Expr *Cond, 
      VarDecl *CondVar, Expr *Inc, Stmt *Body, SourceLocation FL, 
      SourceLocation LP, SourceLocation RP)
    : ForStmt(C, Init, Cond, CondVar, Inc, Body, FL, LP, RP, AmdahlForParallelStmtClass) {}

  //static bool classof(const Stmt *T) {
  //  return T->getStmtClass() == AmdahlForParallelStmtClass;
  //}
};

class AmdahlForCollapseStmt : public ForStmt {
  public:
  AmdahlForCollapseStmt(const ASTContext &C, Stmt *Init, Expr *Cond, 
      VarDecl *CondVar, Expr *Inc, Stmt *Body, SourceLocation FL, 
      SourceLocation LP, SourceLocation RP)
    : ForStmt(C, Init, Cond, CondVar, Inc, Body, FL, LP, RP, AmdahlForCollapseStmtClass) {}
};

}

#endif

