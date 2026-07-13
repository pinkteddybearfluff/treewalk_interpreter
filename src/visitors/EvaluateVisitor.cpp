#include "EvaluateVisitor.h"
#include "../ast/Ast.h"
#include "../error/RuntimeError.h"
#include "../stdlib/Stdlib.h"

void EvaluateVisitor::visit(NumberNode& node)
{
    result = node.value;
}

void EvaluateVisitor::visit(StringNode& node)
{
    result = node.value;
}

void EvaluateVisitor::visit(BooleanNode& node)
{
    result = node.value;
}

void EvaluateVisitor::visit(NullNode& node)
{
    result = {};
}

void EvaluateVisitor::visit(ArrayNode& node)
{
    auto arrayPtr = std::make_shared<RuntimeValue::Array>();
    for (const auto& expr : node.value)
    {
        expr->accept(*this);
        arrayPtr->push_back(std::move(result));
    }
    result = std::move(arrayPtr);
}

void EvaluateVisitor::visit(MapNode& node)
{
    auto mapPtr = std::make_shared<Map>();
    for (const auto& [keyExpr, valExpr] : node.keyValPairs)
    {
        keyExpr->accept(*this);
        auto key = std::move(result);
        if (mapPtr->contains(key))
        {
            throw RuntimeError("duplicate map key", {
                                   .category = ErrorCategory::AlreadyExistsError, .code = ErrorCode::DuplicateMapKey,
                                   .identifier = stdlib::stringify(key), .currentLine = 0
                               }, m_stackTrace);
        }
        valExpr->accept(*this);
        auto value = std::move(result);
        mapPtr->emplace(std::move(key), std::move(value));
    }
    result = std::move(mapPtr);
}

void EvaluateVisitor::visit(VariableNode& node)
{
    if (m_context.env->hasVariable(node.identifierName))
    {
        result = m_context.env->getReference(node.identifierName).value;
        if (result.isUninitialized())
            throw RuntimeError("unitialized variable", {
                                   .category = ErrorCategory::UninitializedError,
                                   .code = ErrorCode::UninitializedVariable, .identifier = node.identifierName,
                                   .currentLine = 0,
                                   .previousLine = m_context.env->getReference(node.identifierName).declarationLine
                               }, m_stackTrace);
        return;
    }
    if (m_context.env->hasType(node.identifierName))
    {
        result = m_context.env->getType(node.identifierName);
        return;
    }
    throw RuntimeError("undefined variable", {
                           .category = ErrorCategory::NameError, .code = ErrorCode::VariableUndefined,
                           .identifier = node.identifierName,
                           .currentLine = 0
                       }, m_stackTrace);
}

void EvaluateVisitor::visit(BinaryNode& node)
{
    node.left->accept(*this);
    auto left = result;
    if (node.op.type == TokenType::AndAnd)
    {
        if (!left.isTruthy())
        {
            result = false;
            return;
        }
        node.right->accept(*this);
        auto right = result;
        result = Operator::logicalAnd(left, right);
        return;
    }
    if (node.op.type == TokenType::OrOr)
    {
        if (left.isTruthy())
        {
            result = true;
            return;
        }

        node.right->accept(*this);
        auto right = result;
        result = Operator::logicalOr(left, right);
        return;
    }

    node.right->accept(*this);
    auto right = result;
    try
    {
        switch (node.op.type)
        {
        case TokenType::Equal:
            result = Operator::equal(left, right);
            break;
        case TokenType::NotEqual:
            result = Operator::notEqual(left, right);
            break;
        case TokenType::Greater:
            result = Operator::greater(left, right);
            break;
        case TokenType::GreaterEqual:
            result = Operator::greaterEqual(left, right);
            break;
        case TokenType::Less:
            result = Operator::less(left, right);
            break;
        case TokenType::LessEqual:
            result = Operator::lessEqual(left, right);
            break;
        case TokenType::Plus:
            result = Operator::add(left, right);
            break;
        case TokenType::Minus:
            result = Operator::sub(left, right);
            break;
        case TokenType::Multiply:
            result = Operator::multiply(left, right);
            break;
        case TokenType::Divide:
            result = Operator::divide(left, right);
            break;
        case TokenType::Modulo:
            result = Operator::modulo(left, right);
            break;
        default:
            throw UnsupportedOperation();
        }
    }
    catch (UnsupportedOperation)
    {
        throw RuntimeError("unsupported binary operation", {
                               .category = ErrorCategory::TypeError, .code = ErrorCode::UnsupportedOperation,
                               .identifier = getStringForType(node.op.type),
                               .primary = left.description(), .secondary = right.description(), .currentLine = 0
                           }, m_stackTrace);
    }
    catch (DivisionByZero)
    {
        throw RuntimeError("division by zero", {
                               .category = ErrorCategory::ZeroDivisionError, .code = ErrorCode::DivisionByZero,
                               .currentLine = 0
                           }, m_stackTrace
        );
    }
}

