#include "RuntimeValue.h"

#include <complex>
#include <ranges>

#include "../ast/Ast.h"
#include "Environment.h"
#include "../stdlib/Stdlib.h"
#include "../visitors/EvaluateVisitor.h"


int gCallDepth{0};
constexpr int maxCallDepth{1000};

RuntimeValue::Kind RuntimeValue::kind() const
{
    if (isString()) return Kind::String;
    if (isNumber()) return Kind::Number;
    if (isBoolean()) return Kind::Boolean;
    if (isArray()) return Kind::Array;
    if (isNull()) return Kind::Null;
    if (isCallable()) return Kind::Callable;
    if (isModule()) return Kind::Module;
    if (isUninitialized()) return Kind::Uninitialized;
    if (isMap()) return Kind::Map;
    if (isStructObj()) return Kind::StructObj;
    if (isEnumObj()) return Kind::EnumObj;
    if (isEnumVariantReference()) return Kind::EnumVariantReference;
    if (isTypeReference()) return Kind::TypeReference;
}

//used for std::unordered_map
bool operator==(const RuntimeValue& lhs, const RuntimeValue& rhs)
{
    if (lhs.isBoolean() && rhs.isBoolean())
        return lhs.asBoolean() == rhs.asBoolean();
    if (lhs.isNull() && rhs.isNull()) return true;
    if (lhs.isString() && rhs.isString()) return lhs.asString() == rhs.asString();
    if (lhs.isNumber() && rhs.isNumber()) return lhs.asNumber() == rhs.asNumber();
    return false;
}

string RuntimeValue::description() const
{
    switch (kind())
    {
    case Kind::Number:
        return "number";
    case Kind::String:
        return "str";
    case Kind::Boolean:
        return "bool";
    case Kind::Array:
        return "array";
    case Kind::Null:
        return "null";
    case Kind::Callable:
        return "function";
    case Kind::Uninitialized:
        return "uninitialized";
    case Kind::Module:
        return "module";
    case Kind::Map:
        return "map";
    case Kind::StructObj:
        return "struct object";
    case Kind::EnumObj:
        return "enum object";
    case Kind::EnumVariantReference:
        return "enumVariantRef";
    case Kind::TypeReference:
        return "typeRef";
    }
}

RuntimeValue BoundMethod::call(const vector<RuntimeValue>& arguments) const
{
    vector<RuntimeValue> args{self};
    for (const auto& arg : arguments)
    {
        args.push_back(arg);
    }
    return function->call(args);
}

RuntimeValue FunctionObject::call(const vector<RuntimeValue>& arguments) const
{
    int defArgs{0};
    bool hasDefaultArgs{false};
    for (const auto& param : parameters)
    {
        if (param.defaultVal)
        {
            ++defArgs;
            hasDefaultArgs = true;
        }
    }
    validateArity(f_name, parameters.size() - defArgs, arguments.size(), hasDefaultArgs);

    auto callEnv = std::make_shared<Environment>();
    if (closure)
        callEnv->parent = closure;
    InterpreterContext ctxNew = {callEnv, {}};

    auto evaluateVisitor = EvaluateVisitor(ctxNew);
    auto callerGuard = CallDepthGuard(gCallDepth);
    if (gCallDepth >= maxCallDepth)
        throw MaxRecursion();

    if (!parameters.empty())
    {
        if (arguments.size() < parameters.size())
        {
            for (int i = 0; i < arguments.size(); ++i)
            {
                callEnv->variables.insert({parameters[i].name, VariableInfo(arguments[i], callLine)});
            }
            for (std::size_t i = arguments.size(); i < parameters.size(); ++i)
            {
                parameters[i].defaultVal->accept(evaluateVisitor);
                callEnv->variables.insert({
                    parameters[i].name, VariableInfo(std::move(evaluateVisitor.result), callLine)
                });
            }
        }
        else // arguments.size() > parameters.size()
        {
            for (int i = 0; i < parameters.size() - 1; ++i)
            {
                callEnv->variables.insert({parameters[i].name, VariableInfo(arguments[i], callLine)});
            }
            if (parameters[parameters.size() - 1].isVariadic)
            {
                vector<RuntimeValue> restArgs;
                for (std::size_t i = parameters.size() - 1; i < arguments.size(); ++i)
                {
                    restArgs.push_back(arguments[i]);
                }
                auto args = std::make_shared<RuntimeValue::Array>(restArgs);
                callEnv->variables.insert({parameters[parameters.size() - 1].name, VariableInfo(args, callLine)});
            }
            else
            {
                callEnv->variables.insert({
                    parameters[parameters.size() - 1].name, VariableInfo(arguments[parameters.size() - 1], callLine)
                });
            }
        }
    }
    try
    {
        body->accept(evaluateVisitor);
        return {};
    }
    catch (const ReturnSignal& r)
    {
        return r.value;
    }
}

