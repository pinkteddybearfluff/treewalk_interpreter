#include "ast.h"
#include <stdexcept>
#include <iostream>

using std::cout;

int NumberNode::evaluateNode(map<string, int>& env) const
{
    return value;
}

// void NumberNode::debugPrint(int indentLevel) const
// {
//     for (int i = 0; i < indentLevel * 2; ++i) cout << " ";
//     for (int i = 0; i < indentLevel * 2; ++i) cout << " ";
//     cout << value << '\n';
// }

int UnaryNode::evaluateNode(map<string, int>& env) const
{
    int value = child->evaluateNode(env);
    if (op.type == TokenType::Minus) return -value;
    return value;
}

// void UnaryNode::debugPrint(int indentLevel) const
// {
//     for (int i = 0; i < indentLevel * 2; ++i) cout << " ";
//     for (int i = 0; i < indentLevel * 2; ++i) cout << " ";
//     cout << op << '\n';
//     for (int i = 0; i < indentLevel * 2; ++i) cout << " ";
//     child->debugPrint(indentLevel + 1);
// }

int BinaryNode::evaluateNode(map<string, int>& env) const
{
    int lval = left->evaluateNode(env);
    int rval = right->evaluateNode(env);

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

// void BinaryNode::debugPrint(int indentLevel) const
// {
// for (int i = 0; i < indentLevel * 2; ++i) cout << " ";
// for (int i = 0; i < indentLevel * 2; ++i) cout << " ";
// cout << op << '\n';
// for (int i = 0; i < indentLevel * 2; ++i) cout << " ";
// left->debugPrint(indentLevel + 1);
// for (int i = 0; i < indentLevel * 2; ++i) cout << " ";
// right->debugPrint(indentLevel + 1);
// }

int VariableNode::evaluateNode(map<string, int>& env) const
{
    auto iter = env.find(identifierName);
    if (iter == env.end())
        throw std::runtime_error("Undefined variable: " + identifierName);
    return iter->second;
}

int AssignmentNode::evaluateNode(map<string, int>& env) const
{
    int right = rvalue->evaluateNode(env);
    env[lvalue->getIdentifierName()] = right;
    return right;
}

int FunctionCallNode::evaluateNode(map<string, int>& env) const
{
    if (identifierName == "abs")
    {
        if (arguments.size() == 1)
            return abs(arguments[0]->evaluateNode(env));
        validateArity(1, arguments.size(), "abs");
    }
    if (identifierName == "max")
    {
        if (arguments.size() == 2)
            return std::max(arguments[0]->evaluateNode(env), arguments[1]->evaluateNode(env));
        validateArity(2, arguments.size(), "max");
    }
    if (identifierName == "min")
    {
        if (arguments.size() == 2)
            return std::min(arguments[0]->evaluateNode(env), arguments[1]->evaluateNode(env));
        validateArity(2, arguments.size(), "min");
    }
    if (identifierName == "avg")
    {
        if (arguments.size() != 0)
        {
            int avg = 0;
            for (int i = 0; i < arguments.size(); ++i)
                avg += arguments[i]->evaluateNode(env);
            avg = avg / arguments.size();
            return avg;
        }
        validateArity(1, arguments.size(), "avg");
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

