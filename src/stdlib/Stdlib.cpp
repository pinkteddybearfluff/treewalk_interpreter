//
// Created by wcosmo on 6/2/26.
//

#include "Stdlib.h"
#include "../ast/Ast.h"
#include "../error/RuntimeError.h"
#include "../utilities/Utilities.h"

static const auto startTime = std::chrono::steady_clock::now();

void registerNativeFunctions(std::function<RuntimeValue(const vector<RuntimeValue>& args)> fn, string f_name,
                             Environment& env)
{
    auto function = std::static_pointer_cast<Callable>(std::make_shared<NativeFunction>(fn, f_name));
    env.declare(f_name, VariableInfo(function, 0));
}

void registerStdLib(Environment& env)
{
    registerNativeFunctions([]
                        (const std::vector<RuntimeValue>& args)
                            {
                                validateArity("stringify", 1, args.size(), false);

                                return stdlib::stringify(args[0]);
                            }, "stringify", env);
    registerNativeFunctions([]
                        (const std::vector<RuntimeValue>& args)
                            {
                                validateArity("print", 0, args.size(), true);
                                bool first{true};
                                for (auto& arg : args)
                                {
                                    if (!first)
                                    {
                                        std::cerr << " ";
                                    }
                                    printRuntimeValue(cout, arg);

                                    first = false;
                                }
                                return RuntimeValue();
                            }, "print", env);
    registerNativeFunctions([]
                        (const std::vector<RuntimeValue>& args)
                            {
                                validateArity("println", 0, args.size(), true);
                                bool first{true};
                                for (auto& arg : args)
                                {
                                    if (!first)
                                    {
                                        std::cerr << " ";
                                    }
                                    printRuntimeValue(cout, arg);
                                    first = false;
                                }
                                std::cout << "\n";
                                return RuntimeValue();
                            }, "println", env);
    registerNativeFunctions([]
                        (const std::vector<RuntimeValue>& args)
                            {
                                validateArity("eprint", 0, args.size(), true);
                                std::cerr << color::red;
                                bool first{true};
                                for (auto& arg : args)
                                {
                                    if (!first)
                                    {
                                        std::cerr << " ";
                                    }
                                    printRuntimeValue(std::cerr, arg);
                                    first = false;
                                }
                                std::cerr << color::reset;
                                return RuntimeValue();
                            }, "eprint", env);
    registerNativeFunctions([]
                        (const std::vector<RuntimeValue>& args)
                            {
                                validateArity("eprintln", 0, args.size(), true);
                                std::cerr << color::red;
                                bool first{true};
                                for (auto& arg : args)
                                {
                                    if (!first)
                                    {
                                        std::cerr << " ";
                                    }
                                    printRuntimeValue(std::cerr, arg);
                                    first = false;
                                }
                                std::cerr << color::reset;
                                std::cerr << "\n";
                                return RuntimeValue();
                            }, "eprintln", env);
    registerNativeFunctions([]
                        (const std::vector<RuntimeValue>& args)
                            {
                                validateArity("len", 1, args.size(), false);
                                if (args[0].isArray())
                                    return static_cast<double>(args[0].asArrayPtr()->size());
                                if (args[0].isString())
                                    return static_cast<double>(args[0].asString().size());
                                if (args[0].isMap())
                                    return static_cast<double>(args[0].asMapPtr()->size());
                                throw std::runtime_error("invalid operand for len");
                            }, "len", env);
    registerNativeFunctions([]
                        (const std::vector<RuntimeValue>& args)
                            {
                                validateArity("type", 1, args.size(), false);
                                return args[0].description();
                            }, "type", env);
    registerNativeFunctions([]
                        (const std::vector<RuntimeValue>& args)
                            {
                                validateArity("read", 1, args.size(), true);

                                for (int i = 0; i < args.size(); ++i)
                                {
                                    printRuntimeValue(cout, args[i]);
                                    if (i < args.size() - 1)
                                        std::cout << " ";
                                }
                                string input;
                                getline(std::cin, input, ' ');
                                return input;
                            }, "read", env);
    registerNativeFunctions([]
                        (const std::vector<RuntimeValue>& args)
                            {
                                validateArity("readln", 1, args.size(), true);
                                for (int i = 0; i < args.size(); ++i)
                                {
                                    printRuntimeValue(cout, args[i]);
                                    if (i < args.size() - 1)
                                        std::cout << " ";
                                }
                                string input;
                                getline(std::cin, input, '\n');
                                return input;
                            }, "readln", env);
    registerNativeFunctions([]
                        (const std::vector<RuntimeValue>& args)
                            {
                                validateArity("number", 1, args.size(), false);
                                if (args[0].isString())
                                    return std::stod(args[0].asString());
                                throw std::runtime_error("invalid operand for number");
                            }, "number", env);
    registerNativeFunctions([]
                        (const std::vector<RuntimeValue>& args)
                            {
                                validateArity("isdigit", 1, args.size(), false);
                                if (args[0].isString())
                                {
                                    for (auto ch : args[0].asString())
                                    {
                                        if (!isdigit(ch))
                                            return false;
                                    }
                                    return true;
                                }
                                throw std::runtime_error("invalid operand for isdigit");
                            }, "isdigit", env);
    registerNativeFunctions([]
                        (const std::vector<RuntimeValue>& args)
                            {
                                validateArity("isalpha", 1, args.size(), false);
                                if (args[0].isString())
                                {
                                    for (auto ch : args[0].asString())
                                    {
                                        if (!isalpha(ch))
                                            return false;
                                    }
                                    return true;
                                }
                                throw std::runtime_error("invalid operand for isalpha");
                            }, "isalpha", env);
    registerNativeFunctions([]
                        (const std::vector<RuntimeValue>& args)
                            {
                                validateArity("isspace", 1, args.size(), false);
                                if (args[0].isString())
                                {
                                    for (auto ch : args[0].asString())
                                    {
                                        if (!isspace(ch))
                                            return false;
                                    }
                                    return true;
                                }
                                throw std::runtime_error("invalid operand for isspace");
                            }, "isspace", env);
    registerNativeFunctions([]
                        (const std::vector<RuntimeValue>& args)
                            {
                                validateArity("isalnum", 1, args.size(), false);
                                if (args[0].isString())
                                {
                                    for (auto ch : args[0].asString())
                                    {
                                        if (!isalnum(ch))
                                            return false;
                                    }
                                    return true;
                                }
                                throw std::runtime_error("invalid operand for isalnum");
                            }, "isalnum", env);
    registerNativeFunctions([]
                        (const std::vector<RuntimeValue>& args)
                            {
                                validateArity("random", 2, args.size(), false);
                                if (args[0].isNumber() && args[1].isNumber())
                                    return Random::get(static_cast<int>(args[0].asNumber()),
                                                       static_cast<int>(args[1].asNumber()));
                                throw std::runtime_error("invalid operand for random");
                            }, "random", env);
    registerNativeFunctions([]
                        (const std::vector<RuntimeValue>& args)
                            {
                                validateArity("clock", 0, args.size(), false);
                                return std::chrono::duration<double>(std::chrono::steady_clock::now() - startTime).
                                    count();
                            }, "clock", env);
    registerNativeFunctions([](const std::vector<RuntimeValue>& args)-> RuntimeValue
    {
        validateArity("assert", 1, args.size(), false);
        if (args[0].isBoolean())
        {
            if (args[0].asBoolean())
                return RuntimeValue();
            throw AssertionError();
        }
        throw std::runtime_error("invalid operand type for assert");
    }, "assert", env);
    registerNativeFunctions([](const std::vector<RuntimeValue>& args) -> RuntimeValue
    {
        validateArity("panic", 1, args.size(), false);
        if (args[0].isString())
            throw PanicError(args[0].asString());
        throw std::runtime_error("invalid operand type for assert");
    }, "panic", env);
    registerNativeFunctions([](const std::vector<RuntimeValue>& args)-> RuntimeValue
    {
        throw Exit();
    }, "exit", env);
    registerNativeFunctions([](const std::vector<RuntimeValue>& args)-> RuntimeValue
    {
        throw Help();
    }, "help", env);
}

