#ifndef INTERPRETER_STACKS_H
#define INTERPRETER_STACKS_H

#include <string>
#include <vector>
#include <map>

using std::string;
using std::vector;
using std::map;

using Environment = map<string, double>;

class EnvironmentStack
{
public:
    EnvironmentStack() : scopes{{}}
    {
    };
    void pushScope();
    void popScope();
    bool isEmpty();

    double get(string name);
    void assign(string name, double value);
    void declare(string name, double value);
    void debugEnvPrint();

private:
    vector<Environment> scopes;
};

#endif //INTERPRETER_STACKS_H
