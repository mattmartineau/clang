//===--- SemaAmdahl.cpp - Semantic Analysis for Amdahl constructs ---------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
/// \file
/// \brief This file implements semantic analysis for Amdahl directives and
/// clauses.
///
//===----------------------------------------------------------------------===//

#include "clang/AST/StmtAmdahl.h"
#include "clang/Sema/Sema.h"

using namespace clang;

StmtResult Sema::ActOnAmdahlParallelForStmt(SourceLocation ForLoc,
                              SourceLocation LParenLoc,
                              Stmt *First,
                              ConditionResult Second,
                              FullExprArg third,
                              SourceLocation RParenLoc,
                              Stmt *Body)
{
  Expr *Third  = third.get();  
  ActOnForStmt(ForLoc, LParenLoc, First, Second, third, RParenLoc, Body);

  return new (Context)
      AmdahlParallelForStmt(Context, First, Second.get().second, Second.get().first, Third,
              Body, ForLoc, LParenLoc, RParenLoc);
}

StmtResult Sema::ActOnAmdahlCollapseForStmt(SourceLocation ForLoc,
                              SourceLocation LParenLoc,
                              Stmt *First,
                              ConditionResult Second,
                              FullExprArg third,
                              SourceLocation RParenLoc,
                              Stmt *Body)
{
  ActOnForStmt(ForLoc, LParenLoc, First, Second, third, RParenLoc, Body);

  Expr *Third  = third.get();
  return new (Context)
      AmdahlCollapseForStmt(Context, First, Second.get().second, Second.get().first, Third,
              Body, ForLoc, LParenLoc, RParenLoc);
}

