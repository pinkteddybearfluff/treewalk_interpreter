#include "FormatVisitor.h"

#include "../ast/Ast.h"
#include "../stdlib/Stdlib.h"

void FormatVisitor::visit(NumberNode& node)
{
    os << node.value;
}

void FormatVisitor::visit(StringNode& node)
{
    os << "\"" << node.value << "\"";
}

void FormatVisitor::visit(BooleanNode& node)
{
    os << ((node.value) ? "true" : "false");
}

void FormatVisitor::visit(NullNode&)
{
    os << "null";
}

void FormatVisitor::visit(ArrayNode& node)
{
    os << "[";
    if (node.value.empty())
    {
        os << "]";
        return;
    }
    auto first{true};
    for (const auto& elementExpr : node.value)
    {
        if (!first)
        {
            os << ", ";
        }
        elementExpr->accept(*this);
        first = false;
    }
    os << "]";
}

void FormatVisitor::visit(MapNode& node)
{
    if (node.keyValPairs.empty())
    {
        os << "{}";
        return;
    }
    os << "{\n";
    ++indentLevel;
    auto first{true};
    for (const auto& keyVal : node.keyValPairs)
    {
        if (!first)
        {
            os << ",\n";
        }
        writeIndent();
        keyVal.first->accept(*this);
        os << ": ";
        keyVal.second->accept(*this);
        first = false;
    }
    --indentLevel;
    os << "}";
}

void FormatVisitor::visit(VariableNode& node)
{
    os << node.identifierName;
}

void FormatVisitor::visit(UnaryNode& node)
{
    os << getSymbolForOp(node.op.type);
    node.child->accept(*this);
}

void FormatVisitor::visit(BinaryNode& node)
{
    node.left->accept(*this);
    os << " " << getSymbolForOp(node.op.type) << " ";
    node.right->accept(*this);
}

void FormatVisitor::visit(IsNode& node)
{
    node.value->accept(*this);
    os << " is ";
    node.enumVariant->accept(*this);
}

void FormatVisitor::visit(MemberAccessNode& node)
{
    node.obj->accept(*this);
    os << ".";
    node.member->accept(*this);
}

void FormatVisitor::visit(IndexNode& node)
{
    node.operand->accept(*this);
    os << "[";
    node.subscript->accept(*this);
    os << "]";
}

void FormatVisitor::visit(FunctionCallNode& node)
{
    node.identifier->accept(*this);
    os << "(";
    auto first{true};
    for (const auto& arg : node.arguments)
    {
        if (!first)
        {
            os << ", ";
        }
        arg->accept(*this);
        first = false;
    }
    os << ")";
}

void FormatVisitor::visit(MatchPatternNode& node)
{
    os << "match ";
    node.scrutinee->accept(*this);
    os << " {\n";
    ++indentLevel;
    for (const auto& arm : node.armsExpr)
    {
        writeIndent();
        arm.pattern->accept(*this);
        os << " => ";
        arm.expr->accept(*this);
        os << ",\n";
    }

    --indentLevel;
    os << "}";
}

void FormatVisitor::visit(BlockExpressionNode& node)
{
    os << "{\n";
    ++indentLevel;
    for (const auto& stmt : node.statements)
    {
        writeIndent();
        stmt->accept(*this);
    }
    --indentLevel;
    os << "}\n";
}

void FormatVisitor::visit(AssignmentNode& node)
{
    node.lvalue->accept(*this);
    os << " = ";
    node.rvalue->accept(*this);
}

void FormatVisitor::visit(CompoundAssignmentNode& node)
{
    node.lvalue->accept(*this);
    os << " " << getSymbolForOp(node.op.type) << " ";
    node.rvalue->accept(*this);
}

void FormatVisitor::visit(ExpressionStatementNode& node)
{
    node.expressionStmt->accept(*this);
}