void registerStdLibArray(Environment& env)
{
    registerNativeFunctions([](const std::vector<RuntimeValue>& args)
    {
        validateArity("push", 2, args.size(), false);

        if (args[0].isArray())
        {
            args[0].asArrayPtr()->push_back(args[1]);
            return RuntimeValue();
        }
        throw std::runtime_error("invalid operand for push " + args[0].description());
    }, "push", env);
    registerNativeFunctions([](const std::vector<RuntimeValue>& args)
    {
        validateArity("pop", 1, args.size(), false);
        if (args[0].isArray())
        {
            const RuntimeValue poppedVal = args[0].asArrayPtr()->at(
                args[0].asArrayPtr()->size() - 1);
            args[0].asArrayPtr()->pop_back();
            return poppedVal;
        }
        throw std::runtime_error("invalid operand for pop");
    }, "pop", env);
}

void registerStdLibMap(Environment& env)
{
    registerNativeFunctions([](const std::vector<RuntimeValue>& args)
    {
        validateArity("insert", 3, args.size(), false);
        if (args[0].isMap())
        {
            args[0].asMapPtr()->insert(std::make_pair(args[1], args[2]));
            return RuntimeValue();
        }
        throw std::runtime_error("invalid operand for insert");
    }, "insert", env);
    registerNativeFunctions([](const std::vector<RuntimeValue>& args)
    {
        validateArity("remove", 2, args.size(), false);
        if (args[0].isMap())
        {
            args[0].asMapPtr()->erase(args[1]);
            return RuntimeValue();
        }
        throw std::runtime_error("invalid operand for remove");
    }, "remove", env);
    registerNativeFunctions([](const std::vector<RuntimeValue>& args)
    {
        validateArity("clear", 1, args.size(), false);
        if (args[0].isMap())
        {
            args[0].asMapPtr()->clear();
            return RuntimeValue();
        }
        throw std::runtime_error("invalid operand for clear");
    }, "clear", env);
    registerNativeFunctions([](const std::vector<RuntimeValue>& args)
    {
        validateArity("contains?", 2, args.size(), false);
        if (args[0].isMap())
        {
            return args[0].asMapPtr()->contains(args[1]);
        }
        throw std::runtime_error("invalid operand for contains?");
    }, "contains?", env);
    registerNativeFunctions([](const std::vector<RuntimeValue>& args)
    {
        validateArity("isEmpty?", 1, args.size(), false);
        if (args[0].isMap())
        {
            return args[0].asMapPtr()->empty();
        }
        throw std::runtime_error("invalid operand for isEmpty?");
    }, "isEmpty?", env);
    registerNativeFunctions([](const std::vector<RuntimeValue>& args)
    {
        validateArity("size", 1, args.size(), false);
        if (args[0].isMap())
            return static_cast<double>(args[0].asMapPtr()->size());
        throw std::runtime_error("invalid operand for size");
    }, "size", env);
}

