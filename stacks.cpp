#include "stacks.h"
#include "ast.h"

#include <stdexcept>
#include <iostream>

RuntimeValue::Kind RuntimeValue::kind() const
{
    if (isString()) return Kind::String;
    if (isNumber()) return Kind::Number;
    if (isBoolean()) return Kind::Boolean;
    if (isArray()) return Kind::Array;
    if (isNull()) return Kind::Null;
    if (isFunctionObj()) return Kind::FunctionObject;
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
    case Kind::FunctionObject:
        return "functionobj";
    }
}

RuntimeValue FunctionObject::call(const vector<RuntimeValue>& arguments) const
{
    auto callEnv = std::make_shared<Environment>();
    callEnv->parent = closure;
    for (int i = 0; i < parameters.size(); ++i)
    {
        callEnv->variables[parameters[i]] = {arguments[i]};
    }
    try
    {
        body->evaluateNode(callEnv);
        return {};
    }
    catch (const ReturnSignal& r)
    {
        return r.value;
    }
}

bool RuntimeValue::isReducibleToBool() const
{
    if (isArray()) return true;
    if (isBoolean()) return true;
    if (isString()) return true;
    if (isNumber()) return true;
    return false;
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
    case RuntimeValue::Kind::FunctionObject:
        std::cout << "function";
        value.asFunctionObj().body->debugPrint(0);
        break;
    case RuntimeValue::Kind::Null:
        std::cout << "null";
        break;
    }
}


VariableInfo& Environment::getReference(const string& identifier)
{
    const auto& iter = variables.find(identifier);
    if (iter != variables.end())
    {
        return iter->second;
    }
    if (parent)
    {
        return parent->getReference(identifier);
    }
    throw UndefinedVariable();
}

void Environment::declare(string name, VariableInfo data)
{
    auto iter = variables.find(name);


    if (iter == variables.end())
        variables[name] = data;
    else throw Redeclaration();
}
