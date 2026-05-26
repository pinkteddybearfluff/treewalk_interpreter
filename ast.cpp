#include "ast.h"
#include <stdexcept>
#include <iostream>

using std::cout;

constexpr int IndentSize = 2;
FunctionTable functions;

Type NumberNode::evaluateNode(EnvironmentStack& scopes) const
{
    return value;
}

void NumberNode::debugPrint(int indentLevel) const
{
    cout << "Number(" << value << ")\n";
}

Type StringNode::evaluateNode(EnvironmentStack& scopes) const
{
    return value;
}

void StringNode::debugPrint(int indentLevel) const
{
    cout << "String(" << value << ")\n";
}

Type UnaryNode::evaluateNode(EnvironmentStack& scopes) const
{
    Type value = child->evaluateNode(scopes);
    if (std::holds_alternative<double>(value))
    {
        if (op.type == TokenType::Minus)
        {
            return -std::get<double>(value);
        }
        return value;
    }
    throw std::runtime_error("invalid operand type for " + getSymbolForOp(op.type));
}

void UnaryNode::debugPrint(int indentLevel) const
{
    cout << "Unary(" << getSymbolForOp(op.type) << ")\n";
    cout << string(IndentSize * indentLevel, ' ');
    child->debugPrint(indentLevel + 1);
}

Type BinaryNode::evaluateNode(EnvironmentStack& scopes) const
{
    Type lval = left->evaluateNode(scopes);
    Type rval = right->evaluateNode(scopes);

    if (std::holds_alternative<double>(lval) && std::holds_alternative<double>(rval))
    {
        const auto& lvalue = std::get<double>(lval);
        const auto& rvalue = std::get<double>(rval);
        switch (this->op.type)
        {
        case TokenType::Plus:
            return lvalue + rvalue;
        case TokenType::Minus:
            return lvalue - rvalue;
        case TokenType::Multiply:
            return lvalue * rvalue;
        case TokenType::Divide:
            return lvalue / rvalue;
        case TokenType::Greater:
            return lvalue > rvalue;
        case TokenType::GreaterEqual:
            return lvalue >= rvalue;
        case TokenType::LessEqual:
            return lvalue <= rvalue;
        case TokenType::Less:
            return lvalue < rvalue;
        case TokenType::Equal:
            return lvalue == rvalue;
        case TokenType::NotEqual:
            return lvalue != rvalue;
        default: throw std::runtime_error("Unknown operator for operand of type double");
        }
    }
    if (std::holds_alternative<string>(lval) && std::holds_alternative<string>(rval))
    {
        const auto& lvalue = std::get<string>(lval);
        const auto& rvalue = std::get<string>(rval);
        switch (this->op.type)
        {
        case TokenType::Plus:
            return lvalue + rvalue;
        case TokenType::Equal:
            return lvalue == rvalue;
        case TokenType::NotEqual:
            return lvalue != rvalue;
        case TokenType::Less:
            return lvalue < rvalue;
        case TokenType::LessEqual:
            return lvalue <= rvalue;
        case TokenType::Greater:
            return lvalue > rvalue;
        case TokenType::GreaterEqual:
            return lvalue >= rvalue;
        default: throw std::runtime_error("Unknown operator for operand of type string");
        }
    }
    throw std::runtime_error("invalid operand types for " + getSymbolForOp(op.type));
}

void BinaryNode::debugPrint(int indentLevel) const
{
    cout << "Binary(" << getSymbolForOp(op.type) << ")\n";
    cout << string(IndentSize * indentLevel, ' ');
    left->debugPrint(indentLevel + 1);
    cout << string(IndentSize * indentLevel, ' ');
    right->debugPrint(indentLevel + 1);
}

Type VariableNode::evaluateNode(EnvironmentStack& scopes) const
{
    return scopes.get(identifierName);
}

void VariableNode::debugPrint(int i) const
{
    cout << "Variable(" << identifierName << ")\n";
}

