#ifndef INTERPRETER_STACKS_H
#define INTERPRETER_STACKS_H

#include <string>
#include <vector>
#include <map>
#include <variant>

using std::string;
using std::vector;
using std::map;


class RuntimeValue
{
public:
    enum class Kind
    {
        Number,
        String,
        Boolean
    };

    using Type = std::variant<double, bool, string, std::monostate>;

    RuntimeValue(Type d) : data{d}
    {
    };

    RuntimeValue() : data{std::monostate{}}
    {
    };

    [[nodiscard]] bool isNumber() const
    {
        return std::holds_alternative<double>(data);
    };

    [[nodiscard]] bool isString() const
    {
        return std::holds_alternative<string>(data);
    };

    [[nodiscard]] bool isBoolean() const
    {
        return std::holds_alternative<bool>(data);
    };

    [[nodiscard]] double asNumber() const { return std::get<double>(data); };
    [[nodiscard]] const string& asString() const { return std::get<string>(data); };
    [[nodiscard]] bool asBoolean() const { return std::get<bool>(data); };
    Kind kind() const;

private:
    Type data;
};

constexpr bool DEBUG_ENV = true;
using Environment = map<string, RuntimeValue>;

class EnvironmentStack
{
public:
    EnvironmentStack() : scopes{{}}
    {
    };
    void pushScope();
    void pushScope(Environment& env);
    void popScope();
    bool isEmpty();

    RuntimeValue get(string name);
    void assign(string name, RuntimeValue value);
    void declare(string name, RuntimeValue value);
    void debugEnvPrint();

private:
    vector<Environment> scopes;
};

#endif //INTERPRETER_STACKS_H