RuntimeValue NativeFunction::call(const vector<RuntimeValue>& arguments) const
{
    return fn(arguments);
}

void validateArity(string identifier, int expectedArgs, int actualArgs, bool variadic)
{
    if (!variadic && actualArgs > expectedArgs)
        throw ArityDiagnostic{
            identifier, ErrorCode::TooManyArguments, expectedArgs, actualArgs, variadic
        };
    if (actualArgs < expectedArgs)
    {
        throw ArityDiagnostic{identifier, ErrorCode::TooFewArguments, expectedArgs, actualArgs, variadic};
    }
}

bool StructInstance::hasMethod(string mName)
{
    return type->methods.contains(mName);
}

shared_ptr<FunctionObject> StructInstance::getMethod(string mName)
{
    return type->methods[mName];
}

bool StructInstance::hasMemberField(string mName)
{
    return fields.contains(mName);
}

RuntimeValue StructInstance::getMemberVal(string mName)
{
    return fields[mName];
}

bool EnumValue::hasMemberField(string mName)
{
    for (std::size_t i = 0; i < type->variants[variantIndex].fields.size(); ++i)
    {
        if (type->variants[variantIndex].fields[i] == mName)return true;
    }
    return false;
}

RuntimeValue EnumValue::getMemberValue(string mName)
{
    for (std::size_t i = 0; i < type->variants[variantIndex].fields.size(); ++i)
    {
        if (type->variants[variantIndex].fields[i] == mName)
        {
            return fields[i];
        };
    }
    throw std::runtime_error("someone didn't check before if enum variant has the member");
}


bool RuntimeValue::isTruthy() const
{
    if (isArray())
    {
        if (!asArrayPtr()->empty())
            return true;
        return false;
    }
    if (isNumber())
    {
        if (static_cast<bool>(asNumber())) return true;
        return false;
    }
    if (isString())
    {
        if (!asString().empty()) return true;
        return false;
    }
    if (isBoolean())
    {
        if (asBoolean()) return true;
        return false;
    }
    if (isCallable()) return true;
    if (isModule())
    {
        return true;
    }
    if (isNull())return false;
    if (isUninitialized())
        throw std::runtime_error("Variable is not initialized");
    if (isStructObj())
        return true;
    if (isEnumObj())
        return true;
    return false;
}

void printRuntimeValue(std::ostream& os, const RuntimeValue& value)
{
    switch (value.kind())
    {
    case RuntimeValue::Kind::String:
    case RuntimeValue::Kind::Boolean:
    case RuntimeValue::Kind::Number:
    case RuntimeValue::Kind::Null:
    case RuntimeValue::Kind::Array:
    case RuntimeValue::Kind::Map:
    case RuntimeValue::Kind::StructObj:
    case RuntimeValue::Kind::EnumObj:
    case RuntimeValue::Kind::EnumVariantReference:
    case RuntimeValue::Kind::TypeReference:
        os << stdlib::stringify(value);
        break;
    case RuntimeValue::Kind::Callable:
        if (auto fn = dynamic_cast<FunctionObject*>(value.asCallableObj().get()))
            os << "function " << "" << " at "
                << value.asCallableObj();
        if (auto fn = dynamic_cast<NativeFunction*>(value.asCallableObj().get()))
            os << "built-in function" << "";
        break;
    case RuntimeValue::Kind::Module:
        os << "module " << value.asModulePtr()->getName();
        break;
    case RuntimeValue::Kind::Uninitialized:
        os << "Uninitialized";
        break;
    }
}

