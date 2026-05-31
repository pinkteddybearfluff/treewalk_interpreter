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
    const RuntimeValue& object = operand->evaluateNode(scopes);
    const RuntimeValue indexV = indexExp->evaluateNode(scopes);
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
                return RuntimeValue(
                    string(1, object.asString().at(index)));
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

RuntimeValue& IndexNode::getReference(EnvironmentStack& scopes)
{
    const RuntimeValue indexV = indexExp->evaluateNode(scopes);
    if (indexV.isNumber())
    {
        if (auto* var = dynamic_cast<VariableNode*>(operand.get()))
        {
            const RuntimeValue& object = var->getReference(scopes);
            if (object.isArray())
                return object.asArrayPtr()->at(indexV.asNumber());
        }
        if (auto* indexNode = dynamic_cast<IndexNode*>(operand.get()))
        {
            const RuntimeValue& object = indexNode->getReference(scopes);

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

RuntimeValue BinaryNode::evaluateNode(EnvironmentStack& scopes) const
{
    RuntimeValue lval = left->evaluateNode(scopes);
    RuntimeValue rval = right->evaluateNode(scopes);

    try
    {
        if (lval.isNumber() && rval.isNumber())
        {
            const auto& lvalue = lval.asNumber();
            const auto& rvalue = rval.asNumber();
            switch (op.type)
            {
            case TokenType::Plus:
                return RuntimeValue(lvalue + rvalue);
            case TokenType::Minus:
                return RuntimeValue(lvalue - rvalue);
            case TokenType::Multiply:
                return RuntimeValue(lvalue * rvalue);
            case TokenType::Divide:
                {
                    if (rvalue)
                        return RuntimeValue(lvalue / rvalue);
                    throw RuntimeError("cannot divide by zero", {
                                           .category = ErrorCategory::ZeroDivisionError,
                                           .kind = ErrorKind::DivisionByZero,
                                           .currentLine = line
                                       });
                }
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
                return RuntimeValue(static_cast<double>(lvalue + rvalue));
            case TokenType::Minus:
                return RuntimeValue(static_cast<double>(lvalue - rvalue));
            case TokenType::Multiply:
                return RuntimeValue(static_cast<double>(lvalue * rvalue));
            case TokenType::Divide:
                {
                    if (rvalue)
                        return RuntimeValue(static_cast<double>(lvalue / rvalue));
                    throw RuntimeError("cannot divide by zero", {
                                           .category = ErrorCategory::ZeroDivisionError,
                                           .kind = ErrorKind::DivisionByZero,
                                           .currentLine = line
                                       });
                }
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
    try
    {
        return scopes.get(identifierName).value;
    }
    catch (UndefinedVariable)
    {
        throw RuntimeError("undefined variable", {
                               .category = ErrorCategory::NameError, .kind = ErrorKind::VariableUndefined,
                               .identifier = identifierName, .currentLine = line
                           });
    }
}

RuntimeValue& VariableNode::getReference(EnvironmentStack& scopes)
{
    try
    {
        return scopes.get(identifierName).value;
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
    if (auto* var = dynamic_cast<VariableNode*>(lvalue.get()))
        try
        {
            scopes.declare(var->getIdentifierName(), {right, line});
        }
        catch (Redeclaration)
        {
            throw RuntimeError("redeclaration of variable", {
                                   .category = ErrorCategory::RedeclarationError,
                                   .kind = ErrorKind::VariableRedeclaration,
                                   .identifier = var->getIdentifierName(),
                                   .currentLine = line,
                                   .previousLine = scopes.get(var->getIdentifierName()).declarationLine
                               });
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

void IfNode::evaluateNode(EnvironmentStack& scopes) const
{
    scopes.pushScope();
    RuntimeValue truthVal = condition->evaluateNode(scopes);

    if (truthVal.isTruthy())
        thenStatement->evaluateNode(scopes);

    else
    {
        if (elseStatement)
            elseStatement->evaluateNode(scopes);
    }

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
    while (condition->evaluateNode(scopes).isTruthy())
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
}

void WhileNode::debugPrint(int indentLevel) const
{
    cout << "While\n";
    cout << string(IndentSize * indentLevel, ' ');
    condition->debugPrint(indentLevel + 1);
    statement->debugPrint(indentLevel + 1);
}

void ForNode::evaluateNode(EnvironmentStack& scopes) const
{
    scopes.pushScope();
    if (initializer)
        initializer->evaluateNode(scopes);

    while (true)
    {
        try
        {
            if (condition)
                if (!condition->evaluateNode(scopes).isTruthy())
                    break;

            statement->evaluateNode(scopes);
            if (expr) expr->evaluateNode(scopes);
        }
        catch (BreakSignal b)
        {
            scopes.popScope();
            break;
        }
        catch (ContinueSignal c)
        {
            if (expr) expr->evaluateNode(scopes);
            scopes.popScope();
            continue;
        }
    }
    scopes.popScope();
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
                    env[parameters[i]] = {RuntimeValue(arguments[i]->evaluateNode(scopes)), line};
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
            validateArity(iter->second->getParametersSize(), arguments.size(), identifierName, line);
        }
        if (identifierName == "abs")
        {
            if (arguments.size() == 1)
                return RuntimeValue(static_cast<double>(abs(arguments[0]->evaluateNode(scopes).asNumber())));
            validateArity(1, arguments.size(), "abs", line);
        }
        if (identifierName == "max")
        {
            if (arguments.size() == 2)
                return RuntimeValue(std::max(arguments[0]->evaluateNode(scopes).asNumber(),
                                             arguments[1]->evaluateNode(scopes).asNumber()));
            validateArity(2, arguments.size(), "max", line);
        }
        if (identifierName == "min")
        {
            if (arguments.size() == 2)
                return RuntimeValue((std::min(arguments[0]->evaluateNode(scopes).asNumber(),
                                              arguments[1]->evaluateNode(scopes).asNumber())));
            validateArity(2, arguments.size(), "min", line);
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
            validateArity(1, arguments.size(), "avg", line);
        }
        if (identifierName == "print")
        {
            if (!arguments.empty())
            {
                for (const auto& argument : arguments)
                {
                    RuntimeValue arg = argument->evaluateNode(scopes);
                    printRuntimeValue(arg);
                    cout << " ";
                }
                cout << '\n';
            }
            else cout << "";
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
            validateArity(1, arguments.size(), "size", line);
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
        try
        {
            scopes.get(identifierName);
            throw RuntimeError("object is not callable", {
                                   .category = ErrorCategory::TypeError,
                                   .kind = ErrorKind::NotCallable,
                                   .primary = identifier->evaluateNode(scopes).description(),
                                   .currentLine = line,
                               });
        }
        catch (UndefinedVariable)
        {
            throw RuntimeError("function is not defined", {
                                   .category = ErrorCategory::NameError, .kind = ErrorKind::FunctionUndefined,
                                   .identifier = identifierName, .currentLine = line
                               });
        }
    }
    throw RuntimeError("object is not callable", {
                           .category = ErrorCategory::TypeError,
                           .kind = ErrorKind::NotCallable,
                           .primary = identifier->evaluateNode(scopes).description(),
                           .currentLine = line,
                       });
}


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

void FunctionDeclarationNode::evaluateNode(EnvironmentStack& scopes) const
{
    auto iter = functions.find(identifier);
    if (iter == functions.end())
        functions[identifier] = this;
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
    }
    cout << ")\n";
    cout << string(IndentSize * indentLevel, ' ');
    body->debugPrint(indentLevel + 1);
}

void ReturnNode::evaluateNode(EnvironmentStack& scopes) const
{
    if (returnStatement)
        throw ReturnSignal(returnStatement->evaluateNode(scopes));
    throw ReturnSignal(RuntimeValue(0.0));
}

void ReturnNode::debugPrint(int indentLevel) const
{
    cout << "Return\n";
    cout << string(IndentSize * indentLevel, ' ');
    if (returnStatement)
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