void EvaluateVisitor::visit(UnaryNode& node)
{
    node.child->accept(*this);
    auto value = result;
    if (node.op.type == TokenType::Not)
    {
        result = !value.isTruthy();
        return;
    }
    if (node.op.type == TokenType::Plus)
    {
        if (value.isNumber())
        {
            result = value;
            return;
        }
        throw RuntimeError("unsupported operation",
                           {. category = ErrorCategory::TypeError, .code = ErrorCode::UnsupportedOperation},
                           m_stackTrace
        );
    }
    if (node.op.type == TokenType::Minus)
    {
        if (value.isNumber())
        {
            result = -value.asNumber();
            return;
        }
        throw RuntimeError("unsupported operation", {
                               .category = ErrorCategory::TypeError, .code = ErrorCode::UnsupportedOperation
                           }, m_stackTrace);
    }
}

void EvaluateVisitor::visit(IsNode& node)
{
    node.value->accept(*this);
    if (result.isEnumObj())
    {
        const auto& enumValuePtr = result.asEnumPtr();
        node.enumVariant->accept(*this);
        if (result.isEnumVariantReference())
        {
            const auto& enumVariantPtr = result.asEnumVariantRef();
            if (enumValuePtr->type->typeName == enumVariantPtr.type->typeName)
            {
                if (enumValuePtr->variantIndex == enumVariantPtr.variantIndex)
                {
                    result = true;
                    return;
                }
                result = false;
            }
            result = false;
        }
        throw RuntimeError("right operand should be enum variant", {
                               .category = ErrorCategory::TypeError, .code = ErrorCode::InvalidRightOperandForIs,
                               .currentLine = 0
                           }, m_stackTrace);
    }
    throw RuntimeError("left operand for is should be enum object", {
                           .category = ErrorCategory::TypeError, .code = ErrorCode::InvalidLeftOperandForIs,
                           .currentLine = 0
                       }, m_stackTrace);
}

void EvaluateVisitor::visit(IndexNode& node)
{
    node.operand->accept(*this);
    auto operand = std::move(result);
    node.subscript->accept(*this);
    auto subscript = std::move(result);
    if (operand.isArray())
    {
        if (subscript.isNumber())
        {
            if (subscript.asNumber() < operand.asArrayPtr()->size())
            {
                result = operand.asArrayPtr()->at(subscript.asNumber());
                return;
            }
            throw RuntimeError("index out of bounds", {
                                   .category = ErrorCategory::IndexError, .code = ErrorCode::IndexOutOfBounds,
                                   .currentLine = 0
                               }, m_stackTrace);
        }
        throw RuntimeError("invalid index type", {
                               .category = ErrorCategory::TypeError, .code = ErrorCode::InvalidIndexType,
                               .currentLine = 0
                           }, m_stackTrace);
    }
    if (operand.isString())
    {
        if (subscript.isNumber())
        {
            if (subscript.asNumber() < operand.asString().size())
            {
                result = operand.asString().at(subscript.asNumber());
                return;
            }
            throw RuntimeError("index out of bounds", {
                                   .category = ErrorCategory::IndexError, .code = ErrorCode::IndexOutOfBounds,
                                   .currentLine = 0,
                               }, m_stackTrace);
        }
        throw RuntimeError("invalid index type", {
                               .category = ErrorCategory::TypeError, .code = ErrorCode::InvalidIndexType,
                               .currentLine = 0
                           }, m_stackTrace);
    }
    if (operand.isMap())
    {
        if (auto iter = operand.asMapPtr()->find(subscript); iter != operand.asMapPtr()->end())
        {
            result = iter->second;
            return;
        }
        throw RuntimeError("key error", {.category = ErrorCategory::KeyError, .code = ErrorCode::MapKeyNotFound},
                           m_stackTrace);
    }
    throw RuntimeError("invalid operand type for subscripting operation", {
                           .category = ErrorCategory::TypeError, .code = ErrorCode::NotSubscriptable, .currentLine = 0
                       }, m_stackTrace);
}

