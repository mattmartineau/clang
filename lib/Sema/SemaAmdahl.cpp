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
    ForStmt* ChildFor, Scope* CompoundScope, const int AmdahlNestingLevel)
{
  bool IsMaster = (AmdahlNestingLevel == 1);
  return new (Context) AmdahlParallelForStmt(
      HandleCurrentNestingLevel(ChildFor, CompoundScope, IsMaster), IsMaster);
}

StmtResult Sema::ActOnAmdahlCollapseForStmt(
    ForStmt* ChildFor, Scope* CompoundScope, const int AmdahlNestingLevel)
{
  return new (Context) AmdahlCollapseForStmt(
      HandleCurrentNestingLevel(ChildFor, CompoundScope, AmdahlNestingLevel));
}

Stmt* Sema::HandleCurrentNestingLevel(
    ForStmt* ChildFor, Scope* CompoundScope, bool IsMaster)
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

  // We are the master Amdahl Parallel loop so capture here.
  if(IsMaster) {
    assert((isa<AmdahlParallelForStmt>(ChildFor)) &&
        "Can only nest Amdahl loops within an Amdahl Parallel loop (pfor).");

    QualType KmpInt32Ty = Context.getIntTypeForBitwidth(32, 1);
    Sema::CapturedParamNameType Params[] = {
      std::make_pair(".amdahl_id.", KmpInt32Ty),
      std::make_pair(StringRef(), QualType()) // __context with shared vars
    };
    ActOnCapturedRegionStart(ChildFor->getLocStart(), CompoundScope, CR_Default, Params);
    return ActOnCapturedRegionEnd(ChildFor).get();
  }

  /// Here we will analyse the actual child For...

#if 0
  if(isa<AmdahlCollapseForStmt>(ChildFor)) {
    // Fetch the cond var of the ChildFor.
    auto OuterForCond = cast<BinaryOperator>(ChildFor->getCond());
    auto OuterForInit = ChildFor->getInit();
    auto OuterForInc = ChildFor->getInc();

    auto Scope = nullptr;
    auto CombinedUpper = BuildBinOp(Scope, OuterForCond->getExprLoc(), 
        BinaryOperatorKind::BO_Mul, ChildForCond->getRHS(), 
        OuterForCond->getRHS()).get();
    ChildFor->setCond(BuildBinOp(Scope, OuterForCond->getExprLoc(),
          ChildForCond->getOpcode(), ChildForCond->getLHS(), CombinedUpper).get());
    ChildFor->dump();

    // Finished stealing the iteration space, so cut the statement out.
    ChildFor->setBody(ChildFor->getBody());
  }
  else if (isa<AmdahlParallelForStmt>(ChildFor)) {
    // We're going to shape the parallelism, whatever that means.
    // For now just disallow.
    llvm_unreachable("Amdahl Parallel For nesting is disallowed for now.");
    ChildFor->setBody(ChildFor->getBody());
  }
  else {
    llvm_unreachable("expected an Amdahl statement");
  }
#endif // if 0
}

