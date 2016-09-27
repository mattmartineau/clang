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
    ForStmt* BaseForStmt, Scope* CompoundScope)
{
  HandleAmdahlNesting(BaseForStmt, CompoundScope);
  return new (Context) AmdahlParallelForStmt(*BaseForStmt, Context);
}

StmtResult Sema::ActOnAmdahlCollapseForStmt(
    ForStmt* BaseForStmt, Scope* CompoundScope)
{
  HandleAmdahlNesting(BaseForStmt, CompoundScope);
  return new (Context) AmdahlCollapseForStmt(*BaseForStmt, Context);
}

void Sema::HandleAmdahlNesting(
    ForStmt* BaseForStmt, Scope* CompoundScope)
{
  // Check for closely nested Amdahl loop.
  auto ImmediateDescendent = cast<CompoundStmt>(BaseForStmt->getBody())->body_front();

  if(!isa<AmdahlParallelForStmt>(ImmediateDescendent) && 
     !isa<AmdahlCollapseForStmt>(ImmediateDescendent)) {
    // The immediate descendent is some serial operation, capture the body.
    QualType KmpInt32Ty = Context.getIntTypeForBitwidth(32, 1);
    Sema::CapturedParamNameType Params[] = {
      std::make_pair(".amdahl_id.", KmpInt32Ty),
      std::make_pair(StringRef(), QualType()) // __context with shared vars
    };
    Stmt* Body = BaseForStmt->getBody();
    ActOnCapturedRegionStart(Body->getLocStart(), CompoundScope, CR_Default, Params);
    auto CapturedBodyAction = ActOnCapturedRegionEnd(Body);
    BaseForStmt->setBody(CapturedBodyAction.get());
    return;
  }

  // Cast the descendent to a ForStmt (FD).
  auto FD = (ForStmt*)ImmediateDescendent;
  auto FDCond = FD->getCond();
  auto FDInit = FD->getInit();
  auto FDInc = FD->getInc();

  // Only allow <, >, <=, >= on Amdahl loops.
  auto FDBinaryOperator = cast<BinaryOperator>(FDCond);
  auto FDBinOpKind = FDBinaryOperator->getOpcode();
  assert((FDBinOpKind == BinaryOperatorKind::BO_LT || 
          FDBinOpKind == BinaryOperatorKind::BO_GT ||
          FDBinOpKind == BinaryOperatorKind::BO_LE || 
          FDBinOpKind == BinaryOperatorKind::BO_GE) &&
      "The binary operator for an Amdahl pfor or cfor loop must be <, >, <=, >=.");

  if(isa<AmdahlCollapseForStmt>(FD)) {
    // Fetch the cond var of the BaseForStmt.
    auto BaseForCond = BaseForStmt->getCond();
    auto BaseForInit = BaseForStmt->getInit();
    auto BaseForInc = BaseForStmt->getInc();

    // Finished stealing the iteration space, so cut the statement out.
    BaseForStmt->setBody(FD->getBody());
  }
  else if (isa<AmdahlParallelForStmt>(FD)) {
    // We're going to shape the parallelism, whatever that means.
    BaseForStmt->setBody(FD->getBody());
  }
  else {
    llvm_unreachable("expected an Amdahl statement");
  }
}

