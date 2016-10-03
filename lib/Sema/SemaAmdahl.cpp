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
#include "clang/AST/OperationKinds.h"

using namespace clang;

StmtResult Sema::ActOnAmdahlParallelForStmt(
    ForStmt* ChildFor, Scope* CompoundScope, const int AmdahlNestLevel)
{
  return new (Context) AmdahlParallelForStmt(
      HandleCurrentNestingLevel(ChildFor, CompoundScope, AmdahlNestLevel), AmdahlNestLevel);
}

StmtResult Sema::ActOnAmdahlCollapseForStmt(
    ForStmt* ChildFor, Scope* CompoundScope, const int AmdahlNestLevel)
{
  return new (Context) AmdahlCollapseForStmt(
      HandleCurrentNestingLevel(ChildFor, CompoundScope, AmdahlNestLevel), AmdahlNestLevel);
}

Stmt* Sema::HandleCurrentNestingLevel(
    ForStmt* ChildFor, Scope* CompoundScope, const int AmdahlNestLevel)
{
  auto ChildForCond = cast<BinaryOperator>(ChildFor->getCond());
  auto ChildForInit = ChildFor->getInit();
  auto ChildForInc = ChildFor->getInc();

  // Only allow <, >, <=, >= on Amdahl loops.
  auto ChildForBinaryOperator = cast<BinaryOperator>(ChildForCond);
  auto ChildForBinOpKind = ChildForBinaryOperator->getOpcode();
  assert((ChildForBinOpKind == BinaryOperatorKind::BO_LT || 
        ChildForBinOpKind == BinaryOperatorKind::BO_GT ||
        ChildForBinOpKind == BinaryOperatorKind::BO_LE || 
        ChildForBinOpKind == BinaryOperatorKind::BO_GE) &&
      "The binary operator for an Amdahl pfor or cfor loop must be <, >, <=, >=.");

  return ChildFor;
}

