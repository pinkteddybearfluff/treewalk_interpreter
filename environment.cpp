#include "environment.h"
#include "ast.h"

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
    cout << identifier << " not found this\n";
    throw UndefinedVariable();
}

bool Environment::hasVariable(string name)
{
    const auto& iter = variables.find(name);
    if (iter != variables.end())
    {
        return true;
    }
    if (parent)
    {
        return parent->hasVariable(name);
    }
    return false;
}

void Environment::declare(string name, VariableInfo data)
{
    auto iter = variables.find(name);


    if (iter == variables.end())
    {
        variables.insert({name, data});
    }
    else throw Redeclaration();
}

bool Environment::hasType(string name) const
{
    if (types.contains(name))
    {
        return true;
    }
    if (parent)
    {
        return parent->hasType(name);
    }
    return false;
}

Type* Environment::getType(string name)
{
    if (types.contains(name))
    {
        return types[name].get();
    }
    if (parent)
    {
        return parent->getType(name);
    }
    throw std::runtime_error("no type name found");
}
