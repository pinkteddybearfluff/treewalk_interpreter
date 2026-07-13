#ifndef INTERPRETER_VISITORS_H
#define INTERPRETER_VISITORS_H
#include "../runtime/RuntimeValue.h"
#include "../runtime/Environment.h"

class NumberNode;
class StringNode;
class BooleanNode;
class NullNode;

class ArrayNode;
class MapNode;

class VariableNode;

class UnaryNode;
class BinaryNode;
class IsNode;

class MemberAccessNode;
class IndexNode;
class FunctionCallNode;

class MatchPatternNode;
class BlockExpressionNode;

class AssignmentNode;
class CompoundAssignmentNode;

class ExpressionStatementNode;

class StructDeclarationNode;
class EnumDeclarationNode;
class DeclarationNode;
class FunctionDeclarationNode;

class ReturnNode;

class BlockNode;
class IfNode;

class WhileNode;
class ForNode;
class ForEachNode;
class ContinueNode;
class BreakNode;

class EmptyNode;

class ImportNode;
class TryCatch;
class ThrowNode;

class Visitor
{
public:
    //Expressions
    virtual void visit(NumberNode& node) =0;
    virtual void visit(StringNode& node) =0;
    virtual void visit(BooleanNode& node) =0;
    virtual void visit(NullNode& node) =0;

    virtual void visit(ArrayNode& node) =0;
    virtual void visit(MapNode& node) = 0;

    virtual void visit(VariableNode& node) =0;

    virtual void visit(UnaryNode& node) =0;
    virtual void visit(BinaryNode& node) =0;
    virtual void visit(IsNode& node) =0;

    virtual void visit(MemberAccessNode& node) =0;
    virtual void visit(IndexNode& node) =0;
    virtual void visit(FunctionCallNode& node) =0;

    virtual void visit(MatchPatternNode& node) =0;
    virtual void visit(BlockExpressionNode& node) =0;

    virtual void visit(AssignmentNode& node) =0;
    virtual void visit(CompoundAssignmentNode& node) =0;

    //Statements
    virtual void visit(ExpressionStatementNode& node) =0;

    virtual void visit(StructDeclarationNode& node) =0;
    virtual void visit(EnumDeclarationNode& node) =0;
    virtual void visit(FunctionDeclarationNode& node) =0;
    virtual void visit(DeclarationNode& node) =0;

    virtual void visit(ReturnNode& node) =0;

    virtual void visit(BlockNode& node) =0;
    virtual void visit(IfNode& node) =0;
    virtual void visit(WhileNode& node) =0;
    virtual void visit(ForNode& node) =0;
    virtual void visit(ForEachNode& node) =0;
    virtual void visit(ContinueNode& node) =0;
    virtual void visit(BreakNode& node) =0;

    virtual void visit(EmptyNode& node) =0;

    virtual void visit(ImportNode& node) =0;
    virtual void visit(TryCatch& node) =0;
    virtual void visit(ThrowNode& node) =0;

    //Program
    virtual void visit(ProgramNode& node) =0;
    virtual ~Visitor() = default;
};

class PrimitiveType;
class ArrayType;
class MapType;
class TupleType;
class ProgramDefinedType;
class UnionType;

class TypeVisitor
{
public:
    virtual void visit(PrimitiveType& type) =0;
    virtual void visit(ArrayType& type) =0;
    virtual void visit(MapType& type) =0;
    virtual void visit(TupleType& type) =0;
    virtual void visit(ProgramDefinedType& type) =0;
    virtual void visit(UnionType& type) =0;
    virtual ~TypeVisitor() = default;
};

class OrPattern;
class LiteralPattern;
class IdentifierPattern;
class RangePattern;
class WildCardPattern;

class PatternVisitor
{
public:
    virtual void visit(OrPattern& pattern) =0;
    virtual void visit(LiteralPattern& pattern) =0;
    virtual void visit(IdentifierPattern& pattern) =0;
    virtual void visit(RangePattern& pattern) =0;
    virtual void visit(WildCardPattern& pattern) =0;
};

#endif //INTERPRETER_VISITORS_H