void FormatVisitor::visit(StructDeclarationNode& node)
{
    os << "struct " << node.typeName << " {\n";
    ++indentLevel;
    for (const auto& field : node.fieldNames)
    {
        writeIndent();
        os << field;
        os << ",\n";
    }
    os << "\n";
    for (const auto& method : node.methodsStmt)
    {
        writeIndent();
        method->accept(*this);
        os << "\n";
    }
    --indentLevel;
    os << "}";
}

void FormatVisitor::visit(EnumDeclarationNode& node)
{
    os << "enum " << node.typeName << " {\n";
    ++indentLevel;
    for (const auto& variant : node.variants)
    {
        writeIndent();
        os << variant.name;
        if (!variant.fields.empty())
        {
            auto first{true};
            os << "(";
            for (const auto& field : variant.fields)
            {
                if (!first)
                {
                    os << ", ";
                }
                os << field;
                first = false;
            }
            os << ")";
        }
        os << ",\n";
    }
    --indentLevel;
    os << "}";
}

void FormatVisitor::visit(FunctionDeclarationNode& node)
{
    os << "fn " << node.identifier;
    os << " (";
    auto first{true};
    for (const auto& parameter : node.parameters)
    {
        if (!first)
        {
            os << ", ";
        }
        if (parameter.isVariadic)
        {
            os << "..." << parameter.identifier;
        }
        else if (parameter.defaultArg)
        {
            os << parameter.identifier << " = ";
            parameter.defaultArg->accept(*this);
        }
        else
        {
            os << parameter.identifier;
        }

        first = false;
    }
    os << ")";
    if (node.type)
    {
        os << " -> ";
        node.type->accept(*this);
    }
    node.body->accept(*this);
    os << "\n";
}

void FormatVisitor::visit(DeclarationNode& node)
{
    if (auto lvalue = dynamic_cast<VariableNode*>(node.lvalue.get()))
    {
        os << "let " << lvalue->identifierName;
        if (node.type)
        {
            os << ": ";
            node.type->accept(*this);
        }
        os << " = ";
        node.rvalue->accept(*this);
    }
}

void FormatVisitor::visit(ReturnNode& node)
{
    os << "return ";
    if (node.returnStatement)
    {
        node.returnStatement->accept(*this);
    }
}

void FormatVisitor::visit(BlockNode& node)
{
    os << "{\n";
    ++indentLevel;
    for (const auto& stmt : node.statements)
    {
        writeIndent();
        stmt->accept(*this);
        if (dynamic_cast<ExpressionStatementNode*>(stmt.get()) || dynamic_cast<DeclarationNode*>(stmt.get()) ||
            dynamic_cast<BreakNode*>(stmt.get()) || dynamic_cast<ContinueNode*>(stmt.get()) ||
            dynamic_cast<ReturnNode*>
            (stmt.get()) || dynamic_cast<ThrowNode*>(stmt.get()))
        {
            os << ";\n";
        }
        else
        {
            os << "\n";
        }
    }
    --indentLevel;
    writeIndent();
    os << "}";
}

