#include "ast.h"
#include <stdexcept>
#include <iostream>

using std::cout;

constexpr int IndentSize = 2;
FunctionTable functions;

EvalResult NumberNode::evaluateNode(shared_ptr<Environment> env) const
{
    return {.hasValue = true, .value = value};
}

void NumberNode::debugPrint(int indentLevel) const
{
    cout << "Number(" << value << ")\n";
}

EvalResult StringNode::evaluateNode(shared_ptr<Environment> env) const
{
    return {true, value};
}

void StringNode::debugPrint(int indentLevel) const
{
    cout << "String(" << value << ")\n";
}

EvalResult UnaryNode::evaluateNode(shared_ptr<Environment> env) const
{
    RuntimeValue value = child->evaluateNode(env).value;
    if (op.type == TokenType::Not)
    {
        return {true, !value.isTruthy()};
    }
    if (value.isNumber())
    {
        if (op.type == TokenType::Minus)
            return {true, -value.asNumber()};
        if (op.type == TokenType::Plus)
            return {true, value};
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

EvalResult BooleanNode::evaluateNode(shared_ptr<Environment> env) const
{
    return {true, value};
}

EvalResult NullNode::evaluateNode(shared_ptr<Environment> env) const
{
    return {true, {}};
}

void NullNode::debugPrint(int indentLevel) const
{
    cout << "Null";
}


EvalResult ArrayNode::evaluateNode(shared_ptr<Environment> env) const
{
    shared_ptr<vector<RuntimeValue>> arrayPtr = std::make_shared<vector<RuntimeValue>>();

    for (int i = 0; i < value.size(); ++i)
    {
        arrayPtr->push_back(value[i]->evaluateNode(env).value);
    }
    return {true, arrayPtr};
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

EvalResult IndexNode::evaluateNode(shared_ptr<Environment> env) const
{
    const RuntimeValue& object = operand->evaluateNode(env).value;
    const RuntimeValue indexV = indexExp->evaluateNode(env).value;
    if (indexV.isNumber())
    {
        const int index = static_cast<int>(indexV.asNumber());
        if (object.isArray())
        {
            if (index < object.asArrayPtr()->size())
                return {true, object.asArrayPtr()->at(index)};
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
                return {true, object.asString().at(index)};
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
    const RuntimeValue indexV = indexExp->evaluateNode(env).value;
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

EvalResult BinaryNode::evaluateNode(shared_ptr<Environment> env) const
{
    if (op.type == TokenType::AndAnd)
    {
        RuntimeValue lval = left->evaluateNode(env).value;
        if (!lval.isTruthy())
            return {true, false};
        RuntimeValue rval = right->evaluateNode(env).value;

        return {true, rval.isTruthy()};
    }
    if (op.type == TokenType::OrOr)
    {
        RuntimeValue lval = left->evaluateNode(env).value;
        if (lval.isTruthy())
            return {true, true};
        RuntimeValue rval = right->evaluateNode(env).value;
        return {true, rval.isTruthy()};
    }
    RuntimeValue lval = left->evaluateNode(env).value;
    RuntimeValue rval = right->evaluateNode(env).value;

    try
    {
        if (lval.isNumber() && rval.isNumber())
        {
            const auto& lvalue = lval.asNumber();
            const auto& rvalue = rval.asNumber();
            switch (op.type)
            {
            case TokenType::Plus:
                return {true, lvalue + rvalue};
            case TokenType::Minus:
                return {true, lvalue - rvalue};
            case TokenType::Multiply:
                return {true, lvalue * rvalue};
            case TokenType::Modulo:
                if (rvalue)
                    return {true, fmod(lvalue, rvalue)};
                throw RuntimeError("cannot divide by zero", {
                                       .category = ErrorCategory::ZeroDivisionError,
                                       .kind = ErrorKind::DivisionByZero,
                                       .currentLine = line
                                   });
            case TokenType::Divide:
                if (rvalue)
                    return {true, lvalue / rvalue};
                throw RuntimeError("cannot divide by zero", {
                                       .category = ErrorCategory::ZeroDivisionError,
                                       .kind = ErrorKind::DivisionByZero,
                                       .currentLine = line
                                   });
            case TokenType::Greater:
                return {true, lvalue > rvalue};
            case TokenType::GreaterEqual:
                return {true, lvalue >= rvalue};
            case TokenType::LessEqual:
                return {true, lvalue <= rvalue};
            case TokenType::Less:
                return {true, lvalue < rvalue};
            case TokenType::Equal:
                return {true, lvalue == rvalue};
            case TokenType::NotEqual:
                return {true, lvalue != rvalue};
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
                return {true, lvalue + rvalue};
            case TokenType::Equal:
                return {true, lvalue == rvalue};
            case TokenType::NotEqual:
                return {true, lvalue != rvalue};
            case TokenType::Less:
                return {true, lvalue < rvalue};
            case TokenType::LessEqual:
                return {true, lvalue <= rvalue};
            case TokenType::Greater:
                return {true, lvalue > rvalue};
            case TokenType::GreaterEqual:
                return {true, lvalue >= rvalue};
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
                return {true, lvalue + rvalue};
            case TokenType::Minus:
                return {true, lvalue - rvalue};
            case TokenType::Multiply:
                return {true, lvalue * rvalue};
            case TokenType::Divide:
                {
                    if (rvalue)
                        return {true, lvalue / rvalue};
                    throw RuntimeError("cannot divide by zero", {
                                           .category = ErrorCategory::ZeroDivisionError,
                                           .kind = ErrorKind::DivisionByZero,
                                           .currentLine = line
                                       });
                }
            case TokenType::Greater:
                return {true, lvalue > rvalue};
            case TokenType::GreaterEqual:
                return {true, lvalue >= rvalue};
            case TokenType::LessEqual:
                return {true, lvalue <= rvalue};
            case TokenType::Less:
                return {true, lvalue < rvalue};
            case TokenType::Equal:
                return {true, lvalue == rvalue};
            case TokenType::NotEqual:
                return {true, lvalue != rvalue};
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

EvalResult ExpressionStatementNode::evaluateNode(shared_ptr<Environment> env) const
{
    RuntimeValue val = expressionStmt->evaluateNode(env).value;
    // printRuntimeValue(val);
    // cout << "\n";
    return {true, val};
}

void ExpressionStatementNode::debugPrint(int indentLevel) const
{
    cout << string(indentLevel * IndentSize, ' ') << "ExpressionStatement\n";
    expressionStmt->debugPrint(indentLevel + 1);
}

EvalResult VariableNode::evaluateNode(shared_ptr<Environment> env) const
{
    try
    {
        return {true, env->getReference(identifierName).value};
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

EvalResult AssignmentNode::evaluateNode(shared_ptr<Environment> env) const
{
    RuntimeValue right = rvalue->evaluateNode(env).value;
    if (auto* var = dynamic_cast<VariableNode*>(lvalue.get()))
    {
        var->getReference(env) = right;
    }
    if (auto* indexElem = dynamic_cast<IndexNode*>(lvalue.get()))
    {
        indexElem->getReference(env) = right;
    }
    return {false, right};
}

void AssignmentNode::debugPrint(int indentLevel) const
{
    cout << "Assignment(=)\n";
    cout << string(IndentSize * indentLevel, ' ');
    lvalue->debugPrint(indentLevel + 1);
    cout << string(IndentSize * indentLevel, ' ');
    rvalue->debugPrint(indentLevel + 1);
}

EvalResult CompoundAssignmentNode::evaluateNode(shared_ptr<Environment> env) const
{
    RuntimeValue rhs = rvalue->evaluateNode(env).value;
    if (auto* var = dynamic_cast<VariableNode*>(lvalue.get()))
    {
        auto& lhs = var->getReference(env);
        switch (op.type)
        {
        case TokenType::PlusEqual:
            if (lhs.isNumber() && rhs.isNumber())
            {
                return {false, lhs.getNumberRef() += rhs.asNumber()};
            }
            if (lhs.isString() && rhs.isString())
            {
                return {false, lhs.getStringRef() += rhs.asString()};
            }
            if (lhs.isBoolean() && rhs.isBoolean())
            {
                return {false, lhs.getBoolRef() += rhs.asBoolean()};
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
                return {false, lhs.getNumberRef() -= rhs.asNumber()};
            }
            if (lhs.isBoolean() && rhs.isBoolean())
            {
                return {false, lhs.getBoolRef() -= rhs.asBoolean()};
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
                return {false, lhs.getNumberRef() *= rhs.asNumber()};
            }
            if (lhs.isBoolean() && rhs.isBoolean())
            {
                return {false, lhs.getBoolRef() *= rhs.asBoolean()};
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
                    return {false, lhs.getNumberRef() /= rhs.asNumber()};
                throw RuntimeError("division by zero", {
                                       .category = ErrorCategory::ZeroDivisionError,
                                       .kind = ErrorKind::DivisionByZero,
                                       .currentLine = line
                                   });
            }
            if (lhs.isBoolean() && rhs.isBoolean())
            {
                if (rhs.asBoolean())
                    return {false, lhs.getBoolRef() /= rhs.asBoolean()};
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
                return {false, lhs.getNumberRef() += rhs.asNumber()};
            }
            if (lhs.isString() && rhs.isString())
            {
                return {false, lhs.getStringRef() += rhs.asString()};
            }
            if (lhs.isBoolean() && rhs.isBoolean())
            {
                return {false, lhs.getBoolRef() += rhs.asBoolean()};
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
                return {false, lhs.getNumberRef() -= rhs.asNumber()};
            }
            if (lhs.isBoolean() && rhs.isBoolean())
            {
                return {false, lhs.getBoolRef() -= rhs.asBoolean()};
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
                return {false, lhs.getNumberRef() *= rhs.asNumber()};
            }
            if (lhs.isBoolean() && rhs.isBoolean())
            {
                return {false, lhs.getBoolRef() *= rhs.asBoolean()};
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
                    return {false, lhs.getNumberRef() /= rhs.asNumber()};
                throw RuntimeError("division by zero", {
                                       .category = ErrorCategory::ZeroDivisionError,
                                       .kind = ErrorKind::DivisionByZero,
                                       .currentLine = line,
                                   });
            }
            if (lhs.isBoolean() && rhs.isBoolean())
            {
                if (rhs.asBoolean())
                    return {false, lhs.getBoolRef() /= rhs.asBoolean()};
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

EvalResult DeclarationNode::evaluateNode(shared_ptr<Environment> env) const
{
    RuntimeValue right = rvalue->evaluateNode(env).value;
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
    return {false};
}

void DeclarationNode::debugPrint(int indentLevel) const
{
    cout << " Let\n";
    cout << string(IndentSize * indentLevel, ' ');
    lvalue->debugPrint(indentLevel + 1);
    cout << string(IndentSize * indentLevel, ' ');
    rvalue->debugPrint(indentLevel + 1);
}

EvalResult IfNode::evaluateNode(shared_ptr<Environment> env) const
{
    auto currentEnv = std::make_shared<Environment>();
    currentEnv->parent = env;

    if (condition->evaluateNode(currentEnv).value.isTruthy())
        thenStatement->evaluateNode(currentEnv);

    else
    {
        if (elseStatement)
            elseStatement->evaluateNode(currentEnv);
    }
    return {false};
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

EvalResult WhileNode::evaluateNode(shared_ptr<Environment> env) const
{
    auto currentEnv = std::make_shared<Environment>();
    currentEnv->parent = env;

    while (condition->evaluateNode(currentEnv).value.isTruthy())
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
    return {false};
}

void WhileNode::debugPrint(int indentLevel) const
{
    cout << "While\n";
    cout << string(IndentSize * indentLevel, ' ');
    condition->debugPrint(indentLevel + 1);
    statement->debugPrint(indentLevel + 1);
}

EvalResult ForNode::evaluateNode(shared_ptr<Environment> env) const
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
                if (!condition->evaluateNode(currentEnv).value.isTruthy())
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
    return {false};
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

EvalResult BreakNode::evaluateNode(shared_ptr<Environment> env) const
{
    throw BreakSignal();
}

void BreakNode::debugPrint(int indentLevel) const
{
    cout << "Break\n";
}

EvalResult ContinueNode::evaluateNode(shared_ptr<Environment> env) const
{
    throw ContinueSignal();
}

void ContinueNode::debugPrint(int indentLevel) const
{
    cout << "Continue\n";
}

EvalResult BlockNode::evaluateNode(shared_ptr<Environment> env) const
{
    // ScopedEnvironment local(env);
    auto currentEnv = std::make_shared<Environment>();
    currentEnv->parent = env;
    for (auto& statement : statements)
    {
        statement->evaluateNode(currentEnv);
    }
    return {false};
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

EvalResult FunctionCallNode::evaluateNode(shared_ptr<Environment> env) const
{
    if (auto* func = dynamic_cast<VariableNode*>(identifier.get()))
    {
        const string& f_name = func->getIdentifierName();
        try
        {
            RuntimeValue obj = env->getReference(f_name).value;
            if (obj.isNativeFunction())
            {
                auto function = obj.asNativeFunction();
                vector<RuntimeValue> args;
                for (const auto& argument : arguments)
                    args.push_back(argument->evaluateNode(env).value);

                return {true, function.call(args)};
            }
            if (obj.isFunctionObj())
            {
                auto function = obj.asFunctionObj();
                vector<RuntimeValue> args;
                if (!function.variadic)
                {
                    if (arguments.size() == function.parameters.size())
                    {
                        for (const auto& argument : arguments)
                            args.push_back(argument->evaluateNode(env).value);

                        try
                        {
                            return {true, function.call(args)};
                        }
                        catch (MaxRecursion)
                        {
                            throw RuntimeError("maximum recursion depth reached", {
                                                   .category = ErrorCategory::RecursionError,
                                                   .kind = ErrorKind::MaxRecursionLimit,
                                                   .identifier = f_name,
                                                   .currentLine = line,
                                               });
                        }
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
                                args.push_back(arguments[i]->evaluateNode(env).value);
                            else
                                restArgs.push_back(arguments[i]->evaluateNode(env).value);
                        }
                        return {true, function.call(args, restArgs)};
                    }
                }
                validateArity(function.parameters.size(), arguments.size(), f_name, line);
            }
            throw RuntimeError("object is not callable", {
                                   .category = ErrorCategory::TypeError,
                                   .kind = ErrorKind::NotCallable,
                                   .primary = identifier->evaluateNode(env).value.description(),
                                   .currentLine = line,
                               });
        }
        catch (UndefinedVariable)
        {
            throw RuntimeError("function is not defined", {
                                   .category = ErrorCategory::NameError, .kind = ErrorKind::FunctionUndefined,
                                   .identifier = f_name, .currentLine = line
                               });
        }
    }
    if (auto* func = dynamic_cast<FunctionCallNode*>(identifier.get()))
    {
        RuntimeValue obj = identifier->evaluateNode(env).value;
        if (obj.isFunctionObj())
        {
            auto function = obj.asFunctionObj();
            vector<RuntimeValue> args;
            if (arguments.size() == function.parameters.size())
            {
                for (const auto& argument : arguments)
                    args.push_back(argument->evaluateNode(env).value);

                return {true, function.call(args)};
            }
            validateArity(function.parameters.size(), arguments.size(), function.f_name, line);
        }
        throw RuntimeError("object is not callable", {
                               .category = ErrorCategory::TypeError,
                               .kind = ErrorKind::NotCallable,
                               .primary = identifier->evaluateNode(env).value.description(),
                               .currentLine = line,
                           });
    }

    throw RuntimeError("object is not callable", {
                           .category = ErrorCategory::TypeError,
                           .kind = ErrorKind::NotCallable,
                           .primary = identifier->evaluateNode(env).value.description(),
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

EvalResult FunctionDeclarationNode::evaluateNode(shared_ptr<Environment> env) const
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
    return {false};
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

EvalResult ReturnNode::evaluateNode(shared_ptr<Environment> env) const
{
    if (returnStatement)
        throw ReturnSignal(returnStatement->evaluateNode(env).value);
    throw ReturnSignal({});
}

void ReturnNode::debugPrint(int indentLevel) const
{
    cout << "Return\n";
    cout << string(IndentSize * indentLevel, ' ');
    if (returnStatement)
        returnStatement->debugPrint(indentLevel + 1);
}

EvalResult ProgramNode::evaluateNode(shared_ptr<Environment> env)
{
    for (auto& stmt : statements)
        stmt->evaluateNode(env);
    return {false};
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

EvalResult EmptyNode::evaluateNode(shared_ptr<Environment> env) const
{
    return {false};
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
    case ErrorCategory::RecursionError:
        return "RecursionError";
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