void registerStdLibMath(Environment& env)
{
    registerNativeFunctions([](const std::vector<RuntimeValue>& args)-> RuntimeValue
    {
        validateArity("sqrt", 1, args.size(), false);
        if (args[0].isNumber())
            return std::sqrt(args[0].asNumber());
        throw std::runtime_error("invalid operand type for sqrt");
    }, "sqrt", env);
    registerNativeFunctions([](const std::vector<RuntimeValue>& args)
    {
        validateArity("pow", 2, args.size(), false);
        if (args[0].isNumber() && args[1].isNumber())
            return std::pow(args[0].asNumber(), args[1].asNumber());
        throw std::runtime_error("invalid operand type for pow");
    }, "pow", env);
    registerNativeFunctions([](const std::vector<RuntimeValue>& args)
    {
        validateArity("ceil", 1, args.size(), false);
        if (args[0].isNumber())
            return std::ceil(args[0].asNumber());
        throw std::runtime_error("invalid operand type for ceil");
    }, "ceil", env);
    registerNativeFunctions([](const std::vector<RuntimeValue>& args)
    {
        validateArity("floor", 1, args.size(), false);
        if (args[0].isNumber())
            return std::floor(args[0].asNumber());
        throw std::runtime_error("invalid operand type for floor");
    }, "floor", env);
    registerNativeFunctions([](const std::vector<RuntimeValue>& args)
    {
        validateArity("round", 1, args.size(), false);
        if (args[0].isNumber())
            return std::round(args[0].asNumber());
        throw std::runtime_error("invalid operand type for round");
    }, "round", env);
    registerNativeFunctions([](const std::vector<RuntimeValue>& args)
    {
        validateArity("trunc", 1, args.size(), false);
        if (args[0].isNumber())
            return std::trunc(args[0].asNumber());
        throw std::runtime_error("invalid operand type for trunc");
    }, "trunc", env);
    registerNativeFunctions([](const std::vector<RuntimeValue>& args)
    {
        validateArity("sin", 1, args.size(), false);
        if (args[0].isNumber())
            return std::sin(args[0].asNumber());
        throw std::runtime_error("invalid operand type for sin");
    }, "sin", env);
    registerNativeFunctions([](const std::vector<RuntimeValue>& args)
    {
        validateArity("cos", 1, args.size(), false);
        if (args[0].isNumber())
            return std::cos(args[0].asNumber());
        throw std::runtime_error("invalid operand type for cos");
    }, "cos", env);
    registerNativeFunctions([](const std::vector<RuntimeValue>& args)
    {
        validateArity("tan", 1, args.size(), false);
        if (args[0].isNumber())
            return std::tan(args[0].asNumber());
        throw std::runtime_error("invalid operand type for tan");
    }, "tan", env);
    registerNativeFunctions([](const std::vector<RuntimeValue>& args)
    {
        validateArity("asin", 1, args.size(), false);
        if (args[0].isNumber())
            return std::asin(args[0].asNumber());
        throw std::runtime_error("invalid operand type for asin");
    }, "asin", env);
    registerNativeFunctions([](const std::vector<RuntimeValue>& args)
    {
        validateArity("acos", 1, args.size(), false);
        if (args[0].isNumber())
            return std::acos(args[0].asNumber());
        throw std::runtime_error("invalid operand type for acos");
    }, "acos", env);
    registerNativeFunctions([](const std::vector<RuntimeValue>& args)
    {
        validateArity("atan", 1, args.size(), false);
        if (args[0].isNumber())
            return std::atan(args[0].asNumber());
        throw std::runtime_error("invalid operand type for atan");
    }, "atan", env);
    registerNativeFunctions([](const std::vector<RuntimeValue>& args)
    {
        validateArity("log", 1, args.size(), false);
        if (args[0].isNumber())
            return std::log(args[0].asNumber());
        throw std::runtime_error("invalid operand type for log");
    }, "log", env);
    registerNativeFunctions([](const std::vector<RuntimeValue>& args)
    {
        validateArity("exp", 1, args.size(), false);
        if (args[0].isNumber())
            return std::exp(args[0].asNumber());
        throw std::runtime_error("invalid operand type for exp");
    }, "exp", env);
}