Type AssignmentNode::evaluateNode(EnvironmentStack& scopes) const
{
    string identifier = lvalue->getIdentifierName();
    scopes.get(identifier);
    Type right = rvalue->evaluateNode(scopes);
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

Type DeclarationNode::evaluateNode(EnvironmentStack& scopes) const
{
    Type right = rvalue->evaluateNode(scopes);
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

Type IfNode::evaluateNode(EnvironmentStack& scopes) const
{
    scopes.pushScope();
    Type truthVal = condition->evaluateNode(scopes);
    if (std::holds_alternative<bool>(truthVal))
    {
        if (std::get<bool>(truthVal))
            thenStatement->evaluateNode(scopes);

        else
        {
            if (elseStatement)
                elseStatement->evaluateNode(scopes);
        }
    }
    else throw std::runtime_error("type for condition does not reduce to boolean");
    scopes.popScope();
    return truthVal;
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

Type WhileNode::evaluateNode(EnvironmentStack& scopes) const
{
    Type truthVal = condition->evaluateNode(scopes);
    if (std::holds_alternative<bool>(truthVal))
        while (std::get<bool>(condition->evaluateNode(scopes)))
        {
            scopes.pushScope();
            statement->evaluateNode(scopes);
            scopes.popScope();
        }
    else throw std::runtime_error("type for condition does not reduce to boolean");
    return truthVal;
}

void WhileNode::debugPrint(int indentLevel) const
{
    cout << "While\n";
    cout << string(IndentSize * indentLevel, ' ');
    condition->debugPrint(indentLevel + 1);
    statement->debugPrint(indentLevel + 1);
}


Type BlockNode::evaluateNode(EnvironmentStack& scopes) const
{
    scopes.pushScope();
    for (auto& statement : statements)
    {
        statement->evaluateNode(scopes);
    }
    // scopes.popScope();
    return false;
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

Type FunctionCallNode::evaluateNode(EnvironmentStack& scopes) const
{
    auto iter = functions.find(identifierName);
    if (iter != functions.end())
    {
        EnvironmentStack localScopes;
        Environment env;
        vector<string> parameters = iter->second->getParameters();
        if (arguments.size() == iter->second->getParametersSize())
        {
            for (int i = 0; i < arguments.size(); ++i)
            {
                env[parameters[i]] = arguments[i]->evaluateNode(scopes);
            }
            localScopes.pushScope(env);
            try
            {
                iter->second->evaluateBody(localScopes);
            }
            catch (ReturnSignal& r)
            {
                return r.value;
            }
            localScopes.popScope();
            return 0.0;
        }
        validateArity(iter->second->getParametersSize(), arguments.size(), identifierName);
    }
    if (identifierName == "abs")
    {
        if (arguments.size() == 1)
            return double(abs(std::get<double>(arguments[0]->evaluateNode(scopes))));
        validateArity(1, arguments.size(), "abs");
    }
    if (identifierName == "max")
    {
        if (arguments.size() == 2)
            return std::max(std::get<double>(arguments[0]->evaluateNode(scopes)),
                            std::get<double>(arguments[1]->evaluateNode(scopes)));
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
                avg += std::get<double>(argument->evaluateNode(scopes));
            avg = avg / arguments.size();
            return avg;
        }
        validateArity(1, arguments.size(), "avg");
    }
    if (identifierName == "print")
    {
        if (!arguments.empty())
        {
            for (const auto& argument : arguments)
            {
                Type arg = argument->evaluateNode(scopes);
                if (std::holds_alternative<double>(arg))
                    cout << std::get<double>(arg) << ' ';
                else if (std::holds_alternative<string>(arg))
                    cout << std::get<string>(arg) << ' ';
                else if (std::holds_alternative<bool>(arg))
                    cout << std::get<bool>(arg) << ' ';
            }
            cout << '\n';
        }
        return true;
    }
    throw std::runtime_error("undefined function identifier");
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

Type FunctionDeclarationNode::evaluateNode(EnvironmentStack& scopes) const
{
    functions[identifier] = this;
    return 0.0;
}

void FunctionDeclarationNode::debugPrint(int indentLevel) const
{
    cout << "FunctionDeclaration\n";
    for (const auto& parameter : parameters)
    {
        cout << string(IndentSize * indentLevel, ' ') << parameter;
    }
    cout << string(IndentSize * indentLevel, ' ');
    body->debugPrint(indentLevel + 1);
}

Type ReturnNode::evaluateNode(EnvironmentStack& scopes) const
{
    throw ReturnSignal(returnStatement->evaluateNode(scopes));
}

void ReturnNode::debugPrint(int indentLevel) const
{
    cout << "Return\n";
    cout << string(IndentSize * indentLevel, ' ');
    returnStatement->debugPrint(indentLevel + 1);
}

Type ProgramNode::evaluateNode(EnvironmentStack& scopes)
{
    for (auto& stmt : statements)
        stmt->evaluateNode(scopes);
    return 0.0;
}

void ProgramNode::debugPrint(int indentLevel)
{
    for (const auto& stmt : statements)
        stmt->debugPrint(indentLevel + 1);
    for (const auto& func : functions)
    {
        cout << func.first << "\n";
    }
}
