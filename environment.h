#ifndef INTERPRETER_STACKS_H
#define INTERPRETER_STACKS_H

#include <map>
#include <memory>

#include "RuntimeValue.h"
#include "RuntimeError.h"

using std::map;
using std::shared_ptr;

class BlockNode;
class ProgramNode;

constexpr bool DEBUG_ENV = true;


struct VariableInfo
{
    VariableInfo(RuntimeValue value, int line) : value{value}, declarationLine{line}
    {
    }

    RuntimeValue value;
    int declarationLine;
};

// Represents lexical scope.
//
// Variables are declared in current environment.
// To lookup variables starts from current Environment -> parent Environment -> Global Enviroment
struct Environment
{
    std::map<string, VariableInfo> variables;
    shared_ptr<Environment> parent;
    VariableInfo& getReference(const string& identifier);
    void declare(string name, VariableInfo data);
};

struct ModuleManager
{
    map<string, std::unique_ptr<ProgramNode>> loadedModules;
};

struct InterpreterContext
{
    shared_ptr<Environment> env;
    shared_ptr<ModuleManager> module;
    string currentFile;
    string workingDir;
};

#endif //INTERPRETER_STACKS_H
