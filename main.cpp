#include <iostream>
#include <string>
#include <vector>
#include <algorithm>
#include <fstream>

#include <nlohmann/json.hpp>

#include "clang/AST/AST.h"
#include "clang/AST/RecursiveASTVisitor.h"
#include "clang/Frontend/FrontendActions.h"
#include "clang/Frontend/CompilerInstance.h"
#include "clang/Tooling/CommonOptionsParser.h"
#include "clang/Tooling/Tooling.h"

#include "llvm/Support/CommandLine.h"

using namespace clang;
using namespace clang::tooling;
using json = nlohmann::json;

struct AstEvent {
    unsigned offset{};
    unsigned line{};
    unsigned column{};
    int priority{};

    std::string kind;
    std::string name;
};

class AstEventVisitor : public RecursiveASTVisitor<AstEventVisitor> {
public:
    explicit AstEventVisitor(ASTContext &context, std::vector<AstEvent> &outputEvents)
        : context(context),
          sourceManager(context.getSourceManager()),
          events(outputEvents) {
    }

    bool VisitFunctionDecl(const FunctionDecl *decl) const {
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

    bool VisitVarDecl(VarDecl *decl) const {
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

    bool VisitIfStmt(const IfStmt *stmt) const {
        if (!IsInMainFile(stmt))
            return true;

        AddEvent(stmt->getBeginLoc(), 0, "IfStatement", "");
        return true;
    }

    bool VisitForStmt(const ForStmt *stmt) const {
        if (!IsInMainFile(stmt))
            return true;

        AddEvent(stmt->getBeginLoc(), 0, "ForLoop", "");
        return true;
    }

    bool VisitWhileStmt(const WhileStmt *stmt) const {
        if (!IsInMainFile(stmt))
            return true;

        AddEvent(stmt->getBeginLoc(), 0, "WhileLoop", "");
        return true;
    }

    bool VisitDoStmt(const DoStmt *stmt) const {
        if (!IsInMainFile(stmt))
            return true;

        AddEvent(stmt->getBeginLoc(), 0, "DoWhileLoop", "");
        return true;
    }

    bool VisitCompoundStmt(const CompoundStmt *stmt) const {
        if (!IsInMainFile(stmt))
            return true;

        AddEvent(stmt->getLBracLoc(), 0, "ScopeBlockStart", "{}");
        AddEvent(stmt->getRBracLoc(), 10, "ScopeBlockEnd", "{}");

        return true;
    }

    bool VisitReturnStmt(const ReturnStmt *stmt) const {
        if (!IsInMainFile(stmt))
            return true;

        AddEvent(stmt->getBeginLoc(), 0, "ReturnStatement", "");
        return true;
    }

    bool VisitCallExpr(CallExpr *expr) const {
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

    void PrintEvents() const {
        std::ranges::sort(
            events,
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

    std::vector<AstEvent> &events;

    [[nodiscard]] bool IsInMainFile(const Decl *decl) const {
        return IsInMainFile(decl->getBeginLoc());
    }

    [[nodiscard]] bool IsInMainFile(const Stmt *stmt) const {
        return IsInMainFile(stmt->getBeginLoc());
    }

    [[nodiscard]] bool IsInMainFile(SourceLocation loc) const {
        loc = sourceManager.getSpellingLoc(loc);

        return
                loc.isValid() &&
                sourceManager.isWrittenInMainFile(loc);
    }

    void AddEvent(
        SourceLocation loc,
        const int priority,
        std::string kind,
        std::string name) const {
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
    AstEventConsumer(ASTContext &context, std::vector<AstEvent> &outputEvents)
        : visitor(context, outputEvents) {
    }

    void HandleTranslationUnit(ASTContext &context) override {
        visitor.TraverseDecl(context.getTranslationUnitDecl());
    }

private:
    AstEventVisitor visitor;
};

class AstEventAction : public ASTFrontendAction {
public:
    explicit AstEventAction(std::vector<AstEvent> &outputEvents)
        : outputEvents(outputEvents) {
    }

protected:
    std::unique_ptr<ASTConsumer> CreateASTConsumer(
        CompilerInstance &compiler,
        llvm::StringRef file) override {
        return std::make_unique<AstEventConsumer>(
            compiler.getASTContext(),
            outputEvents);
    }

private:
    std::vector<AstEvent> &outputEvents;
};

class AstEventActionFactory : public FrontendActionFactory {
public:
    explicit AstEventActionFactory(std::vector<AstEvent> &outputEvents)
        : outputEvents(outputEvents) {
    }

    std::unique_ptr<FrontendAction> create() override {
        return std::make_unique<AstEventAction>(outputEvents);
    }

private:
    std::vector<AstEvent> &outputEvents;
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

    std::vector<AstEvent> events;

    AstEventActionFactory factory(events);

    const int result = tool.run(&factory);

    std::ranges::sort(
        events,
        [](const AstEvent &a, const AstEvent &b) {
            if (a.offset != b.offset)
                return a.offset < b.offset;

            return a.priority < b.priority;
        });

    json output = json::array();

    for (const AstEvent &event: events) {
        output.push_back({
            {"offset", event.offset},
            {"line", event.line},
            {"column", event.column},
            {"priority", event.priority},
            {"kind", event.kind},
            {"name", event.name}
        });
    }

    // events.json に保存
    std::ofstream file("events.json");

    if (!file.is_open()) {
        std::cerr << "Failed to open events.json" << std::endl;
        return 1;
    }

    file << output.dump(4);

    file.close();

    std::cout << "Generated events.json" << std::endl;

    return 0;
}
