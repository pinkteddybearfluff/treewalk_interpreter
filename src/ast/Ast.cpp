#include "Ast.h"
#include <stdexcept>
#include <iostream>
#include <ranges>

#include "../parser/Parser.h"
#include "../stdlib/Stdlib.h"

using std::cout;

constexpr int IndentSize = 2;
vector<string> stackTrace{};

void PrimitiveType::debugPrint(int indentLevel) const
{
    cout << string(indentLevel * IndentSize, ' ') << type << "\n";
}


void ArrayType::debugPrint(int indentLevel) const
{
    cout << string(indentLevel * IndentSize, ' ') << "Array\n";
    nestedType->debugPrint(indentLevel + 1);
}


void TupleType::debugPrint(int indentLevel) const
{
    cout << string(indentLevel * IndentSize, ' ') << "Tuple\n";
    for (const auto& type : types)
    {
        type->debugPrint(indentLevel + 1);
    }
}


void UnionType::debugPrint(int indentLevel) const
{
    cout << string(indentLevel * IndentSize, ' ') << "Union\n";
    for (const auto& type : types)
    {
        cout << string(indentLevel * IndentSize, ' ') << "|";
        type->debugPrint(indentLevel + 1);
    }
}


EvalResult NumberNode::evaluateNode(InterpreterContext& ctx) const
{
    return {.hasValue = true, .value = value};
}

void NumberNode::debugPrint(int indentLevel) const
{
    cout << string(indentLevel * IndentSize, ' ') << "Number(" << value << ")\n";
}


EvalResult StringNode::evaluateNode(InterpreterContext& ctx) const
{
    return {true, value};
}

void StringNode::debugPrint(int indentLevel) const
{
    cout << string(indentLevel * IndentSize, ' ') << "String(" << value << ")\n";
}


EvalResult UnaryNode::evaluateNode(InterpreterContext& ctx) const
{
    RuntimeValue value = child->evaluateNode(ctx).value;
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
    throw RuntimeError("unsupported operation for operand", {
                           .category = ErrorCategory::TypeError,
                           .kind = ErrorKind::OperandTypeMismatch, .identifier = getSymbolForOp(op.type),
                           .primary = value.description(), .currentLine = line
                       }, stackTrace);
}

void UnaryNode::debugPrint(int indentLevel) const
{
    cout << string(indentLevel * IndentSize, ' ') << "Unary(" << getSymbolForOp(op.type) << ")\n";
    child->debugPrint(indentLevel + 1);
}

void BooleanNode::debugPrint(int indentLevel) const
{
    cout << string(indentLevel * IndentSize, ' ') << "Bool(" << ((value == true) ? "True" : "False") << ")\n";
}

EvalResult BooleanNode::evaluateNode(InterpreterContext& ctx) const
{
    return {true, value};
}

EvalResult NullNode::evaluateNode(InterpreterContext& ctx) const
{
    return {true, {}};
}

void NullNode::debugPrint(int indentLevel) const
{
    cout << string(indentLevel * IndentSize, ' ') << "Null\n";
}

EvalResult ArrayNode::evaluateNode(InterpreterContext& ctx) const
{
    shared_ptr<RuntimeValue::Array> arrayPtr = std::make_shared<vector<RuntimeValue>>();

    for (int i = 0; i < value.size(); ++i)
    {
        arrayPtr->push_back(value[i]->evaluateNode(ctx).value);
    }
    return {true, arrayPtr};
}

void ArrayNode::debugPrint(int indentLevel) const
{
    cout << string(indentLevel * IndentSize, ' ') << "Array[\n";
    for (auto& element : value)
        element->debugPrint(indentLevel + 1);
    cout << string(indentLevel * IndentSize, ' ') << "]\n";
}

EvalResult MapNode::evaluateNode(InterpreterContext& ctx) const
{
    auto map = std::make_shared<Map>();
    for (const auto& pairEx : keyValPairs)
    {
        auto [itr,inserted] = map->emplace(std::make_pair(pairEx.first->evaluateNode(ctx).value,
                                                          pairEx.second->evaluateNode(ctx).value));

        if (!inserted)
            throw std::runtime_error("Duplicate key found in map");
    }
    return {true, map};
}

void MapNode::debugPrint(int indentLevel) const
{
    cout << string(indentLevel * IndentSize, ' ') << "Map{\n";
    for (const auto& pair : keyValPairs)
    {
        pair.first->debugPrint(indentLevel + 1);
        pair.second->debugPrint(indentLevel + 1);
    }
    cout << string(indentLevel * IndentSize, ' ') << "}\n";
}

void StructNode::registerInEnv(InterpreterContext& ctx) const
{
    std::map<string, shared_ptr<FunctionObject>> methods;
    for (auto& func : methodsStmt)
    {
        dynamic_cast<FunctionDeclarationNode*>(func.get())->registerInCustomEnv(methods, ctx);
    }

    ctx.env->types.insert(std::make_pair(typeName, make_unique<StructType>(typeName, fieldNames, std::move(methods))));
}

EvalResult StructNode::evaluateNode(InterpreterContext& ctx) const
{
    registerInEnv(ctx);
    return {false};
}

void StructNode::debugPrint(int indentLevel) const
{
    cout << string(indentLevel * IndentSize, ' ') << "Struct " << typeName << " {\n";
    for (const auto& fieldName : fieldNames)
    {
        cout << string((indentLevel + 1) * IndentSize, ' ') << fieldName << "\n";
    }

    cout << string(indentLevel * IndentSize, ' ') << "}\n";
}

void EnumNode::registerInEnv(InterpreterContext& ctx) const
{
    ctx.env->types.insert(std::make_pair(typeName, make_unique<EnumType>(typeName, variants)));
}

EvalResult EnumNode::evaluateNode(InterpreterContext& ctx) const
{
    registerInEnv(ctx);
    return {false};
}

void EnumNode::debugPrint(int indentLevel) const
{
    cout << string(indentLevel * IndentSize, ' ') << "enum " << typeName << "{\n";
    for (const auto& variant : variants)
    {
        cout << string((indentLevel + 1) * IndentSize, ' ') << variant.name << "(\n";
        for (const auto& field : variant.fields)
        {
            cout << string((indentLevel + 2) * IndentSize, ' ') << field << ", ";
        }
        cout << ")\n";
    }
    cout << string(indentLevel * IndentSize, ' ') << "}\n";
}