// RuntimeValue& Module::getMember(const std::string& memberName)
// {
//     if (env->hasVariable(memberName))
//         return env->getReference(memberName).value;
//     if (env->hasType(memberName))
//         return TypeReference{env->getType(memberName)};
// }

bool Module::hasMember(const std::string& memberName) const
{
    if (env->hasVariable(memberName))
        return true;
    if (env->hasType(memberName))
        return true;
    return false;
}

RuntimeValue Module::getMember(const std::string& memberName) const
{
    if (env->hasVariable(memberName))
        return env->getReference(memberName).value;
    if (env->hasType(memberName))
        return TypeReference{env->getType(memberName)};
}

std::size_t RuntimeValueHash::operator()(const RuntimeValue& value) const
{
    switch (value.kind())
    {
    case RuntimeValue::Kind::Number:
        return std::hash<double>{}(value.asNumber());

    case RuntimeValue::Kind::String:
        {
            return std::hash<std::string>{}(value.asString());
        }
    case RuntimeValue::Kind::Boolean:
        return std::hash<bool>{}(value.asBoolean());

    case RuntimeValue::Kind::Null:
        return 0;

    default:
        throw std::runtime_error(
            "value is not hashable");
    }
}

//Throw UnsupportedOperation or DivisionByError
namespace Operator
{
    RuntimeValue add(const RuntimeValue& a, const RuntimeValue& b)
    {
        if ((a.isNumber() && b.isNumber()) || (a.isBoolean() && b.isBoolean()) || (a.isNumber() && b.isBoolean()) || (a.
            isBoolean() && b.isNumber()))
            return a.asNumber() + b.asNumber();

        if (a.isString() && b.isString()) return a.asString() + b.asString();
        if (a.isNumber() && b.isString()) return b.asString() + stdlib::numberToString(a.asNumber());
        if (a.isString() && b.isNumber()) return a.asString() + stdlib::numberToString(b.asNumber());
        throw UnsupportedOperation();
    }

    RuntimeValue sub(const RuntimeValue& a, const RuntimeValue& b)
    {
        if ((a.isNumber() && b.isNumber()) || (a.isBoolean() && b.isBoolean()) || (a.isNumber() && b.isBoolean()) || (a.
            isBoolean() && b.isNumber()))
            return a.asNumber() - b.asNumber();
        throw UnsupportedOperation();
    }

    RuntimeValue multiply(const RuntimeValue& a, const RuntimeValue& b)
    {
        if ((a.isNumber() && b.isNumber()) || (a.isBoolean() && b.isBoolean()) || (a.isNumber() && b.isBoolean()) || (a.
            isBoolean() && b.isNumber()))
            return a.asNumber() * b.asNumber();
        throw UnsupportedOperation();
    }

    RuntimeValue divide(const RuntimeValue& a, const RuntimeValue& b)
    {
        if ((a.isNumber() && b.isNumber()) || (a.isBoolean() && b.isBoolean()) || (a.isNumber() && b.isBoolean()) || (a.
            isBoolean() && b.isNumber()))
        {
            if (static_cast<bool>(b.asNumber()))
                return a.asNumber() / b.asNumber();
            throw DivisionByZero();
        }
        throw UnsupportedOperation();
    }

    RuntimeValue modulo(const RuntimeValue& a, const RuntimeValue& b)
    {
        if ((a.isNumber() && b.isNumber()) || (a.isBoolean() && b.isBoolean()) || (a.isNumber() && b.isBoolean()) || (a.
            isBoolean() && b.isNumber()))
        {
            if (static_cast<bool>(b.asNumber()))
                return fmod(a.asNumber(), b.asNumber());
            throw DivisionByZero();
        }
        throw UnsupportedOperation();
    }