RuntimeValue* EvaluateVisitor::resolveLValue(ExpressionNode& node)
{
    if (auto* varNode = dynamic_cast<VariableNode*>(&node))
    {
        if (m_context.env->hasVariable(varNode->identifierName))
            return &m_context.env->getReference(varNode->identifierName).value;
        throw RuntimeError("undefined variable",
                           {.category = ErrorCategory::NameError, .code = ErrorCode::VariableUndefined}, m_stackTrace);
    }
    if (auto* indexNode = dynamic_cast<IndexNode*>(&node))
    {
        indexNode->operand->accept(*this);
        auto operand = result;
        indexNode->subscript->accept(*this);
        auto subscript = result;
        if (operand.isArray())
        {
            if (subscript.isNumber())
            {
                if (subscript.asNumber() < operand.asArrayPtr()->size())
                    return &operand.asArrayPtr()->at(subscript.asNumber());
                throw RuntimeError("index out of bounds", {
                                       .category = ErrorCategory::IndexError, .code = ErrorCode::IndexOutOfBounds
                                   }, m_stackTrace);
            }
            throw RuntimeError("invalid index type",
                               {.category = ErrorCategory::TypeError, .code = ErrorCode::InvalidIndexType},
                               m_stackTrace);
        }
        if (operand.isMap())
        {
            if (auto iter = operand.asMapPtr()->find(subscript); iter != operand.asMapPtr()->end())
            {
                return &operand.asMapPtr()->at(subscript);
            }
            throw RuntimeError("key error", {.category = ErrorCategory::KeyError, .code = ErrorCode::MapKeyNotFound},
                               m_stackTrace);
        }
    }
}

