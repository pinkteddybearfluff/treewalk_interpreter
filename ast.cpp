#include "ast.h"
#include <stdexcept>
#include <iostream>

using std::cout;

constexpr int IndentSize = 2;
FunctionTable functions;

RuntimeValue NumberNode::evaluateNode(EnvironmentStack& scopes) const
{
    return RuntimeValue(value);
}

void NumberNode::debugPrint(int indentLevel) const
{
    cout << "Number(" << value << ")\n";
}

RuntimeValue StringNode::evaluateNode(EnvironmentStack& scopes) const
{
    return RuntimeValue(value);
}

void StringNode::debugPrint(int indentLevel) const
{
    cout << "String(" << value << ")\n";
}

RuntimeValue UnaryNode::evaluateNode(EnvironmentStack& scopes) const
{
    RuntimeValue value = child->evaluateNode(scopes);
    if (value.isNumber())
    {
        if (op.type == TokenType::Minus)
        {
            return RuntimeValue(-value.asNumber());
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

void BooleanNode::debugPrint(int indentLevel) const
{
    cout << "Boolean(" << ((value == true) ? "true" : "false") << ")\n";
}

RuntimeValue BooleanNode::evaluateNode(EnvironmentStack& scopes) const
{
    return RuntimeValue(value);
}

RuntimeValue ArrayNode::evaluateNode(EnvironmentStack& scopes) const
{
    shared_ptr<vector<RuntimeValue>> arrayPtr = std::make_shared<vector<RuntimeValue>>();

    for (int i = 0; i < value.size(); ++i)
    {
        arrayPtr->push_back(value[i]->evaluateNode(scopes));
    }
    return RuntimeValue(arrayPtr);
}

void ArrayNode::debugPrint(int indentLevel) const
{
    cout << "Array[\n";
    for (auto& element : value)
    {
        cout << string(indentLevel * IndentSize, ' ');
        element->debugPrint(indentLevel + 1);
    }
    cout << string(indentLevel * IndentSize, ' ') << "]\n";
}

RuntimeValue IndexNode::evaluateNode(EnvironmentStack& scopes) const
{
    if (operand->evaluateNode(scopes).isArray())
        return operand->evaluateNode(scopes).asArrayPtr()->at(indexExp->evaluateNode(scopes).asNumber());
    if (operand->evaluateNode(scopes).isString())
        return RuntimeValue(
            string(1, operand->evaluateNode(scopes).asString().at(indexExp->evaluateNode(scopes).asNumber())));
}

RuntimeValue& IndexNode::getReference(EnvironmentStack& scopes)
{
    return operand->evaluateNode(scopes).asArrayPtr()->at(indexExp->evaluateNode(scopes).asNumber());
}


void IndexNode::debugPrint(int indentLevel) const
{
    operand->debugPrint(indentLevel + 1);
    cout << "[";
    indexExp->debugPrint(indentLevel + 1);
    cout << "]\n";
}

RuntimeValue BinaryNode::evaluateNode(EnvironmentStack& scopes) const
{
    RuntimeValue lval = left->evaluateNode(scopes);
    RuntimeValue rval = right->evaluateNode(scopes);

    if (lval.isNumber() && rval.isNumber())
    {
        const auto& lvalue = lval.asNumber();
        const auto& rvalue = rval.asNumber();
        switch (this->op.type)
        {
        case TokenType::Plus:
            return RuntimeValue(lvalue + rvalue);
        case TokenType::Minus:
            return RuntimeValue(lvalue - rvalue);
        case TokenType::Multiply:
            return RuntimeValue(lvalue * rvalue);
        case TokenType::Divide:
            return RuntimeValue(lvalue / rvalue);
        case TokenType::Greater:
            return RuntimeValue(lvalue > rvalue);
        case TokenType::GreaterEqual:
            return RuntimeValue(lvalue >= rvalue);
        case TokenType::LessEqual:
            return RuntimeValue(lvalue <= rvalue);
        case TokenType::Less:
            return RuntimeValue(lvalue < rvalue);
        case TokenType::Equal:
            return RuntimeValue(lvalue == rvalue);
        case TokenType::NotEqual:
            return RuntimeValue(lvalue != rvalue);
        default: throw std::runtime_error("Unknown operator for operand of type double");
        }
    }
    if (lval.isString() && rval.isString())
    {
        const auto& lvalue = lval.asString();
        const auto& rvalue = rval.asString();
        switch (this->op.type)
        {
        case TokenType::Plus:
            return RuntimeValue(lvalue + rvalue);
        case TokenType::Equal:
            return RuntimeValue(lvalue == rvalue);
        case TokenType::NotEqual:
            return RuntimeValue(lvalue != rvalue);
        case TokenType::Less:
            return RuntimeValue(lvalue < rvalue);
        case TokenType::LessEqual:
            return RuntimeValue(lvalue <= rvalue);
        case TokenType::Greater:
            return RuntimeValue(lvalue > rvalue);
        case TokenType::GreaterEqual:
            return RuntimeValue(lvalue >= rvalue);
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

void ExpressionStatementNode::evaluateNode(EnvironmentStack& scopes) const
{
    expressionStmt->evaluateNode(scopes);
}

void ExpressionStatementNode::debugPrint(int indentLevel) const
{
    cout << string(indentLevel * IndentSize, ' ') << "ExpressionStatement\n";
    expressionStmt->debugPrint(indentLevel + 1);
}

RuntimeValue VariableNode::evaluateNode(EnvironmentStack& scopes) const
{
    return scopes.get(identifierName);
}

RuntimeValue& VariableNode::getReference(EnvironmentStack& scopes)
{
    return scopes.get(identifierName);
}

// RuntimeValue& VariableNode::getFuncReference(FunctionTable& function_table)
// {
//     return
// }

void VariableNode::debugPrint(int i) const
{
    cout << "Variable(" << identifierName << ")\n";
}

RuntimeValue AssignmentNode::evaluateNode(EnvironmentStack& scopes) const
{
    RuntimeValue right = rvalue->evaluateNode(scopes);
    if (auto* var = dynamic_cast<VariableNode*>(lvalue.get()))
    {
        var->getReference(scopes) = right;
    }
    if (auto* indexElem = dynamic_cast<IndexNode*>(lvalue.get()))
    {
        indexElem->getReference(scopes) = right;
    }
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

void DeclarationNode::evaluateNode(EnvironmentStack& scopes) const
{
    RuntimeValue right = rvalue->evaluateNode(scopes);
    scopes.declare(lvalue->getIdentifierName(), right);
}

void DeclarationNode::debugPrint(int indentLevel) const
{
    cout << " Let\n";
    cout << string(IndentSize * indentLevel, ' ');
    lvalue->debugPrint(indentLevel + 1);
    cout << string(IndentSize * indentLevel, ' ');
    rvalue->debugPrint(indentLevel + 1);
}

void IfNode::evaluateNode(EnvironmentStack& scopes) const
{
    scopes.pushScope();
    RuntimeValue truthVal = condition->evaluateNode(scopes);
    if (truthVal.isBoolean())
    {
        if (truthVal.asBoolean())
            thenStatement->evaluateNode(scopes);

        else
        {
            if (elseStatement)
                elseStatement->evaluateNode(scopes);
        }
    }
    else throw std::runtime_error("type for condition does not reduce to boolean");
    scopes.popScope();
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

void WhileNode::evaluateNode(EnvironmentStack& scopes) const
{
    if (condition->evaluateNode(scopes).isBoolean())
        while (condition->evaluateNode(scopes).asBoolean())
        {
            try
            {
                scopes.pushScope();
                statement->evaluateNode(scopes);
                scopes.popScope();
            }
            catch (BreakSignal b)
            {
                scopes.popScope();
                break;
            }
            catch (ContinueSignal c)
            {
                scopes.popScope();
                continue;
            }
        }
    else throw std::runtime_error("type for condition does not reduce to boolean");
}

void WhileNode::debugPrint(int indentLevel) const
{
    cout << "While\n";
    cout << string(IndentSize * indentLevel, ' ');
    condition->debugPrint(indentLevel + 1);
    statement->debugPrint(indentLevel + 1);
}

void BreakNode::evaluateNode(EnvironmentStack& scopes) const
{
    throw BreakSignal();
}

void BreakNode::debugPrint(int indentLevel) const
{
    cout << "Break\n";
}

void ContinueNode::evaluateNode(EnvironmentStack& scopes) const
{
    throw ContinueSignal();
}

void ContinueNode::debugPrint(int indentLevel) const
{
    cout << "Continue\n";
}

void BlockNode::evaluateNode(EnvironmentStack& scopes) const
{
    scopes.pushScope();
    for (auto& statement : statements)
    {
        statement->evaluateNode(scopes);
    }
    scopes.popScope();
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

RuntimeValue FunctionCallNode::evaluateNode(EnvironmentStack& scopes) const
{
    if (auto* func = dynamic_cast<VariableNode*>(identifier.get()))
    {
        string identifierName = func->getIdentifierName();
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
                    env[parameters[i]] = RuntimeValue(arguments[i]->evaluateNode(scopes));
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
                return RuntimeValue(0.0);
            }
            validateArity(iter->second->getParametersSize(), arguments.size(), identifierName);
        }
        if (identifierName == "abs")
        {
            if (arguments.size() == 1)
                return RuntimeValue(static_cast<double>(abs(arguments[0]->evaluateNode(scopes).asNumber())));
            validateArity(1, arguments.size(), "abs");
        }
        if (identifierName == "max")
        {
            if (arguments.size() == 2)
                return RuntimeValue(std::max(arguments[0]->evaluateNode(scopes).asNumber(),
                                             arguments[1]->evaluateNode(scopes).asNumber()));
            validateArity(2, arguments.size(), "max");
        }
        if (identifierName == "min")
        {
            if (arguments.size() == 2)
                return RuntimeValue((std::min(arguments[0]->evaluateNode(scopes).asNumber(),
                                              arguments[1]->evaluateNode(scopes).asNumber())));
            validateArity(2, arguments.size(), "min");
        }
        if (identifierName == "avg")
        {
            if (!arguments.empty())
            {
                double avg = 0;
                for (const auto& argument : arguments)
                    avg += argument->evaluateNode(scopes).asNumber();
                avg = avg / arguments.size();
                return RuntimeValue(avg);
            }
            validateArity(1, arguments.size(), "avg");
        }
        if (identifierName == "print")
        {
            if (!arguments.empty())
            {
                for (const auto& argument : arguments)
                {
                    RuntimeValue arg = argument->evaluateNode(scopes);
                    printRuntimeValue(arg);
                }
                cout << '\n';
            }
            return RuntimeValue(0.0);
        }
        if (identifierName == "size")
        {
            if (arguments.size() == 1)
            {
                RuntimeValue arg = arguments[0]->evaluateNode(scopes);
                if (arg.isArray())
                    return RuntimeValue(static_cast<double>(arg.asArrayPtr()->size()));
                if (arg.isString())
                    return RuntimeValue(static_cast<double>(arg.asString().size()));
                throw std::runtime_error("invalid operand for size method");
            }
            validateArity(1, arguments.size(), "size");
        }
        if (identifierName == "push")
        {
            if (arguments.size() == 2)
            {
                if (auto* var = dynamic_cast<VariableNode*>(arguments[0].get()))
                {
                    RuntimeValue arr = var->getReference(scopes);
                    if (arr.isArray())
                    {
                        RuntimeValue value = arguments[1]->evaluateNode(scopes);
                        arr.asArrayPtr()->push_back(value);
                        return RuntimeValue(0.0);
                    }
                    throw std::runtime_error("invalid operand for push");
                }
            }
        }
    }
    throw std::runtime_error("undefined function identifier");
}

// vector<unique_ptr<ExpressionNode>>& FunctionCallNode::getReference(EnvironmentStack& scopes) const
// {
//     return arguments;
// }

void FunctionCallNode::debugPrint(int indentLevel) const
{
    if (auto* func = dynamic_cast<VariableNode*>(identifier.get()))
        cout << "Function(" << func->getIdentifierName() << ")\n";
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

void FunctionDeclarationNode::evaluateNode(EnvironmentStack& scopes) const
{
    functions[identifier] = this;
}

void FunctionDeclarationNode::debugPrint(int indentLevel) const
{
    cout << "FunctionDeclaration\n";
    cout << string(IndentSize * indentLevel, ' ') << "Parameters(";
    for (const auto& parameter : parameters)
    {
        cout << parameter;
    }
    cout << ")\n";
    cout << string(IndentSize * indentLevel, ' ');
    body->debugPrint(indentLevel + 1);
}

void ReturnNode::evaluateNode(EnvironmentStack& scopes) const
{
    throw ReturnSignal(returnStatement->evaluateNode(scopes));
}

void ReturnNode::debugPrint(int indentLevel) const
{
    cout << "Return\n";
    cout << string(IndentSize * indentLevel, ' ');
    returnStatement->debugPrint(indentLevel + 1);
}

void ProgramNode::evaluateNode(EnvironmentStack& scopes)
{
    for (auto& stmt : statements)
        stmt->evaluateNode(scopes);
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

void EmptyNode::evaluateNode(EnvironmentStack& scopes) const
{
}

void EmptyNode::debugPrint(int indentLevel) const
{
    cout << string(indentLevel * IndentSize, ' ') << "EmptyStatement\n";
}