    RuntimeValue equal(const RuntimeValue& a, const RuntimeValue& b)
    {
        if (a.isNumber() && b.isNumber())
            return a.asNumber() == b.asNumber();
        if (a.isBoolean() && b.isBoolean())
            return a.asBoolean() == b.asBoolean();
        if (a.isString() && b.isString())
            return a.asString() == b.asString();
        if (a.isCallable() && b.isCallable())
            return a.asCallableObj()->f_name == b.asCallableObj()->f_name;
        if (a.isNull() && b.isNull())
            return true;
        if (a.isArray() && b.isArray())
        {
            const auto& aArrPtr = a.asArrayPtr();
            const auto& bArrPtr = b.asArrayPtr();
            if (aArrPtr->size() == bArrPtr->size())
            {
                for (int i = 0; i < aArrPtr->size(); ++i)
                {
                    if (!equal(aArrPtr->at(i), bArrPtr->at(i)).asBoolean()) return false;
                }
                return true;
            }
            return false;
        }
        if (a.isNull() && (b.isString() || b.isArray() || b.isBoolean() || b.isCallable() || b.isModule() || b.
            isNumber() || b.isStructObj() || b.isEnumObj() || b.isTypeReference()))
            return false;
        if (b.isNull() && (a.isString() || a.isArray() || a.isBoolean() || a.isCallable() || a.isModule() || a.
            isNumber() || a.isStructObj() || a.isEnumObj() || a.isTypeReference()))
            return false;
        if (a.isEnumObj() && b.isEnumObj())
        {
            if (a.asEnumPtr()->type == b.asEnumPtr()->type)
            {
                if (a.asEnumPtr()->variantIndex == b.asEnumPtr()->variantIndex)
                {
                    for (std::size_t i = 0; i < a.asEnumPtr()->fields.size(); ++i)
                    {
                        if (notEqual(a.asEnumPtr()->fields[i], b.asEnumPtr()->fields[i]).asBoolean()) return false;
                    }
                    return true;
                }
                else return false;
            }
            else return false;
        }
        if (a.isStructObj() && b.isStructObj())
        {
            if (a.asStructObjPtr()->type == b.asStructObjPtr()->type)
            {
                for (const auto& [fieldName, value] : a.asStructObjPtr()->fields)
                {
                    if (notEqual(value, b.asStructObjPtr()->fields[fieldName]).asBoolean())
                        return false;
                }
                return true;
            }
            else return false;
        }
        throw UnsupportedOperation();
    }

    RuntimeValue notEqual(const RuntimeValue& a, const RuntimeValue& b)
    {
        return !equal(a, b).asBoolean();
    }

    RuntimeValue greater(const RuntimeValue& a, const RuntimeValue& b)
    {
        if ((a.isNumber() && b.isNumber()) || (a.isBoolean() && b.isBoolean()) || (a.isNumber() && b.isBoolean()) || (a.
            isBoolean() && b.isNumber()))
            return a.asNumber() > b.asNumber();
        if (a.isString() && b.isString())
            return a.asString() > b.asString();
        throw UnsupportedOperation();
    }

    RuntimeValue greaterEqual(const RuntimeValue& a, const RuntimeValue& b)
    {
        if ((a.isNumber() && b.isNumber()) || (a.isBoolean() && b.isBoolean()) || (a.isNumber() && b.isBoolean()) || (a.
            isBoolean() && b.isNumber()))
            return a.asNumber() >= b.asNumber();
        if (a.isString() && b.isString())
            return a.asString() >= b.asString();
        throw UnsupportedOperation();
    }

    RuntimeValue less(const RuntimeValue& a, const RuntimeValue& b)
    {
        return !greaterEqual(a, b).asBoolean();
    }

    RuntimeValue lessEqual(const RuntimeValue& a, const RuntimeValue& b)
    {
        return !greater(a, b).asBoolean();
    }

    RuntimeValue logicalAnd(const RuntimeValue& a, const RuntimeValue& b)
    {
        return a.isTruthy() && b.isTruthy();
    }

    RuntimeValue logicalOr(const RuntimeValue& a, const RuntimeValue& b)
    {
        return a.isTruthy() || b.isTruthy();
    }
}
