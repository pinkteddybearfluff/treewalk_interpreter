#ifndef INTERPRETER_FORMATVISITOR_H
#define INTERPRETER_FORMATVISITOR_H

#include <sstream>
#include "Visitors.h"

constexpr auto FORMAT{false};

class FormatVisitor : public Visitor, public TypeVisitor, public PatternVisitor
{
public:
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

    //Types
    void visit(PrimitiveType& type) override;
    void visit(ArrayType& type) override;
    void visit(MapType& type) override;
    void visit(TupleType& type) override;
    void visit(ProgramDefinedType& type) override;
    void visit(UnionType& type) override;

    //Patterns
    void visit(LiteralPattern& pattern) override;
    void visit(OrPattern& pattern) override;
    void visit(RangePattern& pattern) override;
    void visit(IdentifierPattern& pattern) override;
    void visit(WildCardPattern& pattern) override;

    string result()
    {
        return os.str();
    }

private:
    int indentLevel{0};

    std::ostringstream os{};

    void writeIndent()
    {
        os << string(indentLevel * 4, ' ');
    }
};


#endif //INTERPRETER_FORMATVISITOR_H
