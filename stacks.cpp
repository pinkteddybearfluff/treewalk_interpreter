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

bool EnvironmentStack::isEmpty()
{
    if (scopes.size() <= 1)
        return true;
    return false;
}

void EnvironmentStack::assign(string name, double value)
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

void EnvironmentStack::declare(string name, double value)
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

double EnvironmentStack::get(string name)
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
            std::cout << string(i * 2, ' ') << var.first << ": " << var.second << '\n';
        }
        ++i;
    }
}
