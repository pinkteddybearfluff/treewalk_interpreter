#include "RuntimeValue.h"

#include <complex>
#include <ranges>

#include "ast.h"
#include "environment.h"


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
        return "struct";
    case Kind::EnumObj:
        return "enum";
    case Kind::EnumVariantReference:
        return "enumVariantRef";
    case Kind::TypeReference:
        return "typeRef";
    }
}

RuntimeValue BoundMethod::call(const vector<RuntimeValue>& arguments, int line) const
{
    vector<RuntimeValue> args{self};
    for (const auto& arg : arguments)
    {
        args.push_back(arg);
    }
    return function->call(args, line);
}

RuntimeValue FunctionObject::call(const vector<RuntimeValue>& arguments, int line) const
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
                callEnv->variables.insert({
                    parameters[i].name, VariableInfo(parameters[i].defaultVal->evaluateNode(ctxNew).value, callLine)
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
        body->evaluateNode(ctxNew);
        return {};
    }
    catch (const ReturnSignal& r)
    {
        return r.value;
    }
}

RuntimeValue NativeFunction::call(const vector<RuntimeValue>& arguments, int line) const
{
    return fn(arguments);
}

void validateArity(string identifier, int expectedArgs, int actualArgs, bool variadic)
{
    if (!variadic && actualArgs > expectedArgs)
        throw ArityDiagnostic{
            identifier, ErrorKind::TooManyArguments, expectedArgs, actualArgs, variadic
        };
    if (actualArgs < expectedArgs)
    {
        throw ArityDiagnostic{identifier, ErrorKind::TooFewArguments, expectedArgs, actualArgs, variadic};
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

void printRuntimeValue(const RuntimeValue& value)
{
    switch (value.kind())
    {
    case RuntimeValue::Kind::String:
        std::cout << value.asString();
        break;
    case RuntimeValue::Kind::Boolean:
        std::cout << (value.asBoolean() ? "true" : "false");
        break;
    case RuntimeValue::Kind::Number:
        std::cout << value.asNumber();
        break;
    case RuntimeValue::Kind::Array:
        std::cout << "[";
        for (int i = 0; i < value.asArrayPtr()->size(); ++i)
        {
            if (i) std::cout << ", ";
            printRuntimeValue(value.asArrayPtr()->at(i));
        }
        std::cout << "]";
        break;
    case RuntimeValue::Kind::Callable:
        if (auto fn = dynamic_cast<FunctionObject*>(value.asCallableObj().get()))
            std::cout << "function " << "" << " at "
                << value.asCallableObj();
        if (auto fn = dynamic_cast<NativeFunction*>(value.asCallableObj().get()))
            std::cout << "built-in function" << "";
        break;
    case RuntimeValue::Kind::Null:
        std::cout << "null";
        break;
    case RuntimeValue::Kind::Module:
        std::cout << "module " << value.asModulePtr()->getName();
        break;
    case RuntimeValue::Kind::Uninitialized:
        std::cout << "Uninitialized";
        break;
    case RuntimeValue::Kind::Map:
        {
            std::cout << "{";

            bool first = true;

            for (const auto& [key, val] : *(value.asMapPtr()))
            {
                if (!first)
                    std::cout << ", ";

                first = false;
                if (key.isString())std::cout << '"';
                printRuntimeValue(key);
                if (key.isString())std::cout << '"';
                std::cout << ": ";
                if (val.isString())std::cout << '"';
                printRuntimeValue(val);
                if (val.isString())std::cout << '"';
            }
            std::cout << "}";
            break;
        }
    case RuntimeValue::Kind::StructObj:
        {
            for (const auto& [fieldName, value] : value.asStructObjPtr()->fields)
            {
                cout << fieldName << ": ";
                printRuntimeValue(value);
                cout << "\n";
            }
            for (const auto& name : value.asStructObjPtr()->type->methods | std::views::keys)
            {
                cout << name;
            }
            break;
        }
    case RuntimeValue::Kind::EnumObj:
        {
            cout << value.asEnumPtr()->type->typeName << "." << value.asEnumPtr()->type->variants[value.asEnumPtr()->
                variantIndex].name;
            if (!value.asEnumPtr()->fields.empty())
            {
                cout << "(";
                for (std::size_t i = 0; i < value.asEnumPtr()->fields.size(); ++i)
                {
                    printRuntimeValue(value.asEnumPtr()->fields[i]);
                    if (i < value.asEnumPtr()->fields.size() - 1)
                    {
                        cout << ", ";
                    }
                }
                cout << ")";
            }
            break;
        }
    case RuntimeValue::Kind::EnumVariantReference:
        {
            cout << "<enum constructor " << value.asEnumVariantRef().type->typeName << "." << value.asEnumVariantRef().
                type->variants[value.asEnumVariantRef().variantIndex].name << ">";
        }
    case RuntimeValue::Kind::TypeReference:
        {
            if (auto* structType = dynamic_cast<StructType*>(value.asTypeRef().type))
            {
                cout << "<struct " << structType->name << ">";
            }
            if (auto* enumType = dynamic_cast<EnumType*>(value.asTypeRef().type))
            {
                cout << "<enum " << enumType->typeName << ">";
            }
        }
    }
}

RuntimeValue& Module::getMember(const std::string& memberName)
{
    return env->getReference(memberName).value;
}

const RuntimeValue& Module::getMember(const std::string& memberName) const
{
    return env->getReference(memberName).value;
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

namespace Operator
{
    RuntimeValue add(const RuntimeValue& a, const RuntimeValue& b)
    {
        if ((a.isNumber() && b.isNumber()) || (a.isBoolean() && b.isBoolean()) || (a.isNumber() && b.isBoolean()) || (a.
            isBoolean() && b.isNumber()))
            return a.asNumber() + b.asNumber();

        if (a.isString() && b.isString()) return a.asString() + b.asString();
        if (a.isNumber() && b.isString()) return b.asString() + std::to_string(a.asNumber());
        if (a.isString() && b.isNumber()) return a.asString() + std::to_string(b.asNumber());
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
            isNumber() || b.isStructObj()))
            return false;
        if (b.isNull() && (a.isString() || a.isArray() || a.isBoolean() || a.isCallable() || a.isModule() || a.
            isNumber() || b.isStructObj()))
            return false;
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
}
