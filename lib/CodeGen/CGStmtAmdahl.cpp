//===--- CGStmtAmdahl.cpp - Emit LLVM Code from Statements ----------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
// This contains code to emit Amdahl nodes as LLVM code.
//
//===----------------------------------------------------------------------===//

#include "CodeGenFunction.h"

using namespace clang;
using namespace CodeGen;

void CodeGenFunction::EmitAmdahlForStmt(const ForStmt &S,
                                  ArrayRef<const Attr *> ForAttrs) {
  JumpDest LoopExit = getJumpDestInCurrentScope("for.end");

  LexicalScope ForScope(*this, S.getSourceRange());

  llvm::DebugLoc DL = Builder.getCurrentDebugLocation();

  // Evaluate the first part before the loop.
  if (S.getInit())
    EmitStmt(S.getInit());

  // Start the loop with a block that tests the condition.
  // If there's an increment, the continue scope will be overwritten
  // later.
  JumpDest Continue = getJumpDestInCurrentScope("for.cond");
  llvm::BasicBlock *CondBlock = Continue.getBlock();
  EmitBlock(CondBlock);

  LoopStack.push(CondBlock, CGM.getContext(), ForAttrs, DL);

  // If the for loop doesn't have an increment we can just use the
  // condition as the continue block.  Otherwise we'll need to create
  // a block for it (in the current scope, i.e. in the scope of the
  // condition), and that we will become our continue block.
  if (S.getInc())
    Continue = getJumpDestInCurrentScope("for.inc");

  // Store the blocks to use for break and continue.
  BreakContinueStack.push_back(BreakContinue(LoopExit, Continue));

  // Create a cleanup scope for the condition variable cleanups.
  LexicalScope ConditionScope(*this, S.getSourceRange());

  if (S.getCond()) {
    // If the for statement has a condition scope, emit the local variable
    // declaration.
    if (S.getConditionVariable()) {
      EmitAutoVarDecl(*S.getConditionVariable());
    }

    llvm::BasicBlock *ExitBlock = LoopExit.getBlock();
    // If there are any cleanups between here and the loop-exit scope,
    // create a block to stage a loop exit along.
    if (ForScope.requiresCleanups())
      ExitBlock = createBasicBlock("for.cond.cleanup");

    // As long as the condition is true, iterate the loop.
    llvm::BasicBlock *ForBody = createBasicBlock("for.body");

    // C99 6.8.5p2/p4: The first substatement is executed if the expression
    // compares unequal to 0.  The condition must be a scalar type.
    llvm::Value *BoolCondVal = EvaluateExprAsBool(S.getCond());
    Builder.CreateCondBr(
        BoolCondVal, ForBody, ExitBlock,
        createProfileWeightsForLoop(S.getCond(), getProfileCount(S.getBody())));

    if (ExitBlock != LoopExit.getBlock()) {
      EmitBlock(ExitBlock);
      EmitBranchThroughCleanup(LoopExit);
    }

    EmitBlock(ForBody);
  } else {
    // Treat it as a non-zero constant.  Don't even create a new block for the
    // body, just fall into it.
  }
  incrementProfileCounter(&S);

  {
    // Create a separate cleanup scope for the body, in case it is not
    // a compound statement.
    RunCleanupsScope BodyScope(*this);
    EmitStmt(S.getBody());
  }

  // If there is an increment, emit it next.
  if (S.getInc()) {
    EmitBlock(Continue.getBlock());
    EmitStmt(S.getInc());
  }

  BreakContinueStack.pop_back();

  ConditionScope.ForceCleanup();

  EmitStopPoint(&S);
  EmitBranch(CondBlock);

  ForScope.ForceCleanup();

  LoopStack.pop();

  // Emit the fall-through block.
  EmitBlock(LoopExit.getBlock(), true);
}

