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
    ForStmt* OuterForStmt, Scope* CompoundScope)
{
  HandleAmdahlNesting(OuterForStmt, CompoundScope);
  return new (Context) AmdahlParallelForStmt(*OuterForStmt, Context);
}

StmtResult Sema::ActOnAmdahlCollapseForStmt(
    ForStmt* OuterForStmt, Scope* CompoundScope)
{
  HandleAmdahlNesting(OuterForStmt, CompoundScope);
  return new (Context) AmdahlCollapseForStmt(*OuterForStmt, Context);
}

void Sema::HandleAmdahlNesting(
    ForStmt* OuterForStmt, Scope* CompoundScope)
{
  // Check for closely nested Amdahl loop.
  auto ImmediateDescendent = cast<CompoundStmt>(OuterForStmt->getBody())->body_front();

  if(!isa<AmdahlParallelForStmt>(ImmediateDescendent) && 
     !isa<AmdahlCollapseForStmt>(ImmediateDescendent)) {
    // The immediate descendent is some serial operation, capture the body.
    QualType KmpInt32Ty = Context.getIntTypeForBitwidth(32, 1);
    Sema::CapturedParamNameType Params[] = {
      std::make_pair(".amdahl_id.", KmpInt32Ty),
      std::make_pair(StringRef(), QualType()) // __context with shared vars
    };
    Stmt* Body = OuterForStmt->getBody();
    ActOnCapturedRegionStart(Body->getLocStart(), CompoundScope, CR_Default, Params);
    auto CapturedBodyAction = ActOnCapturedRegionEnd(Body);
    OuterForStmt->setBody(CapturedBodyAction.get());
    return;
  }

  // Cast the descendent to a ForStmt (InnerFor).
  auto InnerFor = (ForStmt*)ImmediateDescendent;
  auto InnerForCond = cast<BinaryOperator>(InnerFor->getCond());
  auto InnerForInit = InnerFor->getInit();
  auto InnerForInc = InnerFor->getInc();

  // Only allow <, >, <=, >= on Amdahl loops.
  auto InnerForBinaryOperator = cast<BinaryOperator>(InnerForCond);
  auto InnerForBinOpKind = InnerForBinaryOperator->getOpcode();
  assert((InnerForBinOpKind == BinaryOperatorKind::BO_LT || 
          InnerForBinOpKind == BinaryOperatorKind::BO_GT ||
          InnerForBinOpKind == BinaryOperatorKind::BO_LE || 
          InnerForBinOpKind == BinaryOperatorKind::BO_GE) &&
      "The binary operator for an Amdahl pfor or cfor loop must be <, >, <=, >=.");

  if(isa<AmdahlCollapseForStmt>(InnerFor)) {
    // Fetch the cond var of the OuterForStmt.
    auto OuterForCond = cast<BinaryOperator>(OuterForStmt->getCond());
    auto OuterForInit = OuterForStmt->getInit();
    auto OuterForInc = OuterForStmt->getInc();

    auto Scope = nullptr;
    auto CombinedUpper = BuildBinOp(Scope, OuterForCond->getExprLoc(), 
                                    BinaryOperatorKind::BO_Mul, InnerForCond->getRHS(), 
                                    OuterForCond->getRHS()).get();
    OuterForStmt->setCond(BuildBinOp(Scope, OuterForCond->getExprLoc(),
                         InnerForCond->getOpcode(), InnerForCond->getLHS(), CombinedUpper).get());
    OuterForStmt->dump();

    // Finished stealing the iteration space, so cut the statement out.
    OuterForStmt->setBody(InnerFor->getBody());
  }
  else if (isa<AmdahlParallelForStmt>(InnerFor)) {
    // We're going to shape the parallelism, whatever that means.
    // For now just disallow.
    llvm_unreachable("Amdahl Parallel For nesting is disallowed for now.");
    OuterForStmt->setBody(InnerFor->getBody());
  }
  else {
    llvm_unreachable("expected an Amdahl statement");
  }
}

