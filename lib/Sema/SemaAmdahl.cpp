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

#include "TreeTransform.h"
#include "clang/AST/ASTContext.h"
#include "clang/AST/ASTMutationListener.h"
#include "clang/AST/CXXInheritance.h"
#include "clang/AST/Decl.h"
#include "clang/AST/DeclCXX.h"
#include "clang/AST/DeclAmdahl.h"
#include "clang/AST/StmtCXX.h"
#include "clang/AST/StmtAmdahl.h"
#include "clang/AST/StmtVisitor.h"
#include "clang/AST/TypeOrdering.h"
#include "clang/Basic/AmdahlKinds.h"
#include "clang/Basic/TargetInfo.h"
#include "clang/Lex/Preprocessor.h"
#include "clang/Sema/Initialization.h"
#include "clang/Sema/Lookup.h"
#include "clang/Sema/Scope.h"
#include "clang/Sema/ScopeInfo.h"
#include "clang/Sema/SemaInternal.h"
using namespace clang;

StmtResult ActOnAmdahlForParallelStmt(SourceLocation ForLoc,
                              SourceLocation LParenLoc,
                              Stmt *First,
                              ConditionResult Second,
                              FullExprArg Third,
                              SourceLocation RParenLoc,
                              Stmt *Body)
{
  auto ForStmt = ActOnForStmt(
      ForLoc, LParenLoc, First, Second, Third, RParenLoc, Body);
  return AmdahlForParallelStmt(ForStmt);
}

StmtResult ActOnAmdahlForCollapseStmt(SourceLocation ForLoc,
                              SourceLocation LParenLoc,
                              Stmt *First,
                              ConditionResult Second,
                              FullExprArg Third,
                              SourceLocation RParenLoc,
                              Stmt *Body)
{
  auto ForStmt = ActOnForStmt(
      ForLoc, LParenLoc, First, Second, Third, RParenLoc, Body);
  return AmdahlForCollapseStmt(ForStmt);
}

