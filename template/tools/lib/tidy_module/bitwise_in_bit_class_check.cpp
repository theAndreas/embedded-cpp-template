// Copyright 2026 Andreas Burnickl
// SPDX-License-Identifier: GPL-3.0-or-later

#include "bitwise_in_bit_class_check.h"

#include "clang/AST/ASTContext.h"
#include "clang/AST/Decl.h"
#include "clang/AST/Expr.h"
#include "clang/AST/ParentMapContext.h"
#include "clang/ASTMatchers/ASTMatchFinder.h"
#include "clang/ASTMatchers/ASTMatchers.h"
#include "clang/Basic/SourceManager.h"

namespace house {
namespace {

using clang::ASTContext;
using clang::BinaryOperator;
using clang::Expr;
using clang::SourceManager;

// Whether `expr` is a named mask: a constant, an enumerator, a system header's
// macro, as `_BV(DDB5)`, or a `|` of named masks; in parentheses and casts.
bool IsNamedMask(Expr const& expr, ASTContext& context,
                 SourceManager const& sources) {
  Expr const* const bare = expr.IgnoreParenCasts();
  if (sources.isInSystemMacro(bare->getBeginLoc())) {
    return true;
  }
  if (auto const* ref = llvm::dyn_cast<clang::DeclRefExpr>(bare)) {
    clang::ValueDecl const* const decl = ref->getDecl();
    if (llvm::isa<clang::EnumConstantDecl>(decl)) {
      return true;
    }
    auto const* const var = llvm::dyn_cast<clang::VarDecl>(decl);
    return var != nullptr && var->isUsableInConstantExpressions(context);
  }
  if (auto const* op = llvm::dyn_cast<BinaryOperator>(bare)) {
    return op->getOpcode() == clang::BO_Or &&
           IsNamedMask(*op->getLHS(), context, sources) &&
           IsNamedMask(*op->getRHS(), context, sources);
  }
  return false;
}

// Whether `expr` is, or lies in, the initializer of a constexpr variable,
// through parentheses, casts, braces and further `|` only.
bool InConstexprInitializer(Expr const& expr, ASTContext& context) {
  clang::DynTypedNode node{clang::DynTypedNode::create(expr)};
  while (true) {
    clang::DynTypedNodeList const parents{context.getParents(node)};
    if (parents.size() != 1) {
      return false;
    }
    clang::DynTypedNode const& parent = parents[0];
    if (auto const* var = parent.get<clang::VarDecl>()) {
      return var->isConstexpr();
    }
    auto const* const outer = parent.get<Expr>();
    if (outer == nullptr) {
      return false;
    }
    auto const* const op = llvm::dyn_cast<BinaryOperator>(outer);
    bool const passes =
        llvm::isa<clang::ParenExpr, clang::CastExpr, clang::InitListExpr,
                  clang::FullExpr>(outer) ||
        (op != nullptr && op->getOpcode() == clang::BO_Or);
    if (!passes) {
      return false;
    }
    node = parent;
  }
}

}  // namespace

BitwiseInBitClassCheck::BitwiseInBitClassCheck(
    llvm::StringRef name, clang::tidy::ClangTidyContext* context)
    : ClangTidyCheck{name, context},
      bit_classes_{Options.get("BitClasses", "Bit")} {
  llvm::SmallVector<llvm::StringRef, 4> split;
  llvm::StringRef{bit_classes_}.split(split, ';', -1, false);
  for (llvm::StringRef const part : split) {
    names_.push_back(part.trim());
  }
}

void BitwiseInBitClassCheck::storeOptions(
    clang::tidy::ClangTidyOptions::OptionMap& options) {
  Options.store(options, "BitClasses", bit_classes_);
}

void BitwiseInBitClassCheck::registerMatchers(
    clang::ast_matchers::MatchFinder* finder) {
  using clang::ast_matchers::anyOf;
  using clang::ast_matchers::binaryOperator;
  using clang::ast_matchers::cxxMethodDecl;
  using clang::ast_matchers::cxxRecordDecl;
  using clang::ast_matchers::forFunction;
  using clang::ast_matchers::functionDecl;
  using clang::ast_matchers::hasAncestor;
  using clang::ast_matchers::hasAnyName;
  using clang::ast_matchers::hasAnyOperatorName;
  using clang::ast_matchers::hasAnyOverloadedOperatorName;
  using clang::ast_matchers::hasOperatorName;
  using clang::ast_matchers::ofClass;
  using clang::ast_matchers::unaryOperator;
  using clang::ast_matchers::unless;
  auto const bit_class = cxxRecordDecl(hasAnyName(names_));
  // In the class, a class nested in it, or a member defined outside it; or in
  // a bitwise operator of a type of its own, which says what its bits mean.
  auto const in_bit_class = anyOf(
      hasAncestor(bit_class),
      forFunction(
          cxxMethodDecl(ofClass(anyOf(bit_class, hasAncestor(bit_class))))),
      forFunction(functionDecl(hasAnyOverloadedOperatorName(
          "&", "|", "^", "~", "<<", ">>", "&=", "|=", "^=", "<<=", ">>="))));
  finder->addMatcher(
      binaryOperator(hasAnyOperatorName("&", "|", "^", "<<", ">>",
                                        "&=", "|=", "^=", "<<=", ">>="),
                     unless(in_bit_class))
          .bind("op"),
      this);
  finder->addMatcher(
      unaryOperator(hasOperatorName("~"), unless(in_bit_class)).bind("op"),
      this);
}

void BitwiseInBitClassCheck::check(
    clang::ast_matchers::MatchFinder::MatchResult const& result) {
  auto const* const expr = result.Nodes.getNodeAs<Expr>("op");
  SourceManager const& sources = *result.SourceManager;
  // As written in a template: reported in the instantiation, which knows the
  // types.
  if (expr->isTypeDependent() || expr->isValueDependent()) {
    return;
  }
  clang::SourceLocation location;
  llvm::StringRef spelling;
  auto const* const binary = llvm::dyn_cast<BinaryOperator>(expr);
  if (binary != nullptr) {
    location = binary->getOperatorLoc();
    spelling = binary->getOpcodeStr();
  } else {
    location = llvm::cast<clang::UnaryOperator>(expr)->getOperatorLoc();
    spelling = "~";
  }
  if (sources.isInSystemMacro(location)) {
    return;
  }
  ASTContext& context = *result.Context;
  if (binary != nullptr && binary->getOpcode() == clang::BO_Or &&
      IsNamedMask(*binary, context, sources) &&
      InConstexprInitializer(*binary, context)) {
    return;
  }
  diag(location,
       "bitwise operator '%0' outside the bit class; use its functions, as "
       "%1<uint8_t>::SetBits(), or a constexpr of named masks or'ed "
       "(CodingStyle H-9)")
      << spelling << names_.front();
}

}  // namespace house