void EvaluateVisitor::visit(MemberAccessNode& node)
{
    node.obj->accept(*this);
    auto obj = std::move(result);
    if (obj.isModule())
    {
        try
        {
            result = obj.asModulePtr()->getMember(node.getMemberName());
            return;
        }
        catch (UndefinedVariable)
        {
            throw RuntimeError("member access error", {
                                   .category = ErrorCategory::AttributeError, .code = ErrorCode::MissingAttribute
                               }, m_stackTrace);
        }
    }
    if (obj.isStructObj())
    {
        if (obj.asStructObjPtr()->hasMemberField(node.getMemberName()))
        {
            result = obj.asStructObjPtr()->getMemberVal(node.getMemberName());
            return;
        }
        if (obj.asStructObjPtr()->hasMethod(node.getMemberName()))
        {
            result = std::static_pointer_cast<Callable>(
                std::make_shared<BoundMethod>(obj.asStructObjPtr()->getMethod(node.getMemberName()), obj));
            return;
        }
        throw RuntimeError("no struct member with the identifier", {
                               .category = ErrorCategory::AttributeError, .code = ErrorCode::StructMemberNotFound
                           }, m_stackTrace);
    }
    if (obj.isEnumObj())
    {
        if (obj.asEnumPtr()->hasMemberField(node.getMemberName()))
        {
            result = obj.asEnumPtr()->getMemberValue(node.getMemberName());
        }
        throw RuntimeError("no enum variant field with the identifier", {
                               .category = ErrorCategory::AttributeError, .code = ErrorCode::EnumMemberNotFound
                           }, m_stackTrace);
    }
    if (obj.isTypeReference())
    {
        if (auto enumTypeRef = dynamic_cast<EnumType*>(obj.asTypeRef().type))
        {
            for (std::size_t i = 0; i < enumTypeRef->variants.size(); ++i)
            {
                if (enumTypeRef->variants[i].name == node.getMemberName())
                {
                    if (enumTypeRef->variants[i].fields.empty())
                    {
                        result = std::make_shared<EnumValue>(enumTypeRef, i, std::vector<RuntimeValue>{});
                        return;
                    }
                    result = EnumVariantReference(enumTypeRef, i);
                    return;
                }
            }
            throw RuntimeError("enum variant not found", {
                                   .category = ErrorCategory::AttributeError, .code = ErrorCode::EnumVariantNotFound
                               }, m_stackTrace);
        }
        throw RuntimeError("type does not support member access", {
                               .category = ErrorCategory::AttributeError, .code = ErrorCode::InvalidReceiver
                           }, m_stackTrace);
    }
    if (obj.isArray())
    {
        auto module = m_context.env->getReference("ARRAYSTDLIB").value.asModulePtr();
        if (module->hasMember(node.getMemberName()))
        {
            result = std::static_pointer_cast<Callable>(
                std::make_shared<BoundMethod>(module->getMember(node.getMemberName()).asCallableObj(), obj));
            return;
        }
        throw RuntimeError("member access error", {
                               .category = ErrorCategory::AttributeError, .code = ErrorCode::ObjectMethodNotFound
                           }, m_stackTrace);
    }
    if (obj.isString())
    {
        auto module = m_context.env->getReference("STRINGSTDLIB").value.asModulePtr();
        if (module->hasMember(node.getMemberName()))
        {
            result = std::static_pointer_cast<Callable>(
                std::make_shared<BoundMethod>(module->getMember(node.getMemberName()).asCallableObj(), obj));
            return;
        }
        throw RuntimeError("member access error", {
                               .category = ErrorCategory::AttributeError, .code = ErrorCode::ObjectMethodNotFound
                           }, m_stackTrace);
    }
    if (obj.isMap())
    {
        auto module = m_context.env->getReference("MAPSTDLIB").value.asModulePtr();
        if (module->hasMember(node.getMemberName()))
        {
            result = std::static_pointer_cast<Callable>(
                std::make_shared<BoundMethod>(module->getMember(node.getMemberName()).asCallableObj(), obj));;
            return;
        }
        throw RuntimeError("member access error", {
                               .category = ErrorCategory::AttributeError, .code = ErrorCode::ObjectMethodNotFound
                           }, m_stackTrace);
    }
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
        return std::make_shared<StructInstance>(type, fields);
    }
    throw std::runtime_error("Constructor arguments arity error");
}


RuntimeValue constructor(EnumVariantReference typeRef, vector<RuntimeValue> args)
{
    return std::make_shared<EnumValue>(typeRef.type, typeRef.variantIndex, args);
}

void EvaluateVisitor::visit(FunctionCallNode& node)
{
    vector<RuntimeValue> args{};
    for (const auto& argExp : node.arguments)
    {
        argExp->accept(*this);
        args.push_back(std::move(result));
    }
    node.identifier->accept(*this);
    auto obj = std::move(result);
    if (obj.isTypeReference())
    {
        if (auto structType = dynamic_cast<StructType*>(obj.asTypeRef().type))
        {
            result = constructor(structType, std::move(args));
            return;
        }
        throw RuntimeError("object is not callable",
                           {.category = ErrorCategory::TypeError, .code = ErrorCode::NotCallable}, m_stackTrace);
    }
    if (obj.isEnumVariantReference())
    {
        result = constructor(obj.asEnumVariantRef(), args);
        return;
    }
    if (obj.isCallable())
    {
        try
        {
            result = obj.asCallableObj()->call(args);
            return;
        }
        catch (ArityDiagnostic& ad)
        {
            throw RuntimeError("arity error", {.category = ErrorCategory::ArityError}, m_stackTrace);
        }
        catch (AssertionError)
        {
            throw RuntimeError("assertion failure", {
                                   .category = ErrorCategory::AssertionError, .code = ErrorCode::AssertionFailure
                               }, m_stackTrace);
        }
        catch (PanicError)
        {
            throw RuntimeError("panic error", {.category = ErrorCategory::PanicError, .code = ErrorCode::PanicAbort},
                               m_stackTrace);
        }
    }
    throw RuntimeError("object is not callable", {.category = ErrorCategory::TypeError, .code = ErrorCode::NotCallable},
                       m_stackTrace);
}

