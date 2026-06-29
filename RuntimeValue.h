#ifndef INTERPRETER_RUNTIMEVALUE_H
#define INTERPRETER_RUNTIMEVALUE_H

#include<string>
#include <vector>
#include <functional>
#include <map>
#include <memory>
#include <optional>
#include "RuntimeError.h"
#include <variant>


using std::string;
using std::vector;
using std::shared_ptr;

class RuntimeValue;
class BlockNode;
struct Environment;
struct RuntimeValueHash;
class ExpressionNode;
class TypeNode;

class Module
{
public:
    Module(shared_ptr<Environment> env, string name) : env{std::move(env)}, name{name}
    {
    }

    RuntimeValue& getMember(const std::string& memberName);
    const RuntimeValue& getMember(const std::string& memberName) const;

    string getName() const { return name; }

private:
    shared_ptr<Environment> env;
    string name;
};

struct Callable
{
    string f_name;
    int callLine;
    virtual RuntimeValue call(const vector<RuntimeValue>& arguments, int line) const =0;
    virtual ~Callable() = default;
};

struct Parameter
{
    string name;
    ExpressionNode* defaultVal;
    TypeNode* type;
    bool isVariadic;
};

struct FunctionObject : public Callable
{
    vector<Parameter> parameters;
    const BlockNode* body;
    shared_ptr<Environment> closure;

    FunctionObject(string f_name, vector<Parameter> parameters, const BlockNode* body,
                   shared_ptr<Environment> closure, int line) : parameters{std::move(parameters)}, body{body},
                                                                closure{std::move(closure)},
                                                                f_name{f_name},
                                                                callLine{line}
    {
    }


    RuntimeValue call(const vector<RuntimeValue>& arguments, int line) const override;

protected:
    string f_name;
    int callLine;
};


class MaxRecursion
{
};

struct NativeFunction : public Callable
{
    std::function<RuntimeValue(const vector<RuntimeValue>& args)> fn;

    NativeFunction(std::function<RuntimeValue(const vector<RuntimeValue>& args)> function,
                   string f_name) : fn{function}, f_name{f_name}
    {
    }

    RuntimeValue call(const vector<RuntimeValue>& arguments, int line) const override;

protected:
    string f_name;
    int callLine;
};

void validateArity(string identifier, int expectedArgs, int actualArgs, bool variadic = false);


void printRuntimeValue(const RuntimeValue& val);

struct Uninitialized
{
};

struct RuntimeValueHash
{
    std::size_t operator()(const RuntimeValue& value) const;
};

using Map = std::unordered_map<RuntimeValue, RuntimeValue, RuntimeValueHash>;

struct Type
{
    virtual ~Type() = default;
};

struct StructType : public Type
{
    string name;
    vector<string> fieldNames;
    std::map<string, std::shared_ptr<FunctionObject>> methods;

    StructType(string name, vector<string> fieldNames,
               std::map<string, std::shared_ptr<FunctionObject>> methods) : name{name}, fieldNames{fieldNames},
                                                                            methods{methods}
    {
    }
};

struct StructInstance
{
    StructType* type;
    std::map<string, RuntimeValue> fields;
    bool hasMethod(string mName);
    shared_ptr<FunctionObject> getMethod(string mName);
    bool hasMemberField(string mName);
    RuntimeValue getMemberVal(string mName);
};

struct Variant
{
    string name;
    vector<string> fields;
};

struct EnumType : public Type
{
    string typeName;
    vector<Variant> variants;

    EnumType(string typeName, vector<Variant> variants) : typeName{typeName}, variants{variants}
    {
    };
};

struct EnumValue
{
    EnumType* type;
    int variantIndex;
    vector<RuntimeValue> fields;
};

struct TypeReference
{
    Type* type;
};

