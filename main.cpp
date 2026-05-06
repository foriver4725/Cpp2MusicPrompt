#include <iostream>
#include <string>
#include <vector>
#include <algorithm>

#include "clang/AST/AST.h"
#include "clang/AST/RecursiveASTVisitor.h"
#include "clang/Frontend/FrontendActions.h"
#include "clang/Frontend/CompilerInstance.h"
#include "clang/Tooling/CommonOptionsParser.h"
#include "clang/Tooling/Tooling.h"

#include "llvm/Support/CommandLine.h"

using namespace clang;
using namespace clang::tooling;

struct AstEvent {
    unsigned offset;
    unsigned line;
    unsigned column;
    int priority;

    std::string kind;
    std::string name;
};

class AstEventVisitor : public RecursiveASTVisitor<AstEventVisitor> {
public:
    explicit AstEventVisitor(ASTContext &context)
        : context(context),
          sourceManager(context.getSourceManager()) {
    }

    bool VisitFunctionDecl(FunctionDecl *decl) {
        if (!IsInMainFile(decl))
            return true;

        if (!decl->isThisDeclarationADefinition())
            return true;

        AddEvent(
            decl->getBeginLoc(),
            0,
            "FunctionDefinition",
            decl->getNameAsString());

        return true;
    }

    bool VisitVarDecl(VarDecl *decl) {
        if (!IsInMainFile(decl))
            return true;

        // 関数引数は除外
        if (isa<ParmVarDecl>(decl))
            return true;

        AddEvent(
            decl->getBeginLoc(),
            0,
            "VariableDefinition",
            decl->getNameAsString());

        return true;
    }

    bool VisitIfStmt(IfStmt *stmt) {
        if (!IsInMainFile(stmt))
            return true;

        AddEvent(stmt->getBeginLoc(), 0, "IfStatement", "");
        return true;
    }

    bool VisitForStmt(ForStmt *stmt) {
        if (!IsInMainFile(stmt))
            return true;

        AddEvent(stmt->getBeginLoc(), 0, "ForLoop", "");
        return true;
    }

    bool VisitWhileStmt(WhileStmt *stmt) {
        if (!IsInMainFile(stmt))
            return true;

        AddEvent(stmt->getBeginLoc(), 0, "WhileLoop", "");
        return true;
    }

    bool VisitDoStmt(DoStmt *stmt) {
        if (!IsInMainFile(stmt))
            return true;

        AddEvent(stmt->getBeginLoc(), 0, "DoWhileLoop", "");
        return true;
    }

    bool VisitCompoundStmt(CompoundStmt *stmt) {
        if (!IsInMainFile(stmt))
            return true;

        AddEvent(stmt->getLBracLoc(), 0, "ScopeBlockStart", "{}");
        AddEvent(stmt->getRBracLoc(), 10, "ScopeBlockEnd", "{}");

        return true;
    }

    bool VisitReturnStmt(ReturnStmt *stmt) {
        if (!IsInMainFile(stmt))
            return true;

        AddEvent(stmt->getBeginLoc(), 0, "ReturnStatement", "");
        return true;
    }

    bool VisitCallExpr(CallExpr *expr) {
        if (!IsInMainFile(expr))
            return true;

        const FunctionDecl *callee = expr->getDirectCallee();

        AddEvent(
            expr->getBeginLoc(),
            0,
            "FunctionCall",
            callee ? callee->getNameAsString() : "");

        return true;
    }

    void PrintEvents() {
        std::sort(
            events.begin(),
            events.end(),
            [](const AstEvent &a, const AstEvent &b) {
                if (a.offset != b.offset)
                    return a.offset < b.offset;

                return a.priority < b.priority;
            });

        for (const AstEvent &event: events) {
            std::cout
                    << "[" << event.line
                    << ":" << event.column
                    << "] "
                    << event.kind;

            if (!event.name.empty()) {
                std::cout << " : " << event.name;
            }

            std::cout << std::endl;
        }
    }

private:
    ASTContext &context;
    SourceManager &sourceManager;

    std::vector<AstEvent> events;

    bool IsInMainFile(const Decl *decl) const {
        return IsInMainFile(decl->getBeginLoc());
    }

    bool IsInMainFile(const Stmt *stmt) const {
        return IsInMainFile(stmt->getBeginLoc());
    }

    bool IsInMainFile(SourceLocation loc) const {
        loc = sourceManager.getSpellingLoc(loc);

        return
                loc.isValid() &&
                sourceManager.isWrittenInMainFile(loc);
    }

    void AddEvent(
        SourceLocation loc,
        int priority,
        std::string kind,
        std::string name) {
        loc = sourceManager.getSpellingLoc(loc);

        if (!loc.isValid())
            return;

        AstEvent event;

        event.offset =
                sourceManager.getFileOffset(loc);

        event.line =
                sourceManager.getSpellingLineNumber(loc);

        event.column =
                sourceManager.getSpellingColumnNumber(loc);

        event.priority = priority;

        event.kind = std::move(kind);
        event.name = std::move(name);

        events.push_back(std::move(event));
    }
};

class AstEventConsumer : public ASTConsumer {
public:
    explicit AstEventConsumer(ASTContext &context)
        : visitor(context) {
    }

    void HandleTranslationUnit(ASTContext &context) override {
        visitor.TraverseDecl(
            context.getTranslationUnitDecl());

        visitor.PrintEvents();
    }

private:
    AstEventVisitor visitor;
};

class AstEventAction : public ASTFrontendAction {
public:
    std::unique_ptr<ASTConsumer> CreateASTConsumer(
        CompilerInstance &compiler,
        llvm::StringRef file) override {
        return std::make_unique<AstEventConsumer>(
            compiler.getASTContext());
    }
};

static llvm::cl::OptionCategory ToolCategory(
    "cpp2-music-prompt");

int main(int argc, const char **argv) {
    auto expectedParser =
            CommonOptionsParser::create(
                argc,
                argv,
                ToolCategory);

    if (!expectedParser) {
        llvm::errs()
                << expectedParser.takeError();

        return 1;
    }

    CommonOptionsParser &optionsParser =
            expectedParser.get();

    ClangTool tool(
        optionsParser.getCompilations(),
        optionsParser.getSourcePathList());

    return tool.run(
        newFrontendActionFactory<AstEventAction>()
        .get());
}
