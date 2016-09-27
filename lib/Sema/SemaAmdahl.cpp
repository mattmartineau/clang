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

StmtResult Sema::ActOnAmdahlParallelForStmt(
    ForStmt* BaseForStmt, Scope* CompoundScope, bool* CapturedBody)
{
  // Check for closely nested Amdahl loop.
  auto ImmediateDescendent = BaseForStmt->getBody();
  if(isa<AmdahlCollapseForStmt>(ImmediateDescendent)) {
    // We're going to steal it's iteration space.
  }
  else if (isa<AmdahlParallelForStmt>(ImmediateDescendent)) {
    // We're going to shape the parallelism, whatever that means.
  }

  // Check if we are the inner most loop.
  if(!(*CapturedBody)) {
    // We have processed all of the parallel loops, capture the body.
    QualType KmpInt32Ty = Context.getIntTypeForBitwidth(32, 1);
    Sema::CapturedParamNameType Params[] = {
      std::make_pair(".amdahl_id.", KmpInt32Ty),
      std::make_pair(StringRef(), QualType()) // __context with shared vars
    };
    Stmt* Body = BaseForStmt->getBody();
    ActOnCapturedRegionStart(Body->getLocStart(), CompoundScope, CR_Default, Params);
    auto CapturedBodyAction = ActOnCapturedRegionEnd(Body);
    BaseForStmt->setBody(CapturedBodyAction.get());
    *CapturedBody = true;
  }

  return new (Context) AmdahlParallelForStmt(*BaseForStmt, Context);
}

StmtResult Sema::ActOnAmdahlCollapseForStmt(ForStmt* BaseForStmt)
{
  return new (Context) AmdahlCollapseForStmt(*BaseForStmt, Context);
}

