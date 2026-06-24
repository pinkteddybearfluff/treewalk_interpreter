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

void Environment::declare(string name, VariableInfo data)
{
    auto iter = variables.find(name);


    if (iter == variables.end())
    {
        variables.insert({name, data});
    }
    else throw Redeclaration();
}