unique_ptr<ProgramNode> loadStdlib(const string& lib, InterpreterContext& ctx)
{
    if (!(lib == "string" || lib == "array" || lib == "math" || lib == "map" || lib == "core"))
        throw UndefinedVariable();
    if (lib == "array")
    {
        registerStdLibArray(*ctx.env);
    }
    if (lib == "math")
    {
        registerStdLibMath(*ctx.env);
    }
    if (lib == "map")
    {
        registerStdLibMap(*ctx.env);
        return nullptr;
    }
    const string filePath = std::format("/home/wcosmo/Desktop/Projects/interpreter/stdlib/{}.som", lib);
    std::ifstream is(filePath);
    ctx.currentFile = filePath;
    if (!is.is_open())
    {
        throw std::runtime_error("Failed to open file " + filePath);
    }

    vector<unique_ptr<StatementNode>> nodes;
    TokenStream ts{is};
    while (!check(TokenType::End, ts))
    {
        nodes.push_back(parseStatement(ts));
    }
    unique_ptr<ProgramNode> program = make_unique<ProgramNode>(std::move(nodes));
    program->evaluateNode(ctx);
    return program;
}

void importBuiltinStdlib(const string& file_name, string alias, InterpreterContext& ctx)
{
    string stdLibIdentifier = std::format("std.{}", file_name);
    auto itr = ctx.module->loadedModules.find(stdLibIdentifier);
    if (itr == ctx.module->loadedModules.end())
    {
        auto moduleCtx = InterpreterContext(std::make_shared<Environment>(), ctx.builtin, ctx.module);
        auto program = loadStdlib(file_name, moduleCtx);
        ctx.module->loadedModules.insert({stdLibIdentifier, ModuleCtx{std::move(program), moduleCtx.env}});
        moduleCtx.env->parent = ctx.builtin;
        moduleCtx.workingDir = ctx.workingDir;
        ctx.builtin->declare(alias, {
                                 std::make_shared<Module>(moduleCtx.env, alias), -1
                             });
    }
}