EvalResult IndexNode::evaluateNode(InterpreterContext& ctx) const
{
    const RuntimeValue& object = operand->evaluateNode(ctx).value;
    const RuntimeValue indexV = indexExp->evaluateNode(ctx).value;
    if (object.isMap())
    {
        if (object.asMapPtr()->contains(indexV))
            return {true, object.asMapPtr()->at(indexV)};
        throw std::runtime_error("key not found");
    }
    if (object.isArray())
    {
        if (indexV.isNumber())
        {
            const int index = static_cast<int>(indexV.asNumber());
            if (index < object.asArrayPtr()->size())
                return {true, object.asArrayPtr()->at(index)};
            throw RuntimeError("array index out of range", {
                                   .category = ErrorCategory::IndexError,
                                   .kind = ErrorKind::IndexOutOfBounds,
                                   .primary = "array",
                                   .currentLine = line
                               }, stackTrace);
        }
        throw RuntimeError("invalid type for array index", {
                               .category = ErrorCategory::TypeError, .kind = ErrorKind::InvalidIndexType,
                               .identifier = "array",
                               .primary = "numbers",
                               .secondary = indexV.description(), .currentLine = line
                           }, stackTrace);
    }
    if (object.isString())
    {
        if (indexV.isNumber())
        {
            const int index = static_cast<int>(indexV.asNumber());
            if (index < object.asString().size())
                return {true, object.asString().at(index)};
            throw RuntimeError("string index out of range", {
                                   .category = ErrorCategory::IndexError,
                                   .kind = ErrorKind::IndexOutOfBounds,
                                   .primary = "string",
                                   .currentLine = line
                               }, stackTrace);
        }

        throw RuntimeError("invalid type for string index", {
                               .category = ErrorCategory::TypeError, .kind = ErrorKind::InvalidIndexType,
                               .identifier = "string",
                               .primary = "numbers",
                               .secondary = indexV.description(), .currentLine = line
                           }, stackTrace);
    }
    throw
        RuntimeError("object is not subscriptable", {
                         .category = ErrorCategory::TypeError,
                         .kind = ErrorKind::NotSubscriptable,
                         .primary = object.description(),
                         .currentLine = line
                     }, stackTrace);
}

RuntimeValue& IndexNode::getReference(InterpreterContext& ctx)
{
    const RuntimeValue& object = operand->getReference(ctx);
    const RuntimeValue indexV = indexExp->evaluateNode(ctx).value;
    if (object.isMap())
    {
        if (object.asMapPtr()->contains(indexV))
            return object.asMapPtr()->at(indexV);
        throw std::runtime_error("key not found");
    }
    if (object.isArray())
    {
        if (indexV.isNumber())
        {
            const int index = static_cast<int>(indexV.asNumber());
            if (index < object.asArrayPtr()->size())
                return object.asArrayPtr()->at(index);
            throw RuntimeError("array index out of range", {
                                   .category = ErrorCategory::IndexError,
                                   .kind = ErrorKind::IndexOutOfBounds,
                                   .primary = "array",
                                   .currentLine = line
                               }, stackTrace);
        }
        throw RuntimeError("invalid type for array index", {
                               .category = ErrorCategory::TypeError, .kind = ErrorKind::InvalidIndexType,
                               .primary = "numbers",
                               .secondary = indexV.description(), .currentLine = line
                           }, stackTrace);
    }
    throw
        RuntimeError("object is not subscriptable", {
                         .category = ErrorCategory::TypeError,
                         .kind = ErrorKind::NotSubscriptable,
                         .primary = object.description(),
                         .currentLine = line
                     }, stackTrace);
}

void IndexNode::debugPrint(int indentLevel) const
{
    cout << string(indentLevel * IndentSize, ' ') << "IndexNode\n";
    operand->debugPrint(indentLevel + 1);
    cout << string((indentLevel + 1) * IndentSize, ' ') << "[\n";
    indexExp->debugPrint(indentLevel + 1);
    cout << string((indentLevel + 1) * IndentSize, ' ') << "]\n";
}

RuntimeValue& MemberAccessNode::getReference(InterpreterContext& ctx)
{
    RuntimeValue objVal = obj->evaluateNode(ctx).value;
    if (objVal.isModule())
    {
        try
        {
            return objVal.asModulePtr()->getMember(member->getIdentifierName());
        }
        catch (const UndefinedVariable)
        {
            throw RuntimeError("undefined variable", {
                                   .category = ErrorCategory::AttributeError, .kind = ErrorKind::MissingAttribute,
                                   .primary = getObjName(),
                                   .secondary = member->getIdentifierName(),
                                   .currentLine = line
                               }, stackTrace);
        }
    }
    if (objVal.isStructObj())
    {
        return objVal.asStructObjPtr()->fields[member->getIdentifierName()];
    }
    throw RuntimeError("type does not support member access", {
                           .category = ErrorCategory::AttributeError,
                           .kind = ErrorKind::InvalidReceiver, .identifier = objVal.description(),
                           .currentLine = line
                       }, stackTrace);
}

