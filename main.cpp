#include <iostream>

#include "clang/AST/AST.h"
#include "clang/AST/RecursiveASTVisitor.h"
#include "clang/Frontend/FrontendActions.h"
#include "clang/Tooling/CommonOptionsParser.h"
#include "clang/Tooling/Tooling.h"

#include "llvm/Support/CommandLine.h"

using namespace clang;
using namespace clang::tooling;

class IfCounterVisitor : public RecursiveASTVisitor<IfCounterVisitor>
{
public:
    bool VisitIfStmt(IfStmt* stmt)
    {
        ++count;
        return true;
    }

    int GetCount() const
    {
        return count;
    }

private:
    int count = 0;
};

class IfCounterConsumer : public ASTConsumer
{
public:
    void HandleTranslationUnit(ASTContext& context) override
    {
        visitor.TraverseDecl(context.getTranslationUnitDecl());

        std::cout << "If count: "
                  << visitor.GetCount()
                  << std::endl;
    }

private:
    IfCounterVisitor visitor;
};

class IfCounterAction : public ASTFrontendAction
{
public:
    std::unique_ptr<ASTConsumer> CreateASTConsumer(
        CompilerInstance& compiler,
        llvm::StringRef file) override
    {
        return std::make_unique<IfCounterConsumer>();
    }
};

static llvm::cl::OptionCategory ToolCategory("if-counter");

int main(int argc, const char** argv)
{
    auto expectedParser =
        CommonOptionsParser::create(argc, argv, ToolCategory);

    if (!expectedParser)
    {
        llvm::errs() << expectedParser.takeError();
        return 1;
    }

    CommonOptionsParser& optionsParser =
        expectedParser.get();

    ClangTool tool(
        optionsParser.getCompilations(),
        optionsParser.getSourcePathList());

    return tool.run(
        newFrontendActionFactory<IfCounterAction>().get());
}