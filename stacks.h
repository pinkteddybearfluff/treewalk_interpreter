#ifndef INTERPRETER_STACKS_H
#define INTERPRETER_STACKS_H

#include <string>
#include <vector>
#include <map>
#include <memory>
#include <stdexcept>
#include <variant>

using std::string;
using std::vector;
using std::map;
using std::shared_ptr;


constexpr bool DEBUG_ENV = true;

namespace color
{
    constexpr const char* reset = "\033[0m";
    constexpr const char* red = "\033[31m";
    constexpr const char* boldRed = "\033[1;31m";
    constexpr const char* white = "\033[37m";
    constexpr const char* boldWhite = "\033[1;37m";

    constexpr const char* magenta = "\033[35m";
    constexpr const char* boldMagenta = "\033[1;35m";
    constexpr const char* blue = "\033[34m";
    constexpr const char* boldBlue = "\033[1;34m";

    constexpr const char* cyan = "\033[36m";
    constexpr const char* boldCyan = "\033[1;36m";
    constexpr const char* green = "\033[32m";
    constexpr const char* boldGreen = "\033[1;32m";
}

class RuntimeValue
{
public:
    enum class Kind
    {
        Number,
        String,
        Boolean,
        Array
    };

    using Array = vector<RuntimeValue>;
    using Type = std::variant<double, bool, string, std::monostate, shared_ptr<Array>>;

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

    [[nodiscard]] bool isArray() const
    {
        return std::holds_alternative<shared_ptr<Array>>(data);
    }

    [[nodiscard]] bool isReducibleToBool() const;
    [[nodiscard]] bool isTruthy() const;
    [[nodiscard]] double asNumber() const { return std::get<double>(data); };
    [[nodiscard]] const string& asString() const { return std::get<string>(data); };
    string& getStringRef() { return std::get<string>(data); };
    [[nodiscard]] bool asBoolean() const { return std::get<bool>(data); };
    [[nodiscard]] shared_ptr<Array> asArrayPtr() const { return std::get<shared_ptr<Array>>(data); };

    [[nodiscard]] string description() const;
    [[nodiscard]] Kind kind() const;

private:
    Type data;
};


void printRuntimeValue(const RuntimeValue& val);

struct VariableInfo
{
    RuntimeValue value;
    int declarationLine;
};

using Environment = map<string, VariableInfo>;

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

    VariableInfo& get(const string& name);
    void assign(string name, VariableInfo data);
    void declare(string name, VariableInfo data);
    void debugEnvPrint();

private:
    vector<Environment> scopes;
};

class UndefinedVariable
{
};

class Redeclaration
{
};

#endif //INTERPRETER_STACKS_H
