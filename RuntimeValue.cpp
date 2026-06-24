#include "RuntimeValue.h"
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
    }
}

RuntimeValue FunctionObject::call(const vector<RuntimeValue>& arguments, int line) const
{
    try
    {
        if (variadic)
            validateArity(f_name, parameters.size() + 1, arguments.size(), variadic);
        else validateArity(f_name, parameters.size(), arguments.size(), variadic);
    }
    catch (const ArityDiagnostic& ad)
    {
        throw RuntimeError("arity error", {
                               .category = ErrorCategory::ArityError, .kind = ad.kind, .identifier = ad.identifier,
                               .currentLine = line,
                               .expected = ad.expected, .actual = ad.actual, .variadic = ad.variadic
                           });
    }
    auto callEnv = std::make_shared<Environment>();
    callEnv->parent = closure;
    InterpreterContext ctxNew = {callEnv, {}};
    auto callerGuard = CallDepthGuard(gCallDepth);
    if (gCallDepth >= maxCallDepth)
        throw MaxRecursion();
    if (!variadic)
    {
        for (int i = 0; i < parameters.size(); ++i)
        {
            callEnv->variables.insert({parameters[i], VariableInfo(arguments[i], callLine)});
        }
    }
    else
    {
        for (int i = 0; i < parameters.size(); ++i)
        {
            callEnv->variables.insert({parameters[i], VariableInfo(arguments[i], callLine)});
        }
        vector<RuntimeValue> restArgs;
        for (int i = static_cast<int>(parameters.size()); i < arguments.size(); ++i)
        {
            restArgs.push_back(arguments[i]);
        }
        auto args = std::make_shared<RuntimeValue::Array>(restArgs);
        callEnv->variables.insert({variadicParamName, VariableInfo(args, callLine)});
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
    try
    {
        return fn(arguments);
    }
    catch (const ArityDiagnostic& ad)
    {
        throw RuntimeError("arrity error", {
                               .category = ErrorCategory::ArityError,
                               .kind = ad.kind, .identifier = ad.identifier, .currentLine = line,
                               .expected = ad.expected, .actual = ad.actual,
                               .variadic = ad.variadic
                           });
    }
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
            isNumber()))
            return false;
        if (b.isNull() && (a.isString() || a.isArray() || a.isBoolean() || a.isCallable() || a.isModule() || a.
            isNumber()))
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
