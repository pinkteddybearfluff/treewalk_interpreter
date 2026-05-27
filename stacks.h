#ifndef INTERPRETER_STACKS_H
#define INTERPRETER_STACKS_H

#include <string>
#include <vector>
#include <map>
#include <memory>
#include <variant>

using std::string;
using std::vector;
using std::map;
using std::shared_ptr;


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

    [[nodiscard]] double asNumber() const { return std::get<double>(data); };
    [[nodiscard]] const string& asString() const { return std::get<string>(data); };
    [[nodiscard]] bool asBoolean() const { return std::get<bool>(data); };
    [[nodiscard]] shared_ptr<Array> asArrayPtr() const { return std::get<shared_ptr<Array>>(data); };
    Kind kind() const;

private:
    Type data;
};


void printRuntimeValue(const RuntimeValue& val);

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

    RuntimeValue& get(const string& name);
    void assign(string name, RuntimeValue value);
    void declare(string name, RuntimeValue value);
    void debugEnvPrint();

private:
    vector<Environment> scopes;
};

#endif //INTERPRETER_STACKS_H
