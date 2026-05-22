#ifndef INTERPRETER_STACKS_H
#define INTERPRETER_STACKS_H

#include <string>
#include <vector>
#include <map>
#include <variant>

using std::string;
using std::vector;
using std::map;

using Type = std::variant<double, bool, string>;

constexpr bool DEBUG_ENV = false;
using Environment = map<string, Type>;

class EnvironmentStack
{
public:
    EnvironmentStack() : scopes{{}}
    {
    };
    void pushScope();
    void popScope();
    bool isEmpty();

    Type get(string name);
    void assign(string name, Type value);
    void declare(string name, Type value);
    void debugEnvPrint();

private:
    vector<Environment> scopes;
};

#endif //INTERPRETER_STACKS_H
