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

void CodeGenFunction::EmitAmdahlForParallelStmt(const ForStmt &S,
                                  ArrayRef<const Attr *> ForAttrs) {

  auto AmdahlPForBeginBlock = createBasicBlock("amdahl.pfor.begin");
  EmitBlock(AmdahlPForBeginBlock);

#if 0
  // Outline the function
  // 
  // Need to create a new code gen function for the outlined function
  CodeGenFunction CGF(CGM, true);


  // Call the outlined function
  
  auto AmdahlPForEndBlock = createBasicBlock("amdahl.pfor.end");
  EmitBlock(AmdahlPForEndBlock);
  
  const CapturedStmt *CS = *cast<CapturedStmt>(S);

  CS->dump();

  CGOpenMPOutlinedRegionInfo CGInfo(*CS, ThreadIDVar, CodeGen, InnermostKind,
      HasCancel);
  CodeGenFunction::CGCapturedStmtRAII CapInfoRAII(CGF, &CGInfo);
  return CGF.GenerateOpenMPCapturedStmtFunction(*CS);

  assert(
      CapturedStmtInfo &&
      "CapturedStmtInfo should be set when generating the captured function");
  const CapturedDecl *CD = S.getCapturedDecl();
  const RecordDecl *RD = S.getCapturedRecordDecl();
  assert(CD->hasBody() && "missing CapturedDecl body");

  // Build the argument list.
  ASTContext &Ctx = CGM.getContext();
  FunctionArgList Args;
  Args.append(CD->param_begin(),
              std::next(CD->param_begin(), CD->getContextParamPosition()));
  auto I = S.captures().begin();
  for (auto *FD : RD->fields()) {
    QualType ArgType = FD->getType();
    IdentifierInfo *II = nullptr;
    VarDecl *CapVar = nullptr;

    // If this is a capture by copy and the type is not a pointer, the outlined
    // function argument type should be uintptr and the value properly casted to
    // uintptr. This is necessary given that the runtime library is only able to
    // deal with pointers. We can pass in the same way the VLA type sizes to the
    // outlined function.
    if ((I->capturesVariableByCopy() && !ArgType->isAnyPointerType()) ||
        I->capturesVariableArrayType())
      ArgType = Ctx.getUIntPtrType();

    if (I->capturesVariable() || I->capturesVariableByCopy()) {
      CapVar = I->getCapturedVar();
      II = CapVar->getIdentifier();
    } else if (I->capturesThis())
      II = &getContext().Idents.get("this");
    else {
      assert(I->capturesVariableArrayType());
      II = &getContext().Idents.get("vla");
    }
    if (ArgType->isVariablyModifiedType())
      ArgType = getContext().getVariableArrayDecayedType(ArgType);
    Args.push_back(ImplicitParamDecl::Create(getContext(), nullptr,
                                             FD->getLocation(), II, ArgType));
    ++I;
  }
  Args.append(
      std::next(CD->param_begin(), CD->getContextParamPosition() + 1),
      CD->param_end());

  // Create the function declaration.
  FunctionType::ExtInfo ExtInfo;
  const CGFunctionInfo &FuncInfo =
      CGM.getTypes().arrangeBuiltinFunctionDeclaration(Ctx.VoidTy, Args);
  llvm::FunctionType *FuncLLVMTy = CGM.getTypes().GetFunctionType(FuncInfo);

  llvm::Function *F = llvm::Function::Create(
      FuncLLVMTy, llvm::GlobalValue::InternalLinkage,
      CapturedStmtInfo->getHelperName(), &CGM.getModule());
  CGM.SetInternalFunctionAttributes(CD, F, FuncInfo);
  if (CD->isNothrow())
    F->addFnAttr(llvm::Attribute::NoUnwind);

  // Generate the function.
  StartFunction(CD, Ctx.VoidTy, F, FuncInfo, Args, CD->getLocation(),
                CD->getBody()->getLocStart());
  unsigned Cnt = CD->getContextParamPosition();
  I = S.captures().begin();
  for (auto *FD : RD->fields()) {
    // If we are capturing a pointer by copy we don't need to do anything, just
    // use the value that we get from the arguments.
    if (I->capturesVariableByCopy() && FD->getType()->isAnyPointerType()) {
      const VarDecl *CurVD = I->getCapturedVar();
      Address LocalAddr = GetAddrOfLocalVar(Args[Cnt]);
      // If the variable is a reference we need to materialize it here.
      if (CurVD->getType()->isReferenceType()) {
        Address RefAddr = CreateMemTemp(CurVD->getType(), getPointerAlign(),
                                        ".materialized_ref");
        EmitStoreOfScalar(LocalAddr.getPointer(), RefAddr, /*Volatile=*/false,
                          CurVD->getType());
        LocalAddr = RefAddr;
      }
      setAddrOfLocalVar(CurVD, LocalAddr);
      ++Cnt;
      ++I;
      continue;
    }

    LValue ArgLVal =
        MakeAddrLValue(GetAddrOfLocalVar(Args[Cnt]), Args[Cnt]->getType(),
                       AlignmentSource::Decl);
    if (FD->hasCapturedVLAType()) {
      LValue CastedArgLVal =
          MakeAddrLValue(castValueFromUintptr(*this, FD->getType(),
                                              Args[Cnt]->getName(), ArgLVal),
                         FD->getType(), AlignmentSource::Decl);
      auto *ExprArg =
          EmitLoadOfLValue(CastedArgLVal, SourceLocation()).getScalarVal();
      auto VAT = FD->getCapturedVLAType();
      VLASizeMap[VAT->getSizeExpr()] = ExprArg;
    } else if (I->capturesVariable()) {
      auto *Var = I->getCapturedVar();
      QualType VarTy = Var->getType();
      Address ArgAddr = ArgLVal.getAddress();
      if (!VarTy->isReferenceType()) {
        ArgAddr = EmitLoadOfReference(
            ArgAddr, ArgLVal.getType()->castAs<ReferenceType>());
      }
      setAddrOfLocalVar(
          Var, Address(ArgAddr.getPointer(), getContext().getDeclAlign(Var)));
    } else if (I->capturesVariableByCopy()) {
      assert(!FD->getType()->isAnyPointerType() &&
             "Not expecting a captured pointer.");
      auto *Var = I->getCapturedVar();
      QualType VarTy = Var->getType();
      setAddrOfLocalVar(Var, castValueFromUintptr(*this, FD->getType(),
                                                  Args[Cnt]->getName(), ArgLVal,
                                                  VarTy->isReferenceType()));
    } else {
      // If 'this' is captured, load it into CXXThisValue.
      assert(I->capturesThis());
      CXXThisValue =
          EmitLoadOfLValue(ArgLVal, Args[Cnt]->getLocation()).getScalarVal();
    }
    ++Cnt;
    ++I;
  }

  PGO.assignRegionCounters(GlobalDecl(CD), F);
  CapturedStmtInfo->EmitBody(*this, CD->getBody());
  FinishFunction(CD->getBodyRBrace());

  return F;
#endif // if 0
}

#if 0
// Build void __kmpc_fork_call(ident_t *loc, kmp_int32 argc, kmpc_micro
// microtask, ...);
llvm::Type *TypeParams[] = {getIdentTyPointerTy(), CGM.Int32Ty,
  getKmpc_MicroPointerTy()};
llvm::FunctionType *FnTy =
llvm::FunctionType::get(CGM.VoidTy, TypeParams, /*isVarArg*/ true);
RTLFn = CGM.CreateRuntimeFunction(FnTy, "__kmpc_fork_call");

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
#endif // if 0