void EvaluateVisitor::visit(AssignmentNode& node)
{
    node.rvalue->accept(*this);
    auto rvalue = std::move(result);
    auto lvalue = resolveLValue(*node.lvalue);
    *lvalue = rvalue;
    result = rvalue;
}

void EvaluateVisitor::visit(CompoundAssignmentNode& node)
{
    node.rvalue->accept(*this);
    auto rvalue = std::move(result);
    auto lvalue = resolveLValue(*node.lvalue);
    if (node.op.type == TokenType::PlusEqual)
    {
        *lvalue = Operator::add(*lvalue, rvalue);
        result = *lvalue;
        return;
    }
    if (node.op.type == TokenType::MinusEqual)
    {
        *lvalue = Operator::sub(*lvalue, rvalue);
        result = *lvalue;
        return;
    }
    if (node.op.type == TokenType::MultiplyEqual)
    {
        *lvalue = Operator::multiply(*lvalue, rvalue);
        result = *lvalue;
        return;
    }
    if (node.op.type == TokenType::DivideEqual)
    {
        *lvalue = Operator::divide(*lvalue, rvalue);
        result = *lvalue;
        return;
    }
}

void EvaluateVisitor::visit(BlockExpressionNode& node)
{
    auto previous = m_context.env;
    ScopeGuard(previous, m_context.env);
    for (const auto& stmt : node.statements)
    {
        stmt->accept(*this);
    }
    node.resultExpr->accept(*this);
}

void EvaluateVisitor::visit(MatchPatternNode& node)
{
    node.scrutinee->accept(*this);
    auto scrutinee = std::move(result);
    for (const auto& arm : node.armsExpr)
    {
        if (arm.pattern->matches(scrutinee, m_context))
        {
            arm.expr->accept(*this);
            return;
        }
    }
    throw std::runtime_error("impossible case");
}

void EvaluateVisitor::visit(ExpressionStatementNode& node)
{
    node.expressionStmt->accept(*this);
}

void EvaluateVisitor::visit(StructDeclarationNode& node)
{
}

void EvaluateVisitor::visit(EnumDeclarationNode& node)
{
}

void EvaluateVisitor::visit(FunctionDeclarationNode& node)
{
}

void EvaluateVisitor::visit(DeclarationNode& node)
{
    RuntimeValue rvalue = Uninitialized();
    if (node.rvalue)
    {
        node.rvalue->accept(*this);
        rvalue = result;
    }
    if (auto lvalue = dynamic_cast<VariableNode*>(node.lvalue.get()))
    {
        try
        {
            m_context.env->declare(lvalue->getIdentifierName(), {rvalue, 0});
        }
        catch (Redeclaration)
        {
            throw RuntimeError("redeclaration error", {
                                   .category = ErrorCategory::RedeclarationError,
                                   .code = ErrorCode::VariableRedeclaration
                               }, m_stackTrace);
        }
    }
}

void EvaluateVisitor::visit(ReturnNode& node)
{
    node.returnStatement->accept(*this);
    throw ThrowSignal(result);
}

void EvaluateVisitor::visit(BlockNode& node)
{
    auto previous = m_context.env;
    ScopeGuard(previous, m_context.env);

    for (const auto& stmt : node.statements)
    {
        stmt->accept(*this);
    }
}

void EvaluateVisitor::visit(IfNode& node)
{
    auto previous = m_context.env;
    ScopeGuard(previous, m_context.env);

    node.condition->accept(*this);

    auto condition = result.isTruthy();
    if (condition)
    {
        node.thenStatement->accept(*this);
    }
}

