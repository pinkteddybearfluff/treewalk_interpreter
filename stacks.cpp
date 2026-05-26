#include "stacks.h"

#include <stdexcept>
#include <iostream>

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

void EnvironmentStack::assign(string name, Type value)
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

void EnvironmentStack::declare(string name, Type value)
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

Type EnvironmentStack::get(string name)
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
            if (std::holds_alternative<double>(var.second))
                std::cout << std::get<double>(var.second) << '\n';
            else if (std::holds_alternative<bool>(var.second))
                std::cout << std::get<bool>(var.second) << '\n';
            else if (std::holds_alternative<string>(var.second))
                std::cout << std::get<string>(var.second) << '\n';
        }
        ++i;
    }
}
