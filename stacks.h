#ifndef INTERPRETER_STACKS_H
#define INTERPRETER_STACKS_H

#include <functional>
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

class BlockNode;
struct Environment;
class RuntimeValue;

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

struct FunctionObject
{
    string f_name;
    vector<string> parameters;
    const BlockNode* body;
    shared_ptr<Environment> closure;
    bool variadic;
    string variadicParamName;
    RuntimeValue call(const vector<RuntimeValue>& arguments) const;
    RuntimeValue call(const vector<RuntimeValue>& arguments, const vector<RuntimeValue>& restArgs) const;
};

class MaxRecursion
{
};

struct NativeFunction
{
    string f_name;
    std::function<RuntimeValue(const vector<RuntimeValue>& args)> fn;

    RuntimeValue call(const vector<RuntimeValue>& arguments) const;
};

// Runtime value type is decided at run time.
// Allowing for dynamically type language like usage.
class RuntimeValue
{
public:
    enum class Kind
    {
        Number,
        String,
        Boolean,
        Array,
        Null,
        FunctionObject,
        NativeFunction,
    };

    using Array = vector<RuntimeValue>;
    using Type = std::variant<double, bool, string, std::monostate, shared_ptr<Array>, FunctionObject, NativeFunction>;

    RuntimeValue(Type d) : data{d}
    {
    };

    RuntimeValue(FunctionObject f) : data{f}
    {
    };

    RuntimeValue(NativeFunction nf) : data{nf}
    {
    };

    RuntimeValue(char c) : data{string(1, c)}
    {
    };

    RuntimeValue(double d) : data{d}
    {
    };

    RuntimeValue(string str) : data{str}
    {
    };

    RuntimeValue(int i) : data{static_cast<double>(i)}
    {
    };

    RuntimeValue(bool b) : data{b}
    {
    };

    RuntimeValue(shared_ptr<Array> spa) : data{spa}
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

    [[nodiscard]] bool isNull() const
    {
        return std::holds_alternative<std::monostate>(data);
    }

    [[nodiscard]] bool isFunctionObj() const
    {
        return std::holds_alternative<FunctionObject>(data);
    }

    [[nodiscard]] bool isNativeFunction() const
    {
        return std::holds_alternative<NativeFunction>(data);
    }

    [[nodiscard]] bool isReducibleToBool() const;
    [[nodiscard]] bool isTruthy() const;
    [[nodiscard]] double asNumber() const { return std::get<double>(data); };

    [[nodiscard]] double& getNumberRef()
    {
        return std::get<double>(data);
    };
    [[nodiscard]] const string& asString() const { return std::get<string>(data); }

    [[nodiscard]] std::monostate asNull() const { return std::get<std::monostate>(data); }
    string& getStringRef() { return std::get<string>(data); }
    [[nodiscard]] bool asBoolean() const { return std::get<bool>(data); }

    [[nodiscard]] bool& getBoolRef()
    {
        return std::get<bool>(data);
    };
    [[nodiscard]] shared_ptr<Array> asArrayPtr() const { return std::get<shared_ptr<Array>>(data); }
    [[nodiscard]] shared_ptr<Array>& getArrayPtrRef() { return std::get<shared_ptr<Array>>(data); }
    [[nodiscard]] FunctionObject asFunctionObj() const { return std::get<FunctionObject>(data); }
    [[nodiscard]] NativeFunction asNativeFunction() const { return std::get<NativeFunction>(data); }

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

// Represents lexical scope.
//
// Variables are declared in current environment.
// To lookup variables starts from current Environment -> parent Environment -> Global Enviroment
struct Environment
{
    std::map<string, VariableInfo> variables;
    shared_ptr<Environment> parent;
    int callDepthLevel{0};
    VariableInfo& getReference(const string& identifier);
    void declare(string name, VariableInfo data);
};

class CallDepthGuard
{
public:
    CallDepthGuard(int& callDepth) : depth{callDepth}
    {
        ++depth;
    }

    ~CallDepthGuard()
    {
        --depth;
    }

private:
    int& depth;
};


class UndefinedVariable
{
};

class Redeclaration
{
};


#endif //INTERPRETER_STACKS_H
