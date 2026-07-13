#ifndef INTERPRETER_EVALUATEVISITOR_H
#define INTERPRETER_EVALUATEVISITOR_H

#include "Visitors.h"
#include <string_view>

constexpr auto EVALUATE{true};

struct SourceLocation
{
    std::string_view file;
    int line;
    int col;
};

struct StackFrame
{
    std::string function_name;
    SourceLocation callSite;
};

class EvaluateVisitor : public Visitor
{
public:
    explicit EvaluateVisitor(InterpreterContext& context) : m_context{context}
    {
    }

    RuntimeValue result;
    InterpreterContext& m_context;
    std::vector<StackFrame> m_stackTrace;

    //Expressions;
    void visit(NumberNode& node) override;
    void visit(StringNode& node) override;
    void visit(BooleanNode& node) override;
    void visit(NullNode& node) override;

    void visit(ArrayNode& node) override;
    void visit(MapNode& node) override;

    void visit(VariableNode& node) override;

    void visit(BinaryNode& node) override;
    void visit(UnaryNode& node) override;
    void visit(IsNode& node) override;

    void visit(IndexNode& node) override;
    void visit(MemberAccessNode& node) override;
    void visit(FunctionCallNode& node) override;

    RuntimeValue* resolveLValue(ExpressionNode& node);

    void visit(MatchPatternNode& node) override;
    void visit(BlockExpressionNode& node) override;

    void visit(AssignmentNode& node) override;
    void visit(CompoundAssignmentNode& node) override;

    //Statements
    void visit(ExpressionStatementNode& node) override;

    void visit(StructDeclarationNode& node) override;
    void visit(EnumDeclarationNode& node) override;
    void visit(FunctionDeclarationNode& node) override;
    void visit(DeclarationNode& node) override;

    void visit(ReturnNode& node) override;

    void visit(IfNode& node) override;
    void visit(BlockNode& node) override;
    void visit(WhileNode& node) override;
    void visit(ForNode& node) override;
    void visit(ForEachNode& node) override;
    void visit(ContinueNode& node) override;
    void visit(BreakNode& node) override;

    void visit(EmptyNode& node) override;

    void visit(ImportNode& node) override;
    void visit(TryCatch& node) override;
    void visit(ThrowNode& node) override;

    //Program
    void visit(ProgramNode& node) override;
    //imports
};

class ScopeGuard
{
public:
    ScopeGuard(shared_ptr<Environment> previous, shared_ptr<Environment> current) : m_previous{previous},
        m_currentEnv{current}
    {
        m_currentEnv = std::make_shared<Environment>();
        m_currentEnv->parent = m_previous;
    }

    ~ScopeGuard()
    {
        m_currentEnv = m_previous;
    }

private:
    shared_ptr<Environment> m_previous;
    shared_ptr<Environment> m_currentEnv;
};

#endif //INTERPRETER_EVALUATEVISITOR_H