void FormatVisitor::visit(IfNode& node)
{
    os << "if (";
    node.condition->accept(*this);
    os << ")";
    if (!dynamic_cast<BlockNode*>(node.thenStatement.get()))
    {
        os << "\n";
        ++indentLevel;
        writeIndent();
    }
    node.thenStatement->accept(*this);
    if (!dynamic_cast<BlockNode*>(node.thenStatement.get()))
    {
        --indentLevel;
        if (dynamic_cast<ExpressionStatementNode*>(node.thenStatement.get()) || dynamic_cast<DeclarationNode*>(node.
                thenStatement.get()) ||
            dynamic_cast<BreakNode*>(node.thenStatement.get()) || dynamic_cast<ContinueNode*>(node.thenStatement.get())
            ||
            dynamic_cast<ReturnNode*>
            (node.thenStatement.get()) || dynamic_cast<ThrowNode*>(node.thenStatement.get()))
        {
            os << ";\n";
        }
        else
            os << "\n";
    }
    if (node.elseStatement)
    {
        if (dynamic_cast<BlockNode*>(node.thenStatement.get()))
        {
            os << " ";
        }
        os << "else ";
        if (!dynamic_cast<BlockNode*>(node.elseStatement.get()))
        {
            os << "\n";
            ++indentLevel;
            writeIndent();
        }
        node.elseStatement->accept(*this);
        if (!dynamic_cast<BlockNode*>(node.elseStatement.get()))
        {
            if (dynamic_cast<ExpressionStatementNode*>(node.elseStatement.get()) || dynamic_cast<DeclarationNode*>(node.
                    elseStatement.get()) ||
                dynamic_cast<BreakNode*>(node.elseStatement.get()) || dynamic_cast<ContinueNode*>(node.elseStatement.
                    get()) ||
                dynamic_cast<ReturnNode*>
                (node.elseStatement.get()) || dynamic_cast<ThrowNode*>(node.elseStatement.get()))
            {
                os << ";\n";
            }
            else
                os << "\n";
            --indentLevel;
        }
    }
}

void FormatVisitor::visit(ForNode& node)
{
    os << "for (";
    if (node.initializer)
    {
        node.initializer->accept(*this);
    }
    os << ";";
    if (node.condition)
    {
        os << " ";
        node.condition->accept(*this);
    }
    os << ";";
    if (node.expr)
    {
        os << " ";
        node.expr->accept(*this);
    }
    os << ") ";
    if (!dynamic_cast<BlockNode*>(node.statement.get()))
    {
        os << "\n";
        ++indentLevel;
        writeIndent();
    }
    node.statement->accept(*this);
    if (!dynamic_cast<BlockNode*>(node.statement.get()))
    {
        --indentLevel;
        if (dynamic_cast<ExpressionStatementNode*>(node.statement.get()) || dynamic_cast<DeclarationNode*>(node.
                statement.get()) ||
            dynamic_cast<BreakNode*>(node.statement.get()) || dynamic_cast<ContinueNode*>(node.statement.get()) ||
            dynamic_cast<ReturnNode*>
            (node.statement.get()) || dynamic_cast<ThrowNode*>(node.statement.get()))
        {
            os << ";\n";
        }
        else
            os << "\n";
    }
    else
        os << "\n";
}

void FormatVisitor::visit(ForEachNode& node)
{
    os << "for ( let ";
    if (node.identifiers.size() == 1)
    {
        os << node.identifiers[0];
    }
    else
    {
        os << "[";
        auto first{true};
        for (const auto& identifier : node.identifiers)
        {
            if (!first)
            {
                os << ", ";
            }
            os << identifier;
            first = false;
        }
        os << "]";
    }
    os << " in ";
    node.containerExp->accept(*this);
    if (!dynamic_cast<BlockNode*>(node.statement.get()))
    {
        os << "\n";
        ++indentLevel;
        writeIndent();
    }
    node.statement->accept(*this);
    if (!dynamic_cast<BlockNode*>(node.statement.get()))
    {
        --indentLevel;
        if (dynamic_cast<ExpressionStatementNode*>(node.statement.get()) || dynamic_cast<DeclarationNode*>(node.
                statement.get()) ||
            dynamic_cast<BreakNode*>(node.statement.get()) || dynamic_cast<ContinueNode*>(node.statement.get()) ||
            dynamic_cast<ReturnNode*>
            (node.statement.get()) || dynamic_cast<ThrowNode*>(node.statement.get()))
        {
            os << ";\n";
        }
        else
            os << "\n";
    }
    else os << "\n";
}

