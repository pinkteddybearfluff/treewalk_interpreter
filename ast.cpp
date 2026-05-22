#include "ast.h"
#include <stdexcept>
#include <iostream>

using std::cout;

constexpr int IndentSize = 2;

double NumberNode::evaluateNode(EnvironmentStack& scopes) const
{
    return value;
}

void NumberNode::debugPrint(int indentLevel) const
{
    cout << "Number(" << value << ")\n";
}

double UnaryNode::evaluateNode(EnvironmentStack& scopes) const
{
    double value = child->evaluateNode(scopes);
    if (op.type == TokenType::Minus) return -value;
    return value;
}

void UnaryNode::debugPrint(int indentLevel) const
{
    cout << "Unary(" << getSymbolForOp(op.type) << ")\n";
    cout << string(IndentSize * indentLevel, ' ');
    child->debugPrint(indentLevel + 1);
}

double BinaryNode::evaluateNode(EnvironmentStack& scopes) const
{
    double lval = left->evaluateNode(scopes);
    double rval = right->evaluateNode(scopes);

    switch (op.type)
    {
    case TokenType::Plus:
        return lval + rval;
    case TokenType::Minus:
        return lval - rval;
    case TokenType::Multiply:
        return lval * rval;
    case TokenType::Divide:
        return lval / rval;
    case TokenType::Greater:
        return lval > rval;
    case TokenType::GreaterEqual:
        return lval >= rval;
    case TokenType::LessEqual:
        return lval <= rval;
    case TokenType::Less:
        return lval < rval;
    case TokenType::Equal:
        return lval == rval;
    case TokenType::NotEqual:
        return lval != rval;
    default: throw std::runtime_error("Unknown operator");
    }
}

void BinaryNode::debugPrint(int indentLevel) const
{
    cout << "Binary(" << getSymbolForOp(op.type) << ")\n";
    cout << string(IndentSize * indentLevel, ' ');
    left->debugPrint(indentLevel + 1);
    cout << string(IndentSize * indentLevel, ' ');
    right->debugPrint(indentLevel + 1);
}

double VariableNode::evaluateNode(EnvironmentStack& scopes) const
{
    return scopes.get(identifierName);
}

void VariableNode::debugPrint(int i) const
{
    cout << "Variable(" << identifierName << ")\n";
}

double AssignmentNode::evaluateNode(EnvironmentStack& scopes) const
{
    string identifier = lvalue->getIdentifierName();
    scopes.get(identifier);
    double right = rvalue->evaluateNode(scopes);
    scopes.assign(identifier, right);
    return right;
}

void AssignmentNode::debugPrint(int indentLevel) const
{
    cout << "Assignment(=)\n";
    cout << string(IndentSize * indentLevel, ' ');
    lvalue->debugPrint(indentLevel + 1);
    cout << string(IndentSize * indentLevel, ' ');
    rvalue->debugPrint(indentLevel + 1);
}

double DeclarationNode::evaluateNode(EnvironmentStack& scopes) const
{
    double right = rvalue->evaluateNode(scopes);
    scopes.declare(lvalue->getIdentifierName(), right);
    return right;
}

void DeclarationNode::debugPrint(int indentLevel) const
{
    cout << " Let\n";
    cout << string(IndentSize * indentLevel, ' ');
    lvalue->debugPrint(indentLevel + 1);
    cout << string(IndentSize * indentLevel, ' ');
    rvalue->debugPrint(indentLevel + 1);
}

double IfNode::evaluateNode(EnvironmentStack& scopes) const
{
    scopes.pushScope();
    if (condition->evaluateNode(scopes))
    {
        thenStatement->evaluateNode(scopes);
    }
    else
    {
        if (elseStatement)
            elseStatement->evaluateNode(scopes);
    }
    scopes.popScope();
    return 0;
}


void IfNode::debugPrint(int indentLevel) const
{
    cout << "If\n";
    cout << string(IndentSize * indentLevel, ' ');
    condition->debugPrint(indentLevel + 1);
    cout << string(IndentSize * indentLevel, ' ');
    thenStatement->debugPrint(indentLevel + 1);

    if (elseStatement)
    {
        cout << string(IndentSize * indentLevel, ' ') << "Else\n";
        cout << string(IndentSize * indentLevel, ' ');
        elseStatement->debugPrint(indentLevel + 1);
    }
}

double WhileNode::evaluateNode(EnvironmentStack& scopes) const
{
    scopes.pushScope();
    while (condition->evaluateNode(scopes))
    {
        statement->evaluateNode(scopes);
    }
    return 0;
}

void WhileNode::debugPrint(int indentLevel) const
{
    cout << "While\n";
    cout << string(IndentSize * indentLevel, ' ');
    condition->debugPrint(indentLevel + 1);
    statement->debugPrint(indentLevel + 1);
}


double BlockNode::evaluateNode(EnvironmentStack& scopes) const
{
    for (auto& statement : statements)
    {
        statement->evaluateNode(scopes);
    }
    return 0;
}

void BlockNode::debugPrint(int indentLevel) const
{
    cout << "Block{\n";
    for (auto& statement : statements)
    {
        cout << string(IndentSize * indentLevel, ' ');
        statement->debugPrint(indentLevel + 1);
    }
    cout << string(IndentSize * indentLevel, ' ') << "}\n";
}

double FunctionCallNode::evaluateNode(EnvironmentStack& scopes) const
{
    if (identifierName == "abs")
    {
        if (arguments.size() == 1)
            return abs(arguments[0]->evaluateNode(scopes));
        validateArity(1, arguments.size(), "abs");
    }
    if (identifierName == "max")
    {
        if (arguments.size() == 2)
            return std::max(arguments[0]->evaluateNode(scopes), arguments[1]->evaluateNode(scopes));
        validateArity(2, arguments.size(), "max");
    }
    if (identifierName == "min")
    {
        if (arguments.size() == 2)
            return std::min(arguments[0]->evaluateNode(scopes), arguments[1]->evaluateNode(scopes));
        validateArity(2, arguments.size(), "min");
    }
    if (identifierName == "avg")
    {
        if (!arguments.empty())
        {
            double avg = 0;
            for (const auto& argument : arguments)
                avg += argument->evaluateNode(scopes);
            avg = avg / arguments.size();
            return avg;
        }
        validateArity(1, arguments.size(), "avg");
    }
}

void FunctionCallNode::debugPrint(int indentLevel) const
{
    cout << "Function(" << identifierName << ")\n";
    for (const auto& argument : arguments)
    {
        cout << string(IndentSize * indentLevel, ' ');
        argument->debugPrint(indentLevel + 1);
    }
}

void validateArity(int expected_arguments, int given_arguments, string f_name)
{
    if (given_arguments > expected_arguments)
    {
        throw std::runtime_error(
            "too many arguments to function '" + f_name + "'; expected " + std::to_string(expected_arguments) +
            ", have " + std::to_string(given_arguments));
    }
    if (given_arguments < expected_arguments)
    {
        throw std::runtime_error(
            "too few arguments to function '" + f_name + "'; expected " + std::to_string(expected_arguments) +
            ", have " + std::to_string(given_arguments));
    }
}

