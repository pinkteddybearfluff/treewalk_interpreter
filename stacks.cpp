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

void EnvironmentStack::popScope()
{
    if (isEmpty())
        throw std::runtime_error("no scopes to pop");
    scopes.pop_back();
}

// VariableInfo& Environment::getReference(const string& identifier)
// {
//     auto* iter = variables.find()
// }

void EnvironmentStack::pushScope()
{
    scopes.push_back({});
}

void EnvironmentStack::pushScope(Environment& env)
{
    scopes.push_back(env);
}

bool EnvironmentStack::isEmpty()
{
    if (scopes.size() <= 1)
        return true;
    return false;
}


void EnvironmentStack::declare(string name, VariableInfo data)
{
    for (int i = scopes.size() - 1; i >= 0; --i)
    {
        auto iter = scopes[i].find(name);
        if (iter != scopes[i].end())
            throw Redeclaration();
    }
    Environment& env = scopes.back();
    env[name] = data;
}

VariableInfo& EnvironmentStack::get(const string& name)
{
    for (int i = scopes.size() - 1; i >= 0; --i)
    {
        auto iter = scopes[i].find(name);
        if (iter != scopes[i].end())
            return iter->second;
    }
    throw UndefinedVariable();
}

void EnvironmentStack::debugEnvPrint()
{
    int i = 0;
    for (auto& env : scopes)
    {
        std::cout << string(2 * i, ' ') << "Level " << i << ":\n";
        for (auto& var : env)
        {
            std::cout << string(i * 2, ' ') << var.first << ": ";
            printRuntimeValue(var.second.value);
            std::cout << "\n";
        }
        ++i;
    }
}