void FormatVisitor::visit(WhileNode& node)
{
    os << "while (";
    node.condition->accept(*this);
    os << ") ";
    if (!dynamic_cast<BlockNode*>(node.statement.get()))
    {
        os << "\n";
        ++indentLevel;
        writeIndent();
    }
    node.statement->accept(*this);
    if (!dynamic_cast<BlockNode*>(node.statement.get()))
    {
        --indentLevel;
        if (dynamic_cast<ExpressionStatementNode*>(node.statement.get()) || dynamic_cast<DeclarationNode*>(node.
                statement.get()) ||
            dynamic_cast<BreakNode*>(node.statement.get()) || dynamic_cast<ContinueNode*>(node.statement.get()) ||
            dynamic_cast<ReturnNode*>
            (node.statement.get()) || dynamic_cast<ThrowNode*>(node.statement.get()))
        {
            os << ";\n";
        }
        else
            os << "\n";
    }
    else os << "\n";
}

void FormatVisitor::visit(ContinueNode& node)
{
    os << "continue";
}

void FormatVisitor::visit(BreakNode& node)
{
    os << "break";
}

void FormatVisitor::visit(EmptyNode& node)
{
}

void FormatVisitor::visit(TryCatch& node)
{
    os << "try ";
    node.tryStatement->accept(*this);
    os << "catch ";
    node.catches[0].blockStatement->accept(*this);
}

void FormatVisitor::visit(ThrowNode& node)
{
    os << "throw ";
    node.throwValExp->accept(*this);
}

void FormatVisitor::visit(ImportNode& node)
{
    if (node.isStdLib)
        os << "import " << node.file << " as " << node.alias;
    else
        os << "import \"" << node.file << "\" as " << node.alias;
}

//Types
void FormatVisitor::visit(PrimitiveType& type)
{
    os << type.type;
}

void FormatVisitor::visit(ArrayType& type)
{
    os << "Array[";
    type.nestedType->accept(*this);
    os << "]";
}

void FormatVisitor::visit(MapType& type)
{
    os << "Map[";
    type.key->accept(*this);
    os << ", ";
    type.value->accept(*this);
    os << "]";
}

void FormatVisitor::visit(TupleType& type)
{
    os << "Tuple[";
    auto first{true};
    for (const auto& types : type.types)
    {
        if (!first)
        {
            os << ", ";
        }
        types->accept(*this);
        first = false;
    }
    os << "]";
}

void FormatVisitor::visit(ProgramDefinedType& type)
{
    os << type.typeName;
}

void FormatVisitor::visit(UnionType& type)
{
    auto first{true};
    for (const auto& types : type.types)
    {
        if (!first)
        {
            os << " | ";
        }
        types->accept(*this);
        first = false;
    }
}

void FormatVisitor::visit(ProgramNode& node)
{
    for (const auto& stmt : node.statements)
    {
        stmt->accept(*this);
        if (dynamic_cast<ExpressionStatementNode*>(stmt.get()) || dynamic_cast<DeclarationNode*>(stmt.get()) ||
            dynamic_cast<BreakNode*>(stmt.get()) || dynamic_cast<ContinueNode*>(stmt.get()) || dynamic_cast<ReturnNode*>
            (stmt.get()) || dynamic_cast<ThrowNode*>(stmt.get()))
        {
            os << ";\n";
        }
        else
        {
            os << "\n";
        }
    }
}

void FormatVisitor::visit(IdentifierPattern& pattern)
{
    os << pattern.name;
}

void FormatVisitor::visit(LiteralPattern& pattern)
{
    os << stdlib::stringify(pattern.value);
}

void FormatVisitor::visit(RangePattern& pattern)
{
    if (pattern.inclusive)
    {
        os << stdlib::stringify(pattern.start) << "..=" << stdlib::stringify(pattern.end);
    }
}

void FormatVisitor::visit(OrPattern& pattern)
{
    auto first{true};
    for (const auto& pattern_node : pattern.patterns)
    {
        if (!first)
        {
            os << " | ";
        }
        pattern_node->accept(*this);
        first = false;
    }
}

void FormatVisitor::visit(WildCardPattern& pattern)
{
    os << "_";
}