void EvaluateVisitor::visit(WhileNode& node)
{
    auto previous = m_context.env;
    while (true)
    {
        node.condition->accept(*this);
        if (result.isTruthy())
        {
            try
            {
                node.statement->accept(*this);
            }
            catch (BreakNode)
            {
                break;
            }
            catch (ContinueNode)
            {
                continue;
            }
        }
        else break;
    }
}

void EvaluateVisitor::visit(ForNode& node)
{
    auto previous = m_context.env;
    ScopeGuard(previous, m_context.env);
    node.initializer->accept(*this);
    while (true)
    {
        node.condition->accept(*this);
        if (result.isTruthy())
        {
            try
            {
                node.statement->accept(*this);
                node.expr->accept(*this);
            }
            catch (BreakSignal)
            {
                node.expr->accept(*this);
                break;
            }
            catch (ContinueSignal)
            {
                node.expr->accept(*this);
                continue;
            }
        }
    }
}

void EvaluateVisitor::visit(ForEachNode& node)
{
}

void EvaluateVisitor::visit(BreakNode& node)
{
    throw BreakSignal();
}

void EvaluateVisitor::visit(ContinueNode& node)
{
    throw BreakSignal();
}

void EvaluateVisitor::visit(EmptyNode& node)
{
}

void EvaluateVisitor::visit(TryCatch& node)
{
    try
    {
        auto previous = m_context.env;
        ScopeGuard(previous, m_context.env);
        node.tryStatement->accept(*this);
    }
    catch (ThrowSignal& t)
    {
        auto previous = m_context.env;
        ScopeGuard(previous, m_context.env);
        m_context.env->declare(node.catches[0].name, {t.val, 0});
        node.catches[0].blockStatement->accept(*this);
    }
}

void EvaluateVisitor::visit(ThrowNode& node)
{
    node.throwValExp->accept(*this);
    throw ThrowSignal(std::move(result));
}

void EvaluateVisitor::visit(ImportNode& node)
{
    if (node.isStdLib)
    {
        const string& identifier = std::format("std.{}", node.file);
        if (auto itr = m_context.module->loadedModules.find(identifier); itr == m_context.module->loadedModules.end())
        {
            auto programNode = loadStdlib(node.file, *m_context.env);
            auto previous = m_context.env;
            if (node.file == "core")
            {
                m_context.env = m_context.builtin;
                programNode->accept(*this);
            }
            else
            {
                m_context.env = std::make_shared<Environment>();
                programNode->accept(*this);
                m_context.module->loadedModules.emplace(identifier, ModuleCtx{std::move(programNode), m_context.env});
            }

            m_context.env = previous;

            if (node.file == "array" || node.file == "map" || node.file == "string")
            {
                m_context.builtin->declare(std::format("{}STDLIB", node.file),
                                           {
                                               std::make_shared<Module>(m_context.module->loadedModules[identifier].env,
                                                                        "array"),
                                               0
                                           });
            }
        }
        return;
    }
    const string filePath{std::format("{}/{}", m_context.workingDir, node.file)};
    std::ifstream is{filePath};
    if (!is)
    {
        throw std::runtime_error("fil o fsoun");
    }
    TokenStream ts{is};
    auto programNode = parseProgram(ts);

    if (auto itr = m_context.module->loadedModules.find(filePath); itr == m_context.module->loadedModules.end())
    {
        m_context.module->loadedModules.emplace(filePath, ModuleCtx{std::move(programNode), nullptr});

        auto previous = m_context.env;
        m_context.env = std::make_shared<Environment>();
        m_context.env->parent = m_context.builtin;

        programNode->accept(*this);
        m_context.module->loadedModules[filePath].env = m_context.env;

        m_context.env = previous;
        m_context.env->declare(node.alias, {
                                   std::make_shared<Module>(m_context.module->loadedModules[filePath].env, node.file), 0
                               });
    }
    else
    {
        m_context.env->declare(node.alias, {
                                   std::make_shared<Module>(m_context.module->loadedModules[filePath].env, node.file), 0
                               });
    }

    // programNode->accept(*this);
}

void EvaluateVisitor::visit(ProgramNode& node)
{
    for (const auto& stmt : node.statements)
    {
        stmt->accept(*this);
    }
}