struct EnumVariantReference
{
    EnumType* type;
    std::size_t variantIndex;
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
        Callable,
        Uninitialized,
        Module,
        Map,
        StructObj,
        EnumObj,
        TypeReference,
        EnumVariantReference,
    };

    using Array = vector<RuntimeValue>;
    using Type = std::variant<double, bool, string, std::monostate, shared_ptr<Array>, shared_ptr<Callable>, shared_ptr<
                                  Module>, shared_ptr<Map>, Uninitialized, shared_ptr<StructInstance>, shared_ptr<
                                  EnumValue>, TypeReference, EnumVariantReference>;

    RuntimeValue(Type d) : data{d}
    {
    };

    RuntimeValue(Uninitialized) : data{Uninitialized()}
    {
    }

    RuntimeValue(shared_ptr<Callable> fnPtr) : data{fnPtr}
    {
    };

    RuntimeValue(char c) : data{string(1, c)}
    {
    };

    RuntimeValue(double d) : data{d}
    {
    }

    RuntimeValue(string str) : data{str}
    {
    }

    RuntimeValue(int i) : data{static_cast<double>(i)}
    {
    }

    RuntimeValue(bool b) : data{b}
    {
    }

    RuntimeValue(shared_ptr<Map> mapPtr) : data{mapPtr}
    {
    }

    RuntimeValue(shared_ptr<Array> arrayPtr) : data{arrayPtr}
    {
    }

    RuntimeValue(shared_ptr<Module> modulePtr) : data{modulePtr}
    {
    }

    RuntimeValue(shared_ptr<StructInstance> structPtr) : data{structPtr}
    {
    }

    RuntimeValue(shared_ptr<EnumValue> enumPtr) : data{enumPtr}
    {
    }

    RuntimeValue() : data{std::monostate{}}
    {
    }

    RuntimeValue(TypeReference type) : data{type}
    {
    }

    RuntimeValue(EnumVariantReference variantType) : data{variantType}
    {
    }

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

    [[nodiscard]] bool isCallable() const
    {
        return std::holds_alternative<shared_ptr<Callable>>(data);
    }

    [[nodiscard]] bool isModule() const
    {
        return std::holds_alternative<shared_ptr<Module>>(data);
    }

    [[nodiscard]] bool isUninitialized() const
    {
        return std::holds_alternative<Uninitialized>(data);
    }

    [[nodiscard]] bool isMap() const
    {
        return
            std::holds_alternative<shared_ptr<Map>>(data);
    }

    [[nodiscard]] bool isStructObj() const
    {
        return std::holds_alternative<shared_ptr<StructInstance>>(data);
    };

    [[nodiscard]] bool isEnumObj() const
    {
        return std::holds_alternative<shared_ptr<EnumValue>>(data);
    }

    [[nodiscard]] bool isTypeReference() const
    {
        return std::holds_alternative<TypeReference>(data);
    }

    [[nodiscard]] bool isEnumVariantReference() const
    {
        return std::holds_alternative<EnumVariantReference>(data);
    }

    [[nodiscard]] bool isTruthy() const;

    [[nodiscard]] double asNumber() const
    {
        if (isNumber())
            return std::get<double>(data);
        if (isBoolean())
            return static_cast<double>(std::get<bool>(data));
    };

    [[nodiscard]] double& getNumberRef()
    {
        return std::get<double>(data);
    };
    [[nodiscard]] const string& asString() const { return std::get<string>(data); }

    [[nodiscard]] std::monostate asNull() const { return std::get<std::monostate>(data); }
    string& getStringRef() { return std::get<string>(data); }
    [[nodiscard]] bool asBoolean() const { return std::get<bool>(data); }

    [[nodiscard]] shared_ptr<EnumValue> asEnumPtr() const
    {
        return std::get<shared_ptr<EnumValue>>(data);
    }

    [[nodiscard]] bool& getBoolRef()
    {
        return std::get<bool>(data);
    };
    [[nodiscard]] shared_ptr<Array> asArrayPtr() const { return std::get<shared_ptr<Array>>(data); }
    [[nodiscard]] shared_ptr<Array>& getArrayPtrRef() { return std::get<shared_ptr<Array>>(data); }
    [[nodiscard]] shared_ptr<Callable> asCallableObj() const { return std::get<shared_ptr<Callable>>(data); }

    [[nodiscard]] shared_ptr<Module> asModulePtr() const { return std::get<shared_ptr<Module>>(data); }
    [[nodiscard]] shared_ptr<Module>& getModuleRef() { return std::get<shared_ptr<Module>>(data); }

    [[nodiscard]] shared_ptr<Map> asMapPtr() const { return std::get<shared_ptr<Map>>(data); }
    [[nodiscard]] shared_ptr<Map>& getMapRef() { return std::get<shared_ptr<Map>>(data); }

    [[nodiscard]] shared_ptr<StructInstance> asStructObjPtr() const
    {
        return std::get<shared_ptr<StructInstance>>(data);
    }

    [[nodiscard]] TypeReference asTypeRef() const
    {
        return std::get<TypeReference>(data);
    }

    [[nodiscard]] EnumVariantReference asEnumVariantRef() const
    {
        return std::get<EnumVariantReference>(data);
    }

    [[nodiscard]] string description() const;
    [[nodiscard]] Kind kind() const;

private:
    Type data;
};


struct BoundMethod : public Callable
{
public:
    RuntimeValue call(const vector<RuntimeValue>& arguments, int line) const override;
    shared_ptr<Callable> function;
    RuntimeValue self;

    BoundMethod(shared_ptr<Callable> function, RuntimeValue self) : function{function}, self{self}
    {
    }

protected:
    string f_name;
    int callLine;
};

bool operator==(const RuntimeValue& lhs,
                const RuntimeValue& rhs);


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

namespace Operator
{
    RuntimeValue add(const RuntimeValue& a, const RuntimeValue& b);
    RuntimeValue sub(const RuntimeValue& a, const RuntimeValue& b);
    RuntimeValue multiply(const RuntimeValue& a, const RuntimeValue& b);
    RuntimeValue divide(const RuntimeValue& a, const RuntimeValue& b);
    RuntimeValue modulo(const RuntimeValue& a, const RuntimeValue& b);

    RuntimeValue equal(const RuntimeValue& a, const RuntimeValue& b);
    RuntimeValue notEqual(const RuntimeValue& a, const RuntimeValue& b);
    RuntimeValue greater(const RuntimeValue& a, const RuntimeValue& b);
    RuntimeValue greaterEqual(const RuntimeValue& a, const RuntimeValue& b);
    RuntimeValue less(const RuntimeValue& a, const RuntimeValue& b);
    RuntimeValue lessEqual(const RuntimeValue& a, const RuntimeValue& b);

    RuntimeValue logicalAnd(const RuntimeValue& a, const RuntimeValue& b);
}


#endif //INTERPRETER_RUNTIMEVALUE_H
