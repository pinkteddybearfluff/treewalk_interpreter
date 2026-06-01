#include "ast.h"
#include <stdexcept>
#include <iostream>

using std::cout;

constexpr int IndentSize = 2;
FunctionTable functions;

RuntimeValue NumberNode::evaluateNode(shared_ptr<Environment> env) const
{
    return value;
}

void NumberNode::debugPrint(int indentLevel) const
{
    cout << "Number(" << value << ")\n";
}

RuntimeValue StringNode::evaluateNode(shared_ptr<Environment> env) const
{
    return value;
}

void StringNode::debugPrint(int indentLevel) const
{
    cout << "String(" << value << ")\n";
}

RuntimeValue UnaryNode::evaluateNode(shared_ptr<Environment> env) const
{
    RuntimeValue value = child->evaluateNode(env);
    if (op.type == TokenType::Not)
    {
        return !value.isTruthy();
    }
    if (value.isNumber())
    {
        if (op.type == TokenType::Minus)
            return -value.asNumber();
        if (op.type == TokenType::Plus)
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

RuntimeValue BooleanNode::evaluateNode(shared_ptr<Environment> env) const
{
    return value;
}

RuntimeValue NullNode::evaluateNode(shared_ptr<Environment> env) const
{
    return {};
}

void NullNode::debugPrint(int indentLevel) const
{
    cout << "Null";
}


RuntimeValue ArrayNode::evaluateNode(shared_ptr<Environment> env) const
{
    shared_ptr<vector<RuntimeValue>> arrayPtr = std::make_shared<vector<RuntimeValue>>();

    for (int i = 0; i < value.size(); ++i)
    {
        arrayPtr->push_back(value[i]->evaluateNode(env));
    }
    return arrayPtr;
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

RuntimeValue IndexNode::evaluateNode(shared_ptr<Environment> env) const
{
    const RuntimeValue& object = operand->evaluateNode(env);
    const RuntimeValue indexV = indexExp->evaluateNode(env);
    if (indexV.isNumber())
    {
        const int index = static_cast<int>(indexV.asNumber());
        if (object.isArray())
        {
            if (index < object.asArrayPtr()->size())
                return object.asArrayPtr()->at(index);
            throw RuntimeError("array index out of range", {
                                   .category = ErrorCategory::IndexError,
                                   .kind = ErrorKind::IndexOutOfBounds,
                                   .primary = "array",
                                   .currentLine = line
                               });
        }
        if (object.isString())
        {
            if (index < object.asString().size())
                return object.asString().at(index);
            throw RuntimeError("array index out of range", {
                                   .category = ErrorCategory::IndexError,
                                   .kind = ErrorKind::IndexOutOfBounds,
                                   .primary = "string",
                                   .currentLine = line
                               });
        }
        throw RuntimeError("object is not subscriptable", {
                               .category = ErrorCategory::TypeError,
                               .kind = ErrorKind::NotSubscriptable,
                               .primary = object.description(),
                               .currentLine = line
                           });
    }
    throw RuntimeError("invalid type for array index", {
                           .category = ErrorCategory::TypeError, .kind = ErrorKind::InvalidIndexType,
                           .primary = "numbers",
                           .secondary = indexV.description(), .currentLine = line
                       });
}

RuntimeValue& IndexNode::getReference(shared_ptr<Environment> env)
{
    const RuntimeValue indexV = indexExp->evaluateNode(env);
    if (indexV.isNumber())
    {
        if (auto* var = dynamic_cast<VariableNode*>(operand.get()))
        {
            const RuntimeValue& object = var->getReference(env);
            if (object.isArray())
                return object.asArrayPtr()->at(indexV.asNumber());
        }
        if (auto* indexNode = dynamic_cast<IndexNode*>(operand.get()))
        {
            const RuntimeValue& object = indexNode->getReference(env);

            const int index = static_cast<int>(indexV.asNumber());
            if (object.isArray())
            {
                if (index < object.asArrayPtr()->size() && index >= 0)
                    return object.asArrayPtr()->at(index);
                throw RuntimeError("array index out of range", {
                                       .category = ErrorCategory::IndexError,
                                       .kind = ErrorKind::IndexOutOfBounds,
                                       .primary = "array",
                                       .currentLine = line
                                   });
            }

            throw RuntimeError("object is not subscriptable", {
                                   .category = ErrorCategory::TypeError,
                                   .kind = ErrorKind::NotSubscriptable,
                                   .primary = indexV.description(),
                                   .currentLine = line
                               });
        }
    }
    throw RuntimeError("invalid type for array index", {
                           .category = ErrorCategory::TypeError, .kind = ErrorKind::InvalidIndexType,
                           .primary = "numbers",
                           .secondary = indexV.description(), .currentLine = line
                       });
}


void IndexNode::debugPrint(int indentLevel) const
{
    cout << "IndexNode\n";
    operand->debugPrint(indentLevel + 1);
    cout << "[";
    indexExp->debugPrint(indentLevel + 1);
    cout << "]\n";
}

RuntimeValue BinaryNode::evaluateNode(shared_ptr<Environment> env) const
{
    if (op.type == TokenType::AndAnd)
    {
        RuntimeValue lval = left->evaluateNode(env);
        if (!lval.isTruthy())
            return false;
        RuntimeValue rval = right->evaluateNode(env);

        return rval.isTruthy();
    }
    if (op.type == TokenType::OrOr)
    {
        RuntimeValue lval = left->evaluateNode(env);
        if (lval.isTruthy())
            return true;
        RuntimeValue rval = right->evaluateNode(env);
        return rval.isTruthy();
    }
    RuntimeValue lval = left->evaluateNode(env);
    RuntimeValue rval = right->evaluateNode(env);

    try
    {
        if (lval.isNumber() && rval.isNumber())
        {
            const auto& lvalue = lval.asNumber();
            const auto& rvalue = rval.asNumber();
            switch (op.type)
            {
            case TokenType::Plus:
                return lvalue + rvalue;
            case TokenType::Minus:
                return lvalue - rvalue;
            case TokenType::Multiply:
                return lvalue * rvalue;
            case TokenType::Modulo:
                if (rvalue)
                    return fmod(lvalue, rvalue);
                throw RuntimeError("cannot divide by zero", {
                                       .category = ErrorCategory::ZeroDivisionError,
                                       .kind = ErrorKind::DivisionByZero,
                                       .currentLine = line
                                   });
            case TokenType::Divide:
                if (rvalue)
                    return lvalue / rvalue;
                throw RuntimeError("cannot divide by zero", {
                                       .category = ErrorCategory::ZeroDivisionError,
                                       .kind = ErrorKind::DivisionByZero,
                                       .currentLine = line
                                   });
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
            default: throw UnsupportedOperation();
            }
        }
        if (lval.isString() && rval.isString())
        {
            const auto& lvalue = lval.asString();
            const auto& rvalue = rval.asString();
            switch (op.type)
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
            default: throw UnsupportedOperation();
            }
        }
        if (lval.isBoolean() && rval.isBoolean())
        {
            const auto& lvalue = lval.asBoolean();
            const auto& rvalue = rval.asBoolean();
            switch (op.type)
            {
            case TokenType::Plus:
                return lvalue + rvalue;
            case TokenType::Minus:
                return lvalue - rvalue;
            case TokenType::Multiply:
                return lvalue * rvalue;
            case TokenType::Divide:
                {
                    if (rvalue)
                        return lvalue / rvalue;
                    throw RuntimeError("cannot divide by zero", {
                                           .category = ErrorCategory::ZeroDivisionError,
                                           .kind = ErrorKind::DivisionByZero,
                                           .currentLine = line
                                       });
                }
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
            default: throw UnsupportedOperation();
            }
        }
        if (lval.isArray() && rval.isArray())
        {
            const auto& lvalue = lval.asArrayPtr();
            const auto& rvalue = rval.asArrayPtr();
            switch (op.type)
            {
            case TokenType::Equal:
                throw std::runtime_error("currently '==' operator not defined for arrays");
            case TokenType::NotEqual:
                throw std::runtime_error("currently '!=' operator not defined for arrays");
            default: throw UnsupportedOperation();
            }
        }

        throw UnsupportedOperation();
    }
    catch (UnsupportedOperation)
    {
        throw RuntimeError("unsupported operation", {
                               .category = ErrorCategory::TypeError, .kind = ErrorKind::UnsupportedOperation,
                               .identifier = getSymbolForOp(op.type), .primary = lval.description(),
                               .secondary = rval.description(),
                               .currentLine = line
                           });
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

void ExpressionStatementNode::evaluateNode(shared_ptr<Environment> env) const
{
    expressionStmt->evaluateNode(env);
}

void ExpressionStatementNode::debugPrint(int indentLevel) const
{
    cout << string(indentLevel * IndentSize, ' ') << "ExpressionStatement\n";
    expressionStmt->debugPrint(indentLevel + 1);
}

RuntimeValue VariableNode::evaluateNode(shared_ptr<Environment> env) const
{
    try
    {
        return env->getReference(identifierName).value;
    }
    catch (UndefinedVariable)
    {
        throw RuntimeError("undefined variable", {
                               .category = ErrorCategory::NameError, .kind = ErrorKind::VariableUndefined,
                               .identifier = identifierName, .currentLine = line
                           });
    }
}

RuntimeValue& VariableNode::getReference(shared_ptr<Environment> env)
{
    try
    {
        return env->getReference(identifierName).value;
    }
    catch (UndefinedVariable)
    {
        throw RuntimeError("undefined variable", {
                               .category = ErrorCategory::NameError, .kind = ErrorKind::VariableUndefined,
                               .identifier = identifierName, .currentLine = line
                           });
    }
}


void VariableNode::debugPrint(int i) const
{
    cout << "Variable(" << identifierName << ")\n";
}

RuntimeValue AssignmentNode::evaluateNode(shared_ptr<Environment> env) const
{
    RuntimeValue right = rvalue->evaluateNode(env);
    if (auto* var = dynamic_cast<VariableNode*>(lvalue.get()))
    {
        var->getReference(env) = right;
    }
    if (auto* indexElem = dynamic_cast<IndexNode*>(lvalue.get()))
    {
        indexElem->getReference(env) = right;
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

RuntimeValue CompoundAssignmentNode::evaluateNode(shared_ptr<Environment> env) const
{
    RuntimeValue rhs = rvalue->evaluateNode(env);
    if (auto* var = dynamic_cast<VariableNode*>(lvalue.get()))
    {
        auto& lhs = var->getReference(env);
        switch (op.type)
        {
        case TokenType::PlusEqual:
            if (lhs.isNumber() && rhs.isNumber())
            {
                return lhs.getNumberRef() += rhs.asNumber();;
            }
            if (lhs.isString() && rhs.isString())
            {
                return lhs.getStringRef() += rhs.asString();;
            }
            if (lhs.isBoolean() && rhs.isBoolean())
            {
                return lhs.getBoolRef() += rhs.asBoolean();
            }
            throw RuntimeError("unsupported operations", {
                                   .category = ErrorCategory::TypeError, .kind = ErrorKind::UnsupportedOperation,
                                   .identifier = "+", .primary = lhs.description(), .secondary = rhs.description(),
                                   .currentLine = line

                               });
            break;
        case TokenType::MinusEqual:
            if (lhs.isNumber() && rhs.isNumber())
            {
                return lhs.getNumberRef() -= rhs.asNumber();;
            }
            if (lhs.isBoolean() && rhs.isBoolean())
            {
                return lhs.getBoolRef() -= rhs.asBoolean();
            }
            throw RuntimeError("unsupported operations", {
                                   .category = ErrorCategory::TypeError, .kind = ErrorKind::UnsupportedOperation,
                                   .identifier = "-", .primary = lhs.description(), .secondary = rhs.description(),
                                   .currentLine = line

                               });
            break;
        case TokenType::MultiplyEqual:
            if (lhs.isNumber() && rhs.isNumber())
            {
                return lhs.getNumberRef() *= rhs.asNumber();;
            }
            if (lhs.isBoolean() && rhs.isBoolean())
            {
                return lhs.getBoolRef() *= rhs.asBoolean();
            }
            throw RuntimeError("unsupported operations", {
                                   .category = ErrorCategory::TypeError, .kind = ErrorKind::UnsupportedOperation,
                                   .identifier = "*", .primary = lhs.description(), .secondary = rhs.description(),
                                   .currentLine = line
                               });
            break;
        case TokenType::DivideEqual:
            if (lhs.isNumber() && rhs.isNumber())
            {
                if (rhs.asNumber())
                    return lhs.getNumberRef() /= rhs.asNumber();;
                throw RuntimeError("division by zero", {
                                       .category = ErrorCategory::ZeroDivisionError,
                                       .kind = ErrorKind::DivisionByZero,
                                       .currentLine = line
                                   });
            }
            if (lhs.isBoolean() && rhs.isBoolean())
            {
                if (rhs.asBoolean())
                    return lhs.getBoolRef() /= rhs.asBoolean();
                throw RuntimeError("division by zero", {
                                       .category = ErrorCategory::ZeroDivisionError,
                                       .kind = ErrorKind::DivisionByZero,
                                       .currentLine = line
                                   });
            }
            throw RuntimeError("unsupported operations", {
                                   .category = ErrorCategory::TypeError, .kind = ErrorKind::UnsupportedOperation,
                                   .identifier = "/", .primary = lhs.description(), .secondary = rhs.description()
                               });
            break;
        }
    }
    if (auto* indexElem = dynamic_cast<IndexNode*>(lvalue.get()))
    {
        auto& lhs = indexElem->getReference(env);
        switch (op.type)
        {
        case TokenType::PlusEqual:
            if (lhs.isNumber() && rhs.isNumber())
            {
                return lhs.getNumberRef() += rhs.asNumber();;
            }
            if (lhs.isString() && rhs.isString())
            {
                return lhs.getStringRef() += rhs.asString();;
            }
            if (lhs.isBoolean() && rhs.isBoolean())
            {
                return lhs.getBoolRef() += rhs.asBoolean();
            }
            throw RuntimeError("unsupported operations", {
                                   .category = ErrorCategory::TypeError, .kind = ErrorKind::UnsupportedOperation,
                                   .identifier = "+", .primary = lhs.description(), .secondary = rhs.description(),
                                   .currentLine = line

                               });
            break;
        case TokenType::MinusEqual:
            if (lhs.isNumber() && rhs.isNumber())
            {
                return lhs.getNumberRef() -= rhs.asNumber();;
            }
            if (lhs.isBoolean() && rhs.isBoolean())
            {
                return lhs.getBoolRef() -= rhs.asBoolean();
            }
            throw RuntimeError("unsupported operations", {
                                   .category = ErrorCategory::TypeError, .kind = ErrorKind::UnsupportedOperation,
                                   .identifier = "-", .primary = lhs.description(), .secondary = rhs.description(),
                                   .currentLine = line

                               });
            break;
        case TokenType::MultiplyEqual:
            if (lhs.isNumber() && rhs.isNumber())
            {
                return lhs.getNumberRef() *= rhs.asNumber();;
            }
            if (lhs.isBoolean() && rhs.isBoolean())
            {
                return lhs.getBoolRef() *= rhs.asBoolean();
            }
            throw RuntimeError("unsupported operations", {
                                   .category = ErrorCategory::TypeError, .kind = ErrorKind::UnsupportedOperation,
                                   .identifier = "*", .primary = lhs.description(), .secondary = rhs.description(),
                                   .currentLine = line
                               });
            break;
        case TokenType::DivideEqual:
            if (lhs.isNumber() && rhs.isNumber())
            {
                if (rhs.asNumber())
                    return lhs.getNumberRef() /= rhs.asNumber();;
                throw RuntimeError("division by zero", {
                                       .category = ErrorCategory::ZeroDivisionError,
                                       .kind = ErrorKind::DivisionByZero,
                                       .currentLine = line,
                                   });
            }
            if (lhs.isBoolean() && rhs.isBoolean())
            {
                if (rhs.asBoolean())
                    return lhs.getBoolRef() /= rhs.asBoolean();
                throw RuntimeError("division by zero", {
                                       .category = ErrorCategory::ZeroDivisionError,
                                       .kind = ErrorKind::DivisionByZero,
                                       .currentLine = line,
                                   });
            }
            throw RuntimeError("unsupported operations", {
                                   .category = ErrorCategory::TypeError, .kind = ErrorKind::UnsupportedOperation,
                                   .identifier = "/", .primary = lhs.description(), .secondary = rhs.description()
                               });
            break;
        }
    }
}

void CompoundAssignmentNode::debugPrint(int indentLevel) const
{
    cout << "CompoundAssignment";
    lvalue->debugPrint(indentLevel + 1);
    rvalue->debugPrint(indentLevel + 1);
}

void DeclarationNode::evaluateNode(shared_ptr<Environment> env) const
{
    RuntimeValue right = rvalue->evaluateNode(env);
    if (auto* var = dynamic_cast<VariableNode*>(lvalue.get()))
    {
        try
        {
            env->declare(var->getIdentifierName(), {right, line});
        }
        catch (Redeclaration)
        {
            throw RuntimeError("redeclaration of variable", {
                                   .category = ErrorCategory::RedeclarationError,
                                   .kind = ErrorKind::VariableRedeclaration,
                                   .identifier = var->getIdentifierName(),
                                   .currentLine = line,
                                   .previousLine = env->getReference(var->getIdentifierName()).declarationLine
                               });
        }
    }
}

void DeclarationNode::debugPrint(int indentLevel) const
{
    cout << " Let\n";
    cout << string(IndentSize * indentLevel, ' ');
    lvalue->debugPrint(indentLevel + 1);
    cout << string(IndentSize * indentLevel, ' ');
    rvalue->debugPrint(indentLevel + 1);
}

void IfNode::evaluateNode(shared_ptr<Environment> env) const
{
    auto currentEnv = std::make_shared<Environment>();
    currentEnv->parent = env;

    RuntimeValue truthVal = condition->evaluateNode(currentEnv);

    if (truthVal.isTruthy())
        thenStatement->evaluateNode(currentEnv);

    else
    {
        if (elseStatement)
            elseStatement->evaluateNode(currentEnv);
    }
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

void WhileNode::evaluateNode(shared_ptr<Environment> env) const
{
    auto currentEnv = std::make_shared<Environment>();
    currentEnv->parent = env;

    while (condition->evaluateNode(currentEnv).isTruthy())
    {
        try
        {
            // ScopedEnvironment local(currentEnv);
            statement->evaluateNode(currentEnv);
        }
        catch (BreakSignal b)
        {
            break;
        }
        catch (ContinueSignal c)
        {
            continue;
        }
    }
}

void WhileNode::debugPrint(int indentLevel) const
{
    cout << "While\n";
    cout << string(IndentSize * indentLevel, ' ');
    condition->debugPrint(indentLevel + 1);
    statement->debugPrint(indentLevel + 1);
}

void ForNode::evaluateNode(shared_ptr<Environment> env) const
{
    auto currentEnv = std::make_shared<Environment>();
    currentEnv->parent = env;
    // ScopedEnvironment local(scopes);
    if (initializer)
        initializer->evaluateNode(currentEnv);

    while (true)
    {
        try
        {
            if (condition)
                if (!condition->evaluateNode(currentEnv).isTruthy())
                    break;

            statement->evaluateNode(currentEnv);
            if (expr) expr->evaluateNode(currentEnv);
        }
        catch (BreakSignal b)
        {
            break;
        }
        catch (ContinueSignal c)
        {
            if (expr) expr->evaluateNode(currentEnv);
            continue;
        }
    }
}

void ForNode::debugPrint(int indentLevel) const
{
    cout << "For(";
    if (initializer) initializer->debugPrint(indentLevel + 1);
    if (condition) condition->debugPrint(indentLevel + 1);
    if (expr) expr->debugPrint(indentLevel + 1);
    cout << ")";
    statement->debugPrint(indentLevel + 1);
}

void BreakNode::evaluateNode(shared_ptr<Environment> env) const
{
    throw BreakSignal();
}

void BreakNode::debugPrint(int indentLevel) const
{
    cout << "Break\n";
}

void ContinueNode::evaluateNode(shared_ptr<Environment> env) const
{
    throw ContinueSignal();
}

void ContinueNode::debugPrint(int indentLevel) const
{
    cout << "Continue\n";
}

void BlockNode::evaluateNode(shared_ptr<Environment> env) const
{
    // ScopedEnvironment local(env);
    auto currentEnv = std::make_shared<Environment>();
    currentEnv->parent = env;
    for (auto& statement : statements)
    {
        statement->evaluateNode(currentEnv);
    }
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

RuntimeValue FunctionCallNode::evaluateNode(shared_ptr<Environment> env) const
{
    if (auto* func = dynamic_cast<VariableNode*>(identifier.get()))
    {
        const string& f_name = func->getIdentifierName();
        try
        {
            RuntimeValue obj = env->getReference(f_name).value;

            if (obj.isFunctionObj())
            {
                auto function = obj.asFunctionObj();
                vector<RuntimeValue> args;
                if (!function.variadic)
                {
                    if (arguments.size() == function.parameters.size())
                    {
                        for (const auto& argument : arguments)
                            args.push_back(argument->evaluateNode(env));

                        return function.call(args);
                    }
                }
                else
                {
                    if (arguments.size() <= function.parameters.size())
                        validateArity(function.parameters.size() + 1, arguments.size(), f_name, line);
                    else
                    {
                        vector<RuntimeValue> restArgs;
                        for (int i = 0; i < arguments.size(); ++i)
                        {
                            if (i < function.parameters.size())
                                args.push_back(arguments[i]->evaluateNode(env));
                            else
                                restArgs.push_back(arguments[i]->evaluateNode(env));
                        }
                        return function.call(args, restArgs);
                    }
                }
                validateArity(function.parameters.size(), arguments.size(), f_name, line);
            }
            throw RuntimeError("object is not callable", {
                                   .category = ErrorCategory::TypeError,
                                   .kind = ErrorKind::NotCallable,
                                   .primary = identifier->evaluateNode(env).description(),
                                   .currentLine = line,
                               });
        }
        catch (UndefinedVariable)
        {
            if (f_name == "abs")
            {
                if (arguments.size() == 1)
                    return abs(static_cast<int>(arguments[0]->evaluateNode(env).asNumber()));
                validateArity(1, arguments.size(), "abs", line);
            }
            if (f_name == "max")
            {
                if (arguments.size() == 2)
                    return std::max(arguments[0]->evaluateNode(env).asNumber(),
                                    arguments[1]->evaluateNode(env).asNumber());
                validateArity(2, arguments.size(), "max", line);
            }
            if (f_name == "min")
            {
                if (arguments.size() == 2)
                    return std::min(arguments[0]->evaluateNode(env).asNumber(),
                                    arguments[1]->evaluateNode(env).asNumber());
                validateArity(2, arguments.size(), "min", line);
            }
            // if (f_name == "avg")
            // {
            //     if (!arguments.empty())
            //     {
            //         double avg = 0;
            //         for (const auto& argument : arguments)
            //             avg += argument->evaluateNode(env).asNumber();
            //         avg = avg / arguments.size();
            //         return avg;
            //     }
            //     validateArity(1, arguments.size(), "avg", line);
            // }
            if (f_name == "print")
            {
                if (!arguments.empty())
                {
                    for (const auto& argument : arguments)
                    {
                        RuntimeValue arg = argument->evaluateNode(env);
                        printRuntimeValue(arg);
                        cout << " ";
                    }
                }
                else cout << "";
                return {};
            }

            if (f_name == "println")
            {
                if (!arguments.empty())
                {
                    for (const auto& argument : arguments)
                    {
                        RuntimeValue arg = argument->evaluateNode(env);
                        printRuntimeValue(arg);
                        cout << " ";
                    }
                    cout << '\n';
                }
                else cout << "";
                return {};
            }
            if (f_name == "size")
            {
                if (arguments.size() == 1)
                {
                    RuntimeValue arg = arguments[0]->evaluateNode(env);
                    if (arg.isArray())
                        return static_cast<double>(arg.asArrayPtr()->size());
                    if (arg.isString())
                        return static_cast<double>(arg.asString().size());
                    throw std::runtime_error("invalid operand for size method");
                }
                validateArity(1, arguments.size(), "size", line);
            }
            if (f_name == "push")
            {
                if (arguments.size() == 2)
                {
                    if (auto* var = dynamic_cast<VariableNode*>(arguments[0].get()))
                    {
                        RuntimeValue& arr = var->getReference(env);
                        if (arr.isArray())
                        {
                            RuntimeValue value = arguments[1]->evaluateNode(env);
                            arr.asArrayPtr()->push_back(value);
                            return {};
                        }
                        throw std::runtime_error("invalid operand for push");
                    }
                }
            }
            if (f_name == "read")
            {
                if (!arguments.empty())
                {
                    for (const auto& argument : arguments)
                    {
                        RuntimeValue arg = argument->evaluateNode(env);
                        printRuntimeValue(arg);
                        cout << " ";
                    }
                    cout << '\n';
                }
                else cout << "";
                string input;
                getline(std::cin, input, ' ');
                return input;
            }
            if (f_name == "readln")
            {
                if (!arguments.empty())
                {
                    for (const auto& argument : arguments)
                    {
                        RuntimeValue arg = argument->evaluateNode(env);
                        printRuntimeValue(arg);
                        cout << " ";
                    }
                    cout << '\n';
                }
                else cout << "";
                string input;
                getline(std::cin, input);
                return input;
            }
            if (f_name == "type")
            {
                if (arguments.size() == 1)
                {
                    const RuntimeValue& obj = arguments[0]->evaluateNode(env);
                    return obj.description();
                }
                validateArity(1, arguments.size(), f_name, line);
            }
            throw RuntimeError("function is not defined", {
                                   .category = ErrorCategory::NameError, .kind = ErrorKind::FunctionUndefined,
                                   .identifier = f_name, .currentLine = line
                               });
        }
    }
    if (auto* func = dynamic_cast<FunctionCallNode*>(identifier.get()))
    {
        RuntimeValue obj = identifier->evaluateNode(env);
        if (obj.isFunctionObj())
        {
            auto function = obj.asFunctionObj();
            vector<RuntimeValue> args;
            if (arguments.size() == function.parameters.size())
            {
                for (const auto& argument : arguments)
                    args.push_back(argument->evaluateNode(env));

                return function.call(args);
            }
            validateArity(function.parameters.size(), arguments.size(), function.f_name, line);
        }
        throw RuntimeError("object is not callable", {
                               .category = ErrorCategory::TypeError,
                               .kind = ErrorKind::NotCallable,
                               .primary = identifier->evaluateNode(env).description(),
                               .currentLine = line,
                           });
    }

    throw RuntimeError("object is not callable", {
                           .category = ErrorCategory::TypeError,
                           .kind = ErrorKind::NotCallable,
                           .primary = identifier->evaluateNode(env).description(),
                           .currentLine = line,
                       });
}


void FunctionCallNode::debugPrint(int indentLevel) const
{
    if (auto* func = dynamic_cast<VariableNode*>(identifier.get()))
    {
        cout << "FunctionCall{\n";
        cout << "Function(" << func->getIdentifierName() << ")\n";
        for (const auto& argument : arguments)
        {
            cout << string(IndentSize * indentLevel, ' ');
            argument->debugPrint(indentLevel + 1);
        }
        cout << "}\n";
    }
    if (auto* func = dynamic_cast<FunctionCallNode*>(identifier.get()))
    {
        cout << "FunctionCall{\n";

        func->debugPrint(indentLevel + 1);
        for (const auto& argument : arguments)
        {
            cout << string(IndentSize * indentLevel, ' ');
            argument->debugPrint(indentLevel + 1);
        }
        cout << "}\n";
    }
}

void validateArity(int expected_arguments, int given_arguments, string f_name, int callLine)
{
    if (given_arguments > expected_arguments)
    {
        throw RuntimeError("too many arguments", {
                               .category = ErrorCategory::ArityError,
                               .kind = ErrorKind::TooManyArguments,
                               .identifier = f_name,
                               .currentLine = callLine,
                               .expected = expected_arguments,
                               .actual = given_arguments
                           });
    }
    if (given_arguments < expected_arguments)
    {
        throw RuntimeError("too few arguments", {
                               .category = ErrorCategory::ArityError,
                               .kind = ErrorKind::TooFewArguments,
                               .identifier = f_name,
                               .currentLine = callLine,
                               .expected = expected_arguments,
                               .actual = given_arguments
                           });
    }
}

void FunctionDeclarationNode::evaluateNode(shared_ptr<Environment> env) const
{
    auto iter = functions.find(identifier);
    if (iter == functions.end())
    {
        auto* body_ptr = dynamic_cast<BlockNode*>(body.get());
        env->declare(identifier, {
                         FunctionObject{identifier, parameters, body_ptr, env, variadic, variadicParamName}, line
                     });
    }
    else
        throw RuntimeError("redeclaration of function", {
                               .category = ErrorCategory::RedeclarationError,
                               .kind = ErrorKind::FunctionRedeclaration,
                               .identifier = identifier,
                               .currentLine = line
                           });
}

void FunctionDeclarationNode::debugPrint(int indentLevel) const
{
    cout << "FunctionDeclaration\n";
    cout << string(IndentSize * indentLevel, ' ') << "Parameters(";
    for (const auto& parameter : parameters)
    {
        cout << parameter;
        cout << " ";
    }
    if (variadic)
        cout << "...args";
    cout << ")\n";
    cout << string(IndentSize * indentLevel, ' ');
    body->debugPrint(indentLevel + 1);
}

void ReturnNode::evaluateNode(shared_ptr<Environment> env) const
{
    if (returnStatement)
        throw ReturnSignal(returnStatement->evaluateNode(env));
    throw ReturnSignal({});
}

void ReturnNode::debugPrint(int indentLevel) const
{
    cout << "Return\n";
    cout << string(IndentSize * indentLevel, ' ');
    if (returnStatement)
        returnStatement->debugPrint(indentLevel + 1);
}

void ProgramNode::evaluateNode(shared_ptr<Environment> env)
{
    for (auto& stmt : statements)
        stmt->evaluateNode(env);
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

void EmptyNode::evaluateNode(shared_ptr<Environment> env) const
{
}

void EmptyNode::debugPrint(int indentLevel) const
{
    cout << string(indentLevel * IndentSize, ' ') << "EmptyStatement\n";
}

string getErrorCategoryString(ErrorCategory category)
{
    switch (category)
    {
    case ErrorCategory::NameError:
        return "NameError";
    case ErrorCategory::ZeroDivisionError:
        return "ZeroDivisionError";
    case ErrorCategory::TypeError:
        return "TypeError";
    case ErrorCategory::RedeclarationError:
        return "RedeclarationError";
    case ErrorCategory::ArityError:
        return "ArityError";
    case ErrorCategory::ValueError:
        return "ValueError";
    case ErrorCategory::IndexError:
        return "IndexError";
    }
}

string getErrorKindString(ErrorKind kind)
{
    switch (kind)
    {
    case ErrorKind::VariableUndefined:
        return "VariableUndefined";
    case ErrorKind::FunctionUndefined:
        return "FunctionUndefined";
    case ErrorKind::InvalidIndexType:
        return "InvalidIndexType";
    case ErrorKind::OperandTypeMismatch:
        return "OperandTypeMismatch";
    case ErrorKind::UnsupportedOperation:
        return "UnsupportedOperation";
    case ErrorKind::NotCallable:
        return "NotCallable";
    case ErrorKind::NotSubscriptable:
        return "NotSubscriptable";
    case ErrorKind::TooFewArguments:
        return "TooFewArguments";
    case ErrorKind::TooManyArguments:
        return "TooManyArguments";
    case ErrorKind::DivisionByZero:
        return "DivisionByZero";
    case ErrorKind::IndexOutOfBounds:
        return "IndexOutOfBounds";
    case ErrorKind::VariableRedeclaration:
        return "VariableRedeclaration";
    case ErrorKind::FunctionRedeclaration:
        return "FunctionRedeclaration";
    }
}