EvalResult MemberAccessNode::evaluateNode(InterpreterContext& ctx) const
{
    RuntimeValue objVal = obj->evaluateNode(ctx).value;
    if (objVal.isModule())
    {
        try
        {
            return {true, objVal.asModulePtr()->getMember(member->getIdentifierName())};
        }
        catch (const UndefinedVariable)
        {
            throw RuntimeError("undefined variable", {
                                   .category = ErrorCategory::AttributeError, .kind = ErrorKind::MissingAttribute,
                                   .primary = getObjName(),
                                   .secondary = member->getIdentifierName(),
                                   .currentLine = line
                               }, stackTrace);
        }
    }
    if (objVal.isArray())
    {
        RuntimeValue memberVal = member->evaluateNode(ctx).value;
        if (memberVal.isCallable())
        {
            return {
                true,
                std::static_pointer_cast<Callable>(std::make_shared<BoundMethod>(memberVal.asCallableObj(), objVal))
            };
        }
    }
    if (objVal.isTypeReference())
    {
        if (auto type = dynamic_cast<EnumType*>(objVal.asTypeRef().type))
        {
            string variantName = member->getIdentifierName();

            for (std::size_t i = 0; i < type->variants.size(); ++i)
            {
                if (variantName == type->variants[i].name)
                {
                    if (type->variants[i].fields.empty())
                    {
                        vector<RuntimeValue> args{};
                        return {true, std::make_shared<EnumValue>(type, i, args)};
                    }
                    return {true, EnumVariantReference(type, i)};
                }
            }
            throw std::runtime_error("invalid variant index");
        }
    }
    if (objVal.isStructObj())
    {
        if (objVal.asStructObjPtr()->hasMemberField(member->getIdentifierName()))
            return {true, objVal.asStructObjPtr()->getMemberVal(member->getIdentifierName())};

        if (objVal.asStructObjPtr()->hasMethod(member->getIdentifierName()))
        {
            RuntimeValue val = std::static_pointer_cast<Callable>(std::make_shared<BoundMethod>(
                objVal.asStructObjPtr()->getMethod(member->getIdentifierName()), objVal));
            return {true, val};
        }
        throw RuntimeError("undefined variable", {
                               .category = ErrorCategory::AttributeError, .kind = ErrorKind::MissingAttribute,
                               .primary = getObjName(),
                               .secondary = member->getIdentifierName(),
                               .currentLine = line
                           }, stackTrace);
    }
    throw RuntimeError("type does not support member access", {
                           .category = ErrorCategory::AttributeError,
                           .kind = ErrorKind::InvalidReceiver, .identifier = objVal.description(),
                           .currentLine = line
                       }, stackTrace);
}

void MemberAccessNode::debugPrint(int indentLevel) const
{
    cout << string(indentLevel * IndentSize, ' ') << "MemberAccessNode\n";
    obj->debugPrint(indentLevel + 1);
    member->debugPrint(indentLevel + 1);
}

EvalResult BinaryNode::evaluateNode(InterpreterContext& ctx) const
{
    if (op.type == TokenType::AndAnd)
    {
        RuntimeValue lval = left->evaluateNode(ctx).value;
        if (!lval.isTruthy())
            return {true, false};
        RuntimeValue rval = right->evaluateNode(ctx).value;

        return {true, rval.isTruthy()};
    }
    if (op.type == TokenType::OrOr)
    {
        RuntimeValue lval = left->evaluateNode(ctx).value;
        if (lval.isTruthy())
            return {true, true};
        RuntimeValue rval = right->evaluateNode(ctx).value;
        return {true, rval.isTruthy()};
    }
    RuntimeValue lval = left->evaluateNode(ctx).value;
    RuntimeValue rval = right->evaluateNode(ctx).value;

    try
    {
        switch (op.type)
        {
        case TokenType::Plus: return {true, Operator::add(lval, rval)};
        case TokenType::Minus: return {true, Operator::sub(lval, rval)};
        case TokenType::Multiply: return {true, Operator::multiply(lval, rval)};
        case TokenType::Divide: return {true, Operator::divide(lval, rval)};
        case TokenType::Modulo: return {true, Operator::modulo(lval, rval)};
        case TokenType::Equal: return {true, Operator::equal(lval, rval)};
        case TokenType::NotEqual: return {true, Operator::notEqual(lval, rval)};
        case TokenType::Greater: return {true, Operator::greater(lval, rval)};
        case TokenType::GreaterEqual: return {true, Operator::greaterEqual(lval, rval)};
        case TokenType::Less: return {true, Operator::less(lval, rval)};
        case TokenType::LessEqual: return {true, Operator::lessEqual(lval, rval)};
        default: throw UnsupportedOperation();
        }
    }
    catch (UnsupportedOperation)
    {
        throw RuntimeError("unsupported operation", {
                               .category = ErrorCategory::TypeError, .kind = ErrorKind::UnsupportedOperation,
                               .identifier = getSymbolForOp(op.type), .primary = lval.description(),
                               .secondary = rval.description(),
                               .currentLine = line
                           }, stackTrace);
    }
    catch (DivisionByZero)
    {
        throw RuntimeError("division by zero", {
                               .category = ErrorCategory::ZeroDivisionError, .kind = ErrorKind::DivisionByZero,
                               .currentLine = line
                           }, stackTrace);
    }
}

void BinaryNode::debugPrint(int indentLevel) const
{
    cout << string(indentLevel * IndentSize, ' ') << "Binary(" << getSymbolForOp(op.type) << ")\n";
    left->debugPrint(indentLevel + 1);
    right->debugPrint(indentLevel + 1);
}

EvalResult ExpressionStatementNode::evaluateNode(InterpreterContext& ctx) const
{
    auto val = expressionStmt->evaluateNode(ctx).value;
    return {true, val};
}

void ExpressionStatementNode::debugPrint(int indentLevel) const
{
    cout << string(indentLevel * IndentSize, ' ') << "ExpressionStatement\n";
    expressionStmt->debugPrint(indentLevel + 1);
    if (expressionStmt == nullptr) { cout << "null"; }
}

EvalResult VariableNode::evaluateNode(InterpreterContext& ctx) const
{
    try
    {
        RuntimeValue val;
        if (ctx.env->hasVariable(identifierName))
            val = ctx.env->getReference(identifierName).value;
        else if (ctx.env->hasType(identifierName))
        {
            auto value = TypeReference{ctx.env->getType(identifierName)};
            return {true, value};
        }
        if (val.isUninitialized())
            throw RuntimeError("use of uninitialized variable", {
                                   .category = ErrorCategory::UninitializedError,
                                   .kind = ErrorKind::UninitializedVariable,
                                   .identifier = getIdentifierName(), .currentLine = line
                               }, stackTrace);
        return {true, val};
    }
    catch (UndefinedVariable)
    {
        throw RuntimeError("undefined variable", {
                               .category = ErrorCategory::NameError, .kind = ErrorKind::VariableUndefined,
                               .identifier = identifierName, .currentLine = line
                           }, stackTrace);
    }
}

