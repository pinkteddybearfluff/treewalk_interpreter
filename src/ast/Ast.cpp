#include "Ast.h"
#include <stdexcept>
#include <iostream>
#include <ranges>

#include "../parser/Parser.h"
#include "../stdlib/Stdlib.h"

#include "../visitors/Visitors.h"

using std::cout;

constexpr int IndentSize = 2;
vector<string> stackTrace{};

void NumberNode::accept(Visitor& visitor)
{
    visitor.visit(*this);
}

void StringNode::accept(Visitor& visitor)
{
    visitor.visit(*this);
}

void BooleanNode::accept(Visitor& visitor)
{
    visitor.visit(*this);
}

void NullNode::accept(Visitor& visitor)
{
    visitor.visit(*this);
}

void ArrayNode::accept(Visitor& visitor)
{
    visitor.visit(*this);
}

void MapNode::accept(Visitor& visitor)
{
    visitor.visit(*this);
}

void VariableNode::accept(Visitor& visitor)
{
    visitor.visit(*this);
}

void UnaryNode::accept(Visitor& visitor)
{
    visitor.visit(*this);
}

void BinaryNode::accept(Visitor& visitor)
{
    visitor.visit(*this);
}

void IsNode::accept(Visitor& visitor)
{
    visitor.visit(*this);
}

void IndexNode::accept(Visitor& visitor)
{
    visitor.visit(*this);
}

void MemberAccessNode::accept(Visitor& visitor)
{
    visitor.visit(*this);
}

void FunctionCallNode::accept(Visitor& visitor)
{
    visitor.visit(*this);
}

void MatchPatternNode::accept(Visitor& visitor)
{
    visitor.visit(*this);
}

void BlockExpressionNode::accept(Visitor& visitor)
{
    visitor.visit(*this);
}

void AssignmentNode::accept(Visitor& visitor)
{
    visitor.visit(*this);
}

void CompoundAssignmentNode::accept(Visitor& visitor)
{
    visitor.visit(*this);
}

void StructDeclarationNode::registerInEnv(InterpreterContext& ctx) const
{
    std::map<string, shared_ptr<FunctionObject>> methods;
    for (auto& func : methodsStmt)
    {
        dynamic_cast<FunctionDeclarationNode*>(func.get())->registerInCustomEnv(methods, ctx);
    }

    ctx.env->types.insert(std::make_pair(typeName, make_unique<StructType>(typeName, fieldNames, std::move(methods))));
}

void EnumDeclarationNode::registerInEnv(InterpreterContext& ctx) const
{
    ctx.env->types.insert(std::make_pair(typeName, make_unique<EnumType>(typeName, variants)));
}


void FunctionDeclarationNode::registerInCustomEnv(std::map<string, shared_ptr<FunctionObject>>& methodEnv,
                                                  InterpreterContext& ctx)
{
    vector<Parameter> params;
    for (const auto& parameter : parameters)
    {
        params.emplace_back(parameter.identifier, parameter.defaultArg.get(), parameter.type.get(),
                            parameter.isVariadic);
    }
    auto* body_ptr = dynamic_cast<BlockNode*>(body.get());
    auto fn = std::make_shared<FunctionObject>(
        identifier, std::move(params), body_ptr, ctx.env, line);
    methodEnv.insert(std::make_pair(identifier, std::move(fn)));
}

void ExpressionStatementNode::accept(Visitor& visitor)
{
    visitor.visit(*this);
}

void StructDeclarationNode::accept(Visitor& visitor)
{
    visitor.visit(*this);
}

void EnumDeclarationNode::accept(Visitor& visitor)
{
    visitor.visit(*this);
}

void FunctionDeclarationNode::accept(Visitor& visitor)
{
    visitor.visit(*this);
}

void DeclarationNode::accept(Visitor& visitor)
{
    visitor.visit(*this);
}

void ReturnNode::accept(Visitor& visitor)
{
    visitor.visit(*this);
}

void BlockNode::accept(Visitor& visitor)
{
    visitor.visit(*this);
}

void IfNode::accept(Visitor& visitor)
{
    visitor.visit(*this);
}

void WhileNode::accept(Visitor& visitor)
{
    visitor.visit(*this);
}

void ForNode::accept(Visitor& visitor)
{
    visitor.visit(*this);
}

void ForEachNode::accept(Visitor& visitor)
{
    visitor.visit(*this);
}

void ContinueNode::accept(Visitor& visitor)
{
    visitor.visit(*this);
}

void BreakNode::accept(Visitor& visitor)
{
    visitor.visit(*this);
}

void EmptyNode::accept(Visitor& visitor)
{
    visitor.visit(*this);
}

void ImportNode::accept(Visitor& visitor)
{
    visitor.visit(*this);
}

void TryCatch::accept(Visitor& visitor)
{
    visitor.visit(*this);
}

void ThrowNode::accept(Visitor& visitor)
{
    visitor.visit(*this);
}

void ProgramNode::accept(Visitor& visitor)
{
    visitor.visit(*this);
}

void PrimitiveType::accept(TypeVisitor& visitor)
{
    visitor.visit(*this);
}

void ArrayType::accept(TypeVisitor& visitor)
{
    visitor.visit(*this);
}

void MapType::accept(TypeVisitor& visitor)
{
    visitor.visit(*this);
}

void TupleType::accept(TypeVisitor& visitor)
{
    visitor.visit(*this);
}

void UnionType::accept(TypeVisitor& visitor)
{
    visitor.visit(*this);
}

void ProgramDefinedType::accept(TypeVisitor& visitor)
{
    visitor.visit(*this);
}

void OrPattern::accept(PatternVisitor& visitor)
{
    visitor.visit(*this);
}

void IdentifierPattern::accept(PatternVisitor& visitor)
{
    visitor.visit(*this);
}

void LiteralPattern::accept(PatternVisitor& visitor)
{
    visitor.visit(*this);
}

void RangePattern::accept(PatternVisitor& visitor)
{
    visitor.visit(*this);
}

void WildCardPattern::accept(PatternVisitor& visitor)
{
    visitor.visit(*this);
}
