#include "stacks.h"

#include <stdexcept>
#include <iostream>

RuntimeValue::Kind RuntimeValue::kind() const
{
    if (isString()) return Kind::String;
    if (isNumber()) return Kind::Number;
    if (isBoolean()) return Kind::Boolean;
    if (isArray()) return Kind::Array;
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
    }
}

void EnvironmentStack::popScope()
{
    if (isEmpty())
        throw std::runtime_error("no scopes to pop");
    scopes.pop_back();
}

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

void EnvironmentStack::assign(string name, RuntimeValue value)
{
    bool idenExists = false;
    for (int i = scopes.size() - 1; i >= 0; --i)
    {
        auto iter = scopes[i].find(name);
        if (iter != scopes[i].end())
        {
            idenExists = true;
            iter->second = value;
            return;
        }
    }
    if (!idenExists)
    {
        Environment& env = scopes.back();
        env[name] = value;
    }
}

void EnvironmentStack::declare(string name, RuntimeValue value)
{
    for (int i = scopes.size() - 1; i >= 0; --i)
    {
        auto iter = scopes[i].find(name);
        if (iter != scopes[i].end())
            throw std::runtime_error("variable " + name + " already exists");
    }
    Environment& env = scopes.back();
    env[name] = value;
}

RuntimeValue& EnvironmentStack::get(const string& name)
{
    for (int i = scopes.size() - 1; i >= 0; --i)
    {
        auto iter = scopes[i].find(name);
        if (iter != scopes[i].end())
            return iter->second;
    }
    throw std::runtime_error("Undefined variable");
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
            printRuntimeValue(var.second);
            std::cout << "\n";
        }
        ++i;
    }
}