RuntimeValue& VariableNode::getReference(InterpreterContext& ctx)
{
    try
    {
        return ctx.env->getReference(identifierName).value;
    }
    catch (UndefinedVariable)
    {
        throw RuntimeError("undefined variable", {
                               .category = ErrorCategory::NameError, .kind = ErrorKind::VariableUndefined,
                               .identifier = identifierName, .currentLine = line
                           }, stackTrace);
    }
}

void VariableNode::debugPrint(int indentLevel) const
{
    cout << string(indentLevel * IndentSize, ' ') << "Variable(" << identifierName << ")\n";
}

EvalResult AssignmentNode::evaluateNode(InterpreterContext& ctx) const
{
    RuntimeValue right = rvalue->evaluateNode(ctx).value;
    lvalue->getReference(ctx) = right;
    return {false, right};
}

void AssignmentNode::debugPrint(int indentLevel) const
{
    cout << string(indentLevel * IndentSize, ' ') << "Assignment(=)\n";
    lvalue->debugPrint(indentLevel + 1);
    rvalue->debugPrint(indentLevel + 1);
}

EvalResult CompoundAssignmentNode::evaluateNode(InterpreterContext& ctx) const
{
    RuntimeValue rhs = rvalue->evaluateNode(ctx).value;

    auto& lhs = lvalue->getReference(ctx);
    if (lhs.isUninitialized())
        throw RuntimeError("use of uninitialized variable", {
                               .category = ErrorCategory::UninitializedError,
                               .kind = ErrorKind::UninitializedVariable,
                               .identifier = lvalue->getIdentifierName(), .currentLine = line
                           }, stackTrace);
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

                           }, stackTrace);
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

                           }, stackTrace);
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
                           }, stackTrace);
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
                               }, stackTrace);
        }
        if (lhs.isBoolean() && rhs.isBoolean())
        {
            if (rhs.asBoolean())
                return {false, lhs.getBoolRef() /= rhs.asBoolean()};
            throw RuntimeError("division by zero", {
                                   .category = ErrorCategory::ZeroDivisionError,
                                   .kind = ErrorKind::DivisionByZero,
                                   .currentLine = line
                               }, stackTrace);
        }
        throw RuntimeError("unsupported operations", {
                               .category = ErrorCategory::TypeError, .kind = ErrorKind::UnsupportedOperation,
                               .identifier = "/", .primary = lhs.description(), .secondary = rhs.description()
                           }, stackTrace);
        break;
    }
}

void CompoundAssignmentNode::debugPrint(int indentLevel) const
{
    cout << string(indentLevel * IndentSize, ' ') << "CompoundAssignment(" << getSymbolForOp(op.type) << ")\n";
    lvalue->debugPrint(indentLevel + 1);
    rvalue->debugPrint(indentLevel + 1);
}

EvalResult MatchPatternNode::evaluateNode(InterpreterContext& ctx) const
{
    RuntimeValue scrutineeVal = scrutinee->evaluateNode(ctx).value;
    // ExpressionNode* wildCardExpr;
    for (std::size_t i = 0; i < armsExpr.size(); ++i)
    {
        if (armsExpr[i].pattern->matches(scrutineeVal, ctx))
        {
            return {true, armsExpr[i].expr->evaluateNode(ctx).value};
        }
    }
    throw std::runtime_error("default case not found");
}

void MatchPatternNode::debugPrint(int indentLevel) const
{
    scrutinee->debugPrint(indentLevel + 1);
    for (const auto& arm : armsExpr)
    {
        // arm.first->debugPrint(indentLevel + 1);
        arm.expr->debugPrint(indentLevel + 1);
    }
}

EvalResult DeclarationNode::evaluateNode(InterpreterContext& ctx) const
{
    RuntimeValue right = Uninitialized();
    if (rvalue)
        right = rvalue->evaluateNode(ctx).value;
    if (auto* var = dynamic_cast<VariableNode*>(lvalue.get()))
    {
        try
        {
            ctx.env->declare(var->getIdentifierName(), {right, line});
        }
        catch (Redeclaration)
        {
            throw RuntimeError("redeclaration of variable", {
                                   .category = ErrorCategory::RedeclarationError,
                                   .kind = ErrorKind::VariableRedeclaration,
                                   .identifier = var->getIdentifierName(),
                                   .currentLine = line,
                                   .previousLine = ctx.env->getReference(var->getIdentifierName()).declarationLine
                               }, stackTrace);
        }
    }
    return {false};
}

void DeclarationNode::debugPrint(int indentLevel) const
{
    cout << string(indentLevel * IndentSize, ' ') << "Let\n";
    lvalue->debugPrint(indentLevel + 1);
    if (type)
    {
        cout << string(indentLevel * IndentSize, ' ') << "Type\n";
        type->debugPrint(indentLevel + 1);
    }
    if (rvalue)
        rvalue->debugPrint(indentLevel + 1);
}

EvalResult IfNode::evaluateNode(InterpreterContext& ctx) const
{
    auto currentEnv = std::make_shared<Environment>();
    currentEnv->parent = ctx.env;
    InterpreterContext ctxNew{currentEnv, ctx.module};
    if (condition->evaluateNode(ctxNew).value.isTruthy())
        thenStatement->evaluateNode(ctxNew);
    else
    {
        if (elseStatement)
            elseStatement->evaluateNode(ctxNew);
    }
    return {false};
}

void IfNode::debugPrint(int indentLevel) const
{
    cout << string(indentLevel * IndentSize, ' ') << "If\n";
    condition->debugPrint(indentLevel + 1);
    thenStatement->debugPrint(indentLevel + 1);

    if (elseStatement)
    {
        cout << string(IndentSize * indentLevel, ' ') << "Else\n";
        elseStatement->debugPrint(indentLevel + 1);
    }
    cout << "\n";
}

