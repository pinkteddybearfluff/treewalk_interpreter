#include "DeclarationVisitor.h"

#include "../ast/Ast.h"

void DeclarationVisitor::visit(FunctionDeclarationNode& node)
{
    vector<Parameter> params;
    for (const auto& paramAst : node.parameters)
    {
        params.emplace_back(paramAst.identifier, paramAst.defaultArg.get(), paramAst.type.get(), paramAst.isVariadic);
    }
}