void importStdlibModule(const string& file_name, string alias, InterpreterContext& ctx)
{
    string stdLibIdentifier = std::format("std.{}", file_name);
    auto itr = ctx.module->loadedModules.find(stdLibIdentifier);
    if (itr == ctx.module->loadedModules.end())
    {
        auto moduleCtx = InterpreterContext(std::make_shared<Environment>(), ctx.builtin, ctx.module);
        auto program = loadStdlib(file_name, moduleCtx);
        ctx.module->loadedModules.insert({stdLibIdentifier, ModuleCtx{std::move(program), moduleCtx.env}});
        moduleCtx.env->parent = ctx.builtin;
        moduleCtx.workingDir = ctx.workingDir;
        ctx.env->declare(alias, {
                             std::make_shared<Module>(moduleCtx.env, alias), -1
                         });
        return;
    }

    ctx.env->declare(alias, {std::make_shared<Module>(itr->second.env, alias), 0});
}

string stdlib::stringify(const RuntimeValue& value)
{
    string str{};
    if (value.isString())
    {
        str.append(value.asString());
        return str;
    }
    if (value.isNumber())
    {
        str.append(stdlib::numberToString(value.asNumber()));
        return str;
    }
    if (value.isBoolean())
    {
        if (value.asBoolean())
            str.append("true");
        else
            str.append("false");
        return str;
    }
    if (value.isArray())
    {
        if (value.asArrayPtr()->empty())
        {
            str.append("[]");
            return str;
        }
        bool first{true};
        str.append("[");
        for (auto val : *value.asArrayPtr())
        {
            if (!first)
            {
                str.append(", ");
            }
            str.append(stringify(val));
            first = false;
        }
        str.append("]");
        return str;
    }
    if (value.isMap())
    {
        if (value.asMapPtr()->empty())
        {
            str.append("{}");
            return str;
        }
        bool first{true};
        str.append("{");
        for (auto [key,val] : *value.asMapPtr())
        {
            if (!first)
            {
                str.append(", ");
            }
            str.append(std::format("{}: {}", stringify(key), stringify(val)));
        }
        str.append("}");
    }
    if (value.isNull())
    {
        str.append("null");
        return str;
    }
    if (value.isStructObj())
    {
        if (value.asStructObjPtr()->hasMethod("stringify"))
        {
            str.append(stringify(value.asStructObjPtr()->getMethod("stringify")->call({value}, 0)));
            return str;
        }
        str.append("{");
        bool first{true};
        for (const auto& [key, val] : value.asStructObjPtr()->fields)
        {
            if (!first)
            {
                str.append(", ");
            }
            str.append(std::format("{}: {}", key, stringify(val)));
            first = false;
        }
        str.append("}");
        return str;
    }
    if (value.isEnumObj())
    {
        str.append(value.asEnumPtr()->type->typeName);
        str.append(".");
        str.append(value.asEnumPtr()->type->variants[value.asEnumPtr()->variantIndex].name);
        if (!value.asEnumPtr()->fields.empty())
        {
            str.append("(");
            bool first{true};
            for (auto field : value.asEnumPtr()->fields)
            {
                if (!first)
                {
                    str.append(", ");
                }
                str.append(stringify(field));
                first = false;
            }
            str.append(")");
        }
        return str;
    }
    if (value.isTypeReference())
    {
        if (auto* structType = dynamic_cast<StructType*>(value.asTypeRef().type))
            str.append(structType->name);
        else if (auto* enumType = dynamic_cast<EnumType*>(value.asTypeRef().type))
            str.append((enumType->typeName));
        return str;
    }
    if (value.isEnumVariantReference())
    {
        str.append(value.asEnumVariantRef().type->typeName);
        str.append(".");
        str.append(value.asEnumVariantRef().type->variants[value.asEnumVariantRef().variantIndex].name);
        return str;
    }
    if (value.isCallable())
    {
        str.append(value.asCallableObj()->f_name);
    }
    if (value.isModule())
    {
        str.append(value.asModulePtr()->getName());
    }
    throw std::runtime_error("value cannot be converted to string " + value.description());
}

string stdlib::numberToString(double number)
{
    std::ostringstream out;
    out << number;
    return out.str();
}