EvalResult WhileNode::evaluateNode(InterpreterContext& ctx) const
{
    InterpreterContext currentCtx{std::make_shared<Environment>(), ctx.module};
    currentCtx.env->parent = ctx.env;


    while (condition->evaluateNode(currentCtx).value.isTruthy())
    {
        try
        {
            // ScopedEnvironment local(currentEnv);
            InterpreterContext localContext{std::make_shared<Environment>(), ctx.module};
            localContext.env->parent = currentCtx.env;
            statement->evaluateNode(localContext);
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
    cout << string(indentLevel * IndentSize, ' ') << "While\n";
    condition->debugPrint(indentLevel + 1);
    statement->debugPrint(indentLevel + 1);
}

EvalResult ForNode::evaluateNode(InterpreterContext& ctx) const
{
    InterpreterContext currentCtx{std::make_shared<Environment>(), ctx.module};
    currentCtx.env->parent = ctx.env;

    // ScopedEnvironment local(scopes);
    if (initializer)
        initializer->evaluateNode(currentCtx);

    while (true)
    {
        try
        {
            if (condition)
                if (!condition->evaluateNode(currentCtx).value.isTruthy())
                    break;

            InterpreterContext localCtx{std::make_shared<Environment>(), ctx.module};
            localCtx.env->parent = currentCtx.env;
            statement->evaluateNode(localCtx);
            if (expr) expr->evaluateNode(currentCtx);
        }
        catch (BreakSignal b)
        {
            break;
        }
        catch (ContinueSignal c)
        {
            if (expr) expr->evaluateNode(currentCtx);
            continue;
        }
    }
    return {false};
}

void ForNode::debugPrint(int indentLevel) const
{
    cout << string(indentLevel * IndentSize, ' ') << "For(\n";
    if (initializer) initializer->debugPrint(indentLevel + 1);
    if (condition) condition->debugPrint(indentLevel + 1);
    if (expr) expr->debugPrint(indentLevel + 1);
    cout << string(indentLevel * IndentSize, ' ') << ")\n";
    statement->debugPrint(indentLevel + 1);
}

void ForEachNode::debugPrint(int indentLevel) const
{
}

EvalResult ForEachNode::evaluateNode(InterpreterContext& ctx) const
{
    RuntimeValue container = containerExp->evaluateNode(ctx).value;
    if (container.isArray())
    {
        for (std::size_t i = 0; i < container.asArrayPtr()->size(); ++i)
        {
            try
            {
                InterpreterContext currentCtx{std::make_shared<Environment>(), ctx.module};
                currentCtx.env->parent = ctx.env;
                currentCtx.env->declare(identifiers[0], VariableInfo{container.asArrayPtr()->at(i), 0});
                if (identifiers.size() == 2)
                {
                    currentCtx.env->declare(identifiers[1], VariableInfo{static_cast<int>(i), 0});
                }
                statement->evaluateNode(currentCtx);
            }
            catch (ContinueSignal)
            {
                continue;
            }
            catch (BreakSignal)
            {
                break;
            }
        }
    }
    if (container.isString())
    {
        for (std::size_t i = 0; i < container.asString().size(); ++i)
        {
            try
            {
                InterpreterContext currentCtx{std::make_shared<Environment>(), ctx.module};
                currentCtx.env->parent = ctx.env;
                currentCtx.env->declare(identifiers[0], VariableInfo{container.asString().at(i), 0});
                if (identifiers.size() == 2)
                {
                    currentCtx.env->declare(identifiers[1], VariableInfo{static_cast<int>(i), 0});
                }
                statement->evaluateNode(currentCtx);
            }
            catch (ContinueSignal)
            {
                continue;
            }
            catch (BreakSignal)
            {
                break;
            }
        }
    }
    if (container.isMap())
    {
        for (auto [key,value] : *container.asMapPtr())
        {
            try
            {
                InterpreterContext currentCtx{std::make_shared<Environment>(), ctx.module};
                currentCtx.env->parent = ctx.env;
                currentCtx.env->declare(identifiers[0], VariableInfo{key, 0});
                currentCtx.env->declare(identifiers[1], VariableInfo{value, 0});
                statement->evaluateNode(currentCtx);
            }
            catch (ContinueSignal)
            {
                continue;
            }
            catch (BreakSignal)
            {
                break;
            }
        }
    }
    return {false};
}

EvalResult BreakNode::evaluateNode(InterpreterContext& ctx) const
{
    throw BreakSignal();
}

void BreakNode::debugPrint(int indentLevel) const
{
    cout << string(indentLevel * IndentSize, ' ') << "Break\n";
}

EvalResult ContinueNode::evaluateNode(InterpreterContext& ctx) const
{
    throw ContinueSignal();
}

void ContinueNode::debugPrint(int indentLevel) const
{
    cout << string(indentLevel * IndentSize, ' ') << "Continue\n";
}

EvalResult BlockExpressionNode::evaluateNode(InterpreterContext& ctx) const
{
    InterpreterContext localCtx = {std::make_shared<Environment>(), ctx.module};
    localCtx.env->parent = ctx.env;
    for (auto& statement : statements)
    {
        if (auto* fcn = dynamic_cast<FunctionDeclarationNode*>(statement.get()))
        {
            fcn->registerInEnv(localCtx);
        }
    }
    for (auto& statement : statements)
    {
        statement->evaluateNode(localCtx);
    }
    return resultExpr->evaluateNode(localCtx);
}

void BlockExpressionNode::debugPrint(int indentLevel) const
{
}

EvalResult BlockNode::evaluateNode(InterpreterContext& ctx) const
{
    // ScopedEnvironment local(env);
    auto currentEnv = std::make_shared<Environment>();
    currentEnv->parent = ctx.env;
    InterpreterContext ctxNew = {currentEnv, ctx.module};

    for (auto& statement : statements)
    {
        if (auto* fcn = dynamic_cast<FunctionDeclarationNode*>(statement.get()))
        {
            fcn->registerInEnv(ctxNew);
        }
    }
    for (auto& statement : statements)
        statement->evaluateNode(ctxNew);
    return {false};
}

void BlockNode::debugPrint(int indentLevel) const
{
    cout << string(indentLevel * IndentSize, ' ') << "Block{\n";
    for (auto& statement : statements)
    {
        statement->debugPrint(indentLevel + 1);
    }
    cout << string(IndentSize * indentLevel, ' ') << "}\n";
}

RuntimeValue constructor(StructType* type, vector<RuntimeValue> args)
{
    map<string, RuntimeValue> fields;
    if (type->fieldNames.size() == args.size())
    {
        for (std::size_t i = 0; i < type->fieldNames.size(); ++i)
        {
            fields[type->fieldNames[i]] = args[i];
        }
    }
    return std::make_shared<StructInstance>(type, fields);
}

RuntimeValue constructor(EnumVariantReference typeRef, vector<RuntimeValue> args)
{
    return std::make_shared<EnumValue>(typeRef.type, typeRef.variantIndex, args);
}

EvalResult FunctionCallNode::evaluateNode(InterpreterContext& ctx) const
{
    vector<RuntimeValue> args;
    for (const auto& argument : arguments)
        args.push_back(argument->evaluateNode(ctx).value);

    RuntimeValue obj = identifier->evaluateNode(ctx).value;
    if (obj.isTypeReference())
    {
        if (auto structType = dynamic_cast<StructType*>(obj.asTypeRef().type))
        {
            stackTrace.push_back(structType->name);
            RuntimeValue val = constructor(structType, args);
            stackTrace.pop_back();
            return {true, val};
        }
    }
    if (obj.isEnumVariantReference())
    {
        if (auto enumVariantType = obj.asEnumVariantRef().type)
        {
            cout << "into the constructor we go " << std::endl;
            stackTrace.push_back(std::format("{}.{}()", enumVariantType->typeName,
                                             enumVariantType->variants[obj.asEnumVariantRef().variantIndex].name));
            RuntimeValue val = constructor(obj.asEnumVariantRef(), args);
            cout << "Out the constructor we come" << std::endl;
            stackTrace.pop_back();
            return {true, val};
        }
    }
    if (obj.isCallable())
    {
        auto function = obj.asCallableObj();

        stackTrace.push_back(identifier->getIdentifierName());
        RuntimeValue result;
        try
        {
            result = function->call(args, line);
        }
        catch (const ArityDiagnostic& ad)
        {
            throw RuntimeError("arity error", {
                                   .category = ErrorCategory::ArityError,
                                   .kind = ad.kind, .identifier = ad.identifier, .currentLine = line,
                                   .expected = ad.expected, .actual = ad.actual,
                                   .variadic = ad.variadic
                               }, stackTrace);
        }
        stackTrace.pop_back();
        return {true, result};
    }

    throw RuntimeError("object is not callable", {
                           .category = ErrorCategory::TypeError,
                           .kind = ErrorKind::NotCallable,
                           .primary = identifier->evaluateNode(ctx).value.description(),
                           .currentLine = line,
                       }, stackTrace);
}

void FunctionCallNode::debugPrint(int indentLevel) const
{
    cout << string(indentLevel * IndentSize, ' ') << "FunctionCall{\n";

    identifier->debugPrint(indentLevel + 1);
    for (const auto& argument : arguments)
    {
        argument->debugPrint(indentLevel + 1);
    }
    cout << string(indentLevel * IndentSize, ' ') << "}\n";
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

void FunctionDeclarationNode::registerInEnv(InterpreterContext& ctx) const
{
    vector<Parameter> params;
    for (const auto& parameter : parameters)
        params.emplace_back(parameter.identifier, parameter.defaultArg.get(), parameter.type.get(),
                            parameter.isVariadic);
    try
    {
        auto* body_ptr = dynamic_cast<BlockNode*>(body.get());
        auto fn = std::static_pointer_cast<Callable>(
            make_shared<FunctionObject>(
                identifier, std::move(params), body_ptr, ctx.env, line)
        );
        ctx.env->declare(identifier, VariableInfo(fn,
                                                  line
                         ));
    }
    catch (Redeclaration)
    {
        throw RuntimeError("redeclaration of function", {
                               .category = ErrorCategory::RedeclarationError,
                               .kind = ErrorKind::FunctionRedeclaration,
                               .identifier = identifier,
                               .currentLine = line,
                           }, stackTrace);
    }
}

EvalResult FunctionDeclarationNode::evaluateNode(InterpreterContext& ctx) const
{
    return {false};
}

void FunctionDeclarationNode::debugPrint(int indentLevel) const
{
    cout << string(indentLevel * IndentSize, ' ') << "FunctionDeclaration\n";
    cout << string(indentLevel * IndentSize, ' ') << "Parameters(";
    for (const auto& parameter : parameters)
    {
        cout << parameter.identifier;
        if (parameter.type)
            parameter.type->debugPrint(indentLevel + 1);
        cout << " ";
    }
    cout << string(indentLevel * IndentSize, ' ') << ")\n";
    if (type)
    {
        type->debugPrint(indentLevel + 1);
    }
    body->debugPrint(indentLevel + 1);
}

EvalResult ReturnNode::evaluateNode(InterpreterContext& ctx) const
{
    if (returnStatement)
        throw ReturnSignal(returnStatement->evaluateNode(ctx).value);
    throw ReturnSignal({});
}

void ReturnNode::debugPrint(int indentLevel) const
{
    cout << string(indentLevel * IndentSize, ' ') << "Return\n";
    if (returnStatement)
        returnStatement->debugPrint(indentLevel + 1);
}

EvalResult ProgramNode::evaluateNode(InterpreterContext& ctx) const
{
    //first pass
    for (auto& stmt : statements)
    {
        if (auto* fn = dynamic_cast<FunctionDeclarationNode*>(stmt.get()))
        {
            fn->registerInEnv(ctx);
        }
    }
    //second pass
    for (auto& stmt : statements)
        stmt->evaluateNode(ctx);
    return {false};
}

void ProgramNode::debugPrint(int indentLevel) const
{
    for (const auto& stmt : statements)
        stmt->debugPrint(indentLevel + 1);
}

EvalResult EmptyNode::evaluateNode(InterpreterContext& ctx) const
{
    return {false};
}

void EmptyNode::debugPrint(int indentLevel) const
{
    cout << string(indentLevel * IndentSize, ' ') << "EmptyStatement\n";
}

EvalResult ThrowNode::evaluateNode(InterpreterContext& ctx) const
{
    throw ThrowSignal(throwValExp->evaluateNode(ctx).value);
}

void ThrowNode::debugPrint(int indentLevel) const
{
}

EvalResult TryCatch::evaluateNode(InterpreterContext& ctx) const
{
    try
    {
        InterpreterContext localCtx = InterpreterContext(std::make_shared<Environment>(), ctx.module);
        localCtx.env->parent = ctx.env;
        tryStatement->evaluateNode(localCtx);
    }
    catch (const ThrowSignal& e)
    {
        InterpreterContext localCtx = InterpreterContext(std::make_shared<Environment>(), ctx.module);
        localCtx.env->parent = ctx.env;
        localCtx.env->declare(catches[0].name, VariableInfo(e.val, 0));
        catches[0].blockStatement->evaluateNode(localCtx);
    }
    return {false};
}

void TryCatch::debugPrint(int indentLevel) const
{
}

EvalResult ImportNode::evaluateNode(InterpreterContext& ctx) const
{
    if (isStdLib)
    {
        string stdLibIdentifier = std::format("std.{}", file);
        auto itr = ctx.module->loadedModules.find(stdLibIdentifier);
        if (itr == ctx.module->loadedModules.end())
        {
            auto moduleCtx = InterpreterContext(std::make_shared<Environment>(), ctx.module);
            try
            {
                ctx.module->loadedModules.insert({stdLibIdentifier, loadStdlib(file, moduleCtx)});
            }
            catch (UndefinedVariable)
            {
                throw RuntimeError("no stdlib with given module name", {
                                       .category = ErrorCategory::ImportError, .kind = ErrorKind::ModuleNotFound,
                                       .identifier = file, .primary = "std", .currentLine = line
                                   }, stackTrace);
            }
            moduleCtx.env->parent = ctx.env;
            moduleCtx.workingDir = ctx.workingDir;
            // ctx.module->loadedModules[stdLibIdentifier]->evaluateNode(moduleCtx);
            ctx.env->declare(alias, {
                                 std::make_shared<Module>(moduleCtx.env, alias), line
                             });
        }
        return {false};
    }
    const string filePath = std::format("{}/{}", ctx.workingDir, file);
    std::ifstream is(filePath);
    string parentFile = ctx.currentFile;
    ctx.currentFile = filePath;
    if (!is.is_open())
    {
        throw RuntimeError("module not found at given file path", {
                               .category = ErrorCategory::ImportError, .kind = ErrorKind::ModuleNotFound,
                               .identifier = filePath, .currentLine = line
                           }, stackTrace);
    }

    vector<unique_ptr<StatementNode>> nodes;
    TokenStream ts{is};
    while (!check(TokenType::End, ts))
    {
        nodes.push_back(parseStatement(ts));
    }
    unique_ptr<ProgramNode> program = make_unique<ProgramNode>(std::move(nodes));

    auto itr = ctx.module->loadedModules.find(filePath);
    if (itr == ctx.module->loadedModules.end())
    {
        ctx.module->loadedModules.insert({filePath, std::move(program)});
        auto moduleCtx = InterpreterContext(std::make_shared<Environment>(), ctx.module);
        moduleCtx.env->parent = ctx.env;
        moduleCtx.workingDir = ctx.workingDir;
        ctx.module->loadedModules[filePath]->evaluateNode(moduleCtx);
        ctx.env->declare(alias, {
                             std::make_shared<Module>(moduleCtx.env, alias), line
                         });
    }
    ctx.currentFile = parentFile;
    return {false};
}

void ImportNode::debugPrint(int indentLevel) const
{
    cout << string(indentLevel * IndentSize, ' ') << "Import '" << file << "' As " << alias << "\n";
}


void terminateWithNL(string description, std::ostream& os)
{
    if (description == "Declaration" || description == "ExpressionStatement" || description == "Continue" || description
        == "Break" || description == "Return")
        os << ";\n";
    else os << "\n\n";
}

void terminateLine(string description, std::ostream& os)
{
    if (description == "Declaration" || description == "ExpressionStatement" || description == "Continue" || description
        == "Break" || description == "Return")
        os << ";";
}


void NumberNode::format(FormatContext& ctx) const
{
    ctx.os << value;
}

void StringNode::format(FormatContext& ctx) const
{
    ctx.os << "\"" << value << "\"";
}


void BooleanNode::format(FormatContext& ctx) const
{
    ctx.os << ((value == true) ? "true" : "false");
}

void NullNode::format(FormatContext& ctx) const
{
    ctx.os << "null";
}

void ArrayNode::format(FormatContext& ctx) const
{
    ctx.os << "[";
    for (int i = 0; i < value.size(); ++i)
    {
        value[i]->format(ctx);
        if (i < value.size() - 1)
            ctx.os << ", ";
    }
    ctx.os << "]";
}

void MapNode::format(FormatContext& ctx) const
{
    ctx.os << "{\n";
    ctx.indent();
    for (std::size_t i = 0; i < keyValPairs.size(); ++i)
    {
        ctx.writeIndent();
        keyValPairs[i].first->format(ctx);
        ctx.os << ": ";
        keyValPairs[i].second->format(ctx);
        if (i < keyValPairs.size() - 1)
            ctx.os << ",\n";
        else ctx.os << "\n";
    }
    ctx.dedent();
    ctx.writeIndent();
    ctx.os << "}";
}

void StructNode::format(FormatContext& ctx) const
{
}

void EnumNode::format(FormatContext& ctx) const
{
}

void VariableNode::format(FormatContext& ctx) const
{
    ctx.os << identifierName;
}

void IndexNode::format(FormatContext& ctx) const
{
    operand->format(ctx);
    ctx.os << "[";
    indexExp->format(ctx);
    ctx.os << "]";
}

void MemberAccessNode::format(FormatContext& ctx) const
{
    obj->format(ctx);
    ctx.os << ".";
    member->format(ctx);
}

void FunctionCallNode::format(FormatContext& ctx) const
{
    identifier->format(ctx);
    ctx.os << "(";
    for (int i = 0; i < arguments.size(); ++i)
    {
        arguments[i]->format(ctx);
        if (i < arguments.size() - 1)
            ctx.os << ", ";
    }
    ctx.os << ")";
}

void MatchPatternNode::format(FormatContext& ctx) const
{
}

void UnaryNode::format(FormatContext& ctx) const
{
    ctx.os << getSymbolForOp(op.type);
    child->format(ctx);
}

void BinaryNode::format(FormatContext& ctx) const
{
    left->format(ctx);
    ctx.os << " " << getSymbolForOp(op.type) << " ";
    right->format(ctx);
}

void AssignmentNode::format(FormatContext& ctx) const
{
    lvalue->format(ctx);
    ctx.os << " = ";
    rvalue->format(ctx);
}

void CompoundAssignmentNode::format(FormatContext& ctx) const
{
    lvalue->format(ctx);
    ctx.os << " " << getSymbolForOp(op.type) << " ";
    rvalue->format(ctx);
}

void DeclarationNode::format(FormatContext& ctx) const
{
    ctx.os << "let ";
    lvalue->format(ctx);
    if (type)
    {
        ctx.os << ": ";
        type->format(ctx);
    }
    if (rvalue)
    {
        ctx.os << " = ";
        rvalue->format(ctx);
    }
}

void BlockExpressionNode::format(FormatContext& ctx) const
{
}


void ExpressionStatementNode::format(FormatContext& ctx) const
{
    expressionStmt->format(ctx);
}

void IfNode::format(FormatContext& ctx) const
{
    ctx.os << "if (";
    condition->format(ctx);
    ctx.os << ")";
    if (thenStatement->description() == "block")
    {
        ctx.os << " ";
        thenStatement->format(ctx);
        // formatTerminator(thenStatement->description());
    }
    else
    {
        ctx.os << "\n";
        ctx.indent();
        ctx.writeIndent();
        thenStatement->format(ctx);
        terminateLine(thenStatement->description(), ctx.os);
        ctx.dedent();
    }

    if (elseStatement)
    {
        ctx.os << "\n";
        ctx.writeIndent();
        ctx.os << "else";
        if (elseStatement->description() == "block")
        {
            ctx.os << " ";
            elseStatement->format(ctx);
        }
        else
        {
            ctx.os << "\n";
            ctx.indent();
            ctx.writeIndent();
            elseStatement->format(ctx);
            terminateLine(elseStatement->description(), ctx.os);
            ctx.dedent();
        }
    }
}


void WhileNode::format(FormatContext& ctx) const
{
    ctx.os << "while (";
    condition->format(ctx);
    ctx.os << ")";
    if (statement->description() == "block")
    {
        ctx.os << " ";
        statement->format(ctx);
    }
    else
    {
        ctx.os << "\n";
        ctx.indent();
        ctx.writeIndent();
        statement->format(ctx);
        terminateLine(statement->description(), ctx.os);

        ctx.dedent();
    }
}


void ForNode::format(FormatContext& ctx) const
{
    ctx.os << "for (";
    if (initializer)
        initializer->format(ctx);
    ctx.os << ";";
    if (condition)
    {
        ctx.os << " ";
        condition->format(ctx);
    }
    ctx.os << ";";
    if (expr)
    {
        ctx.os << " ";
        expr->format(ctx);
    }
    ctx.os << ")";
    if (statement->description() == "block")
    {
        ctx.os << " ";

        statement->format(ctx);
    }
    else
    {
        ctx.os << "\n";
        ctx.indent();
        ctx.writeIndent();
        statement->format(ctx);
        terminateLine(statement->description(), ctx.os);
        ctx.dedent();
    }
}

void ForEachNode::format(FormatContext& ctx) const
{
}


void BreakNode::format(FormatContext& ctx) const
{
    ctx.os << "break";
}


void ContinueNode::format(FormatContext& ctx) const
{
    ctx.os << "continue";
}


void BlockNode::format(FormatContext& ctx) const
{
    ctx.os << "{\n";

    ctx.indent();
    for (auto& statement : statements)
    {
        ctx.writeIndent();
        statement->format(ctx);
        terminateWithNL(statement->description(), ctx.os);
    }
    ctx.dedent();
    ctx.writeIndent();
    ctx.os << "}";
}


void FunctionDeclarationNode::format(FormatContext& ctx) const
{
    ctx.os << "fn " << identifier << "(";
    for (int i = 0; i < parameters.size(); ++i)
    {
        ctx.os << parameters[i].identifier;
        if (parameters[i].type)
        {
            ctx.os << ": ";
            parameters[i].type->format(ctx);
        }
        if (i < parameters.size() - 1)
            ctx.os << ", ";
    }
    // if (variadic)
    // {
    //     ctx.os << "..." << variadicParam.identifier;
    //     if (variadicParam.type)
    //     {
    //         ctx.os << ": ";
    //         variadicParam.type->format(ctx);
    //     }
    // }
    ctx.os << ") ";
    if (type)
    {
        ctx.os << "-> ";
        type->format(ctx);
        ctx.os << " ";
    }
    body->format(ctx);
}

void ReturnNode::format(FormatContext& ctx) const
{
    ctx.os << "return ";
    if (returnStatement)
    {
        returnStatement->format(ctx);
    }
}


void EmptyNode::format(FormatContext& ctx) const
{
    ctx.os << ";";
}

void ImportNode::format(FormatContext& ctx) const
{
    if (isStdLib)
        ctx.os << "import " << file << " as " << alias;
    else
        ctx.os << "import \"" << file << "\" as " << alias;
}

void TryCatch::format(FormatContext& ctx) const
{
}

void ThrowNode::format(FormatContext& ctx) const
{
}

void ProgramNode::format(FormatContext& ctx) const
{
    for (auto& stmt : statements)
    {
        ctx.writeIndent();
        stmt->format(ctx);
        terminateWithNL(stmt->description(), ctx.os);
    }
}

void PrimitiveType::format(FormatContext& ctx) const
{
    ctx.os << type;
}

void ArrayType::format(FormatContext& ctx) const
{
    ctx.os << "Array[";
    nestedType->format(ctx);
    ctx.os << "]";
}

void TupleType::format(FormatContext& ctx) const
{
    ctx.os << "Tuple[";
    for (int i = 0; i < types.size(); ++i)
    {
        types[i]->format(ctx);
        if (i < types.size() - 1)
            ctx.os << ", ";
    }
    ctx.os << "]";
}

void UnionType::format(FormatContext& ctx) const
{
    for (int i = 0; i < types.size(); ++i)
    {
        types[i]->format(ctx);
        if (i < types.size() - 1)
            ctx.os << " | ";
    }
}

