#ifndef INTERPRETER_DECLARATIONVISITOR_H
#define INTERPRETER_DECLARATIONVISITOR_H

#include "Visitors.h"

class DeclarationVisitor : public Visitor
{
    InterpreterContext& m_context;

    void visit(NumberNode& node) override
    {
    };

    void visit(StringNode& node) override
    {
    };

    void visit(BooleanNode& node) override
    {
    };

    void visit(NullNode& node) override
    {
    };

    void visit(ArrayNode& node) override
    {
    };

    void visit(MapNode& node) override
    {
    };

    void visit(VariableNode& node) override
    {
    };

    void visit(UnaryNode& node) override
    {
    };

    void visit(BinaryNode& node) override
    {
    };

    void visit(IsNode& node) override
    {
    };

    void visit(IndexNode& node) override
    {
    };

    void visit(MemberAccessNode& node) override
    {
    };

    void visit(FunctionCallNode& node) override
    {
    };

    void visit(MatchPatternNode& node) override
    {
    };

    void visit(BlockExpressionNode& node) override
    {
    };

    void visit(AssignmentNode& node) override
    {
    };

    void visit(CompoundAssignmentNode& node) override
    {
    };

    void visit(ExpressionStatementNode& node) override
    {
    };

    void visit(StructDeclarationNode& node) override;
    void visit(EnumDeclarationNode& node) override;
    void visit(FunctionDeclarationNode& node) override;
    void visit(DeclarationNode& node) override;

    void visit(ReturnNode& node) override
    {
    };

    void visit(BlockNode& node) override;
    void visit(IfNode& node) override;

    void visit(WhileNode& node) override;
    void visit(ForNode& node) override;
    void visit(ForEachNode& node) override;

    void visit(BreakNode& node) override
    {
    };

    void visit(ContinueNode& node) override
    {
    };

    void visit(TryCatch& node) override;

    void visit(ThrowNode& node) override
    {
    };

    void visit(ImportNode& node) override
    {
    };

    void visit(ProgramNode& node) override;
};


#endif //INTERPRETER_DECLARATIONVISITOR_H
