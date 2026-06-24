//
// Created by wcosmo on 6/2/26.
//

#include "stdlib.h"
#include "ast.h"
#include "secrets.h"
#include "RuntimeError.h"


static const auto startTime = std::chrono::steady_clock::now();

void registerNativeFunctions(std::function<RuntimeValue(const vector<RuntimeValue>& args)> fn, string f_name,
                             Environment& env)
{
    auto function = std::static_pointer_cast<Callable>(std::make_shared<NativeFunction>(fn, f_name));
    env.declare(f_name, VariableInfo(function, 0));
}

void registerStdLib(Environment& env)
{
    registerNativeFunctions([](
                            const std::vector<RuntimeValue>& args)
                            {
                                validateArity("print", 0, args.size(), true);
                                for (auto& arg : args)
                                {
                                    printRuntimeValue(arg);
                                    std::cout << " ";
                                }
                                return RuntimeValue();
                            }, "print", env);
    registerNativeFunctions([](
                            const std::vector<RuntimeValue>& args)
                            {
                                validateArity("println", 0, args.size(), true);
                                for (auto& arg : args)
                                {
                                    printRuntimeValue(arg);
                                    std::cout << " ";
                                }
                                std::cout << "\n";
                                return RuntimeValue();
                            }, "println", env);
    registerNativeFunctions([](const std::vector<RuntimeValue>& args)
    {
        validateArity("len", 1, args.size(), false);
        if (args[0].isArray())
            return static_cast<double>(args[0].asArrayPtr()->size());
        if (args[0].isString())
            return static_cast<double>(args[0].asString().size());
        throw RuntimeError("type error", {});
    }, "len", env);
    registerNativeFunctions([](const std::vector<RuntimeValue>& args)
    {
        validateArity("type", 1, args.size(), false);
        return args[0].description();
    }, "type", env);
    registerNativeFunctions([](const std::vector<RuntimeValue>& args)
    {
        validateArity("read", 1, args.size(), true);

        for (int i = 0; i < args.size(); ++i)
        {
            printRuntimeValue(args[i]);
            if (i < args.size() - 1)
                std::cout << " ";
        }
        string input;
        getline(std::cin, input, ' ');
        return input;
    }, "read", env);
    registerNativeFunctions([](const std::vector<RuntimeValue>& args)
    {
        validateArity("readln", 1, args.size(), true);
        for (int i = 0; i < args.size(); ++i)
        {
            printRuntimeValue(args[i]);
            if (i < args.size() - 1)
                std::cout << " ";
        }
        string input;
        getline(std::cin, input, '\n');
        return input;
    }, "readln", env);
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
        validateArity("pop", 2, args.size(), false);
        if (args[0].isArray())
        {
            const RuntimeValue poppedVal = args[0].asArrayPtr()->at(
                args[0].asArrayPtr()->size() - 1);
            args[0].asArrayPtr()->pop_back();
            return poppedVal;
        }
        throw std::runtime_error("invalid operand for pop");
    }, "pop", env);
    registerNativeFunctions([](const std::vector<RuntimeValue>& args)
    {
        validateArity("number", 1, args.size(), false);
        if (args[0].isString())
            return std::stod(args[0].asString());
        throw std::runtime_error("invalid operand for number");
    }, "number", env);
    registerNativeFunctions([](const std::vector<RuntimeValue>& args)
    {
        validateArity("str", 1, args.size(), false);
        if (args[0].isNumber())
            return std::to_string(args[0].asNumber());
        throw std::runtime_error("invalid operand for number");
    }, "str", env);
    registerNativeFunctions([](const std::vector<RuntimeValue>& args)
    {
        validateArity("random", 2, args.size(), false);
        if (args[0].isNumber() && args[1].isNumber())
            return Random::get(static_cast<int>(args[0].asNumber()), static_cast<int>(args[1].asNumber()));
        throw std::runtime_error("invalid operand for random");
    }, "random", env);
    registerNativeFunctions([](const std::vector<RuntimeValue>& args)
    {
        validateArity("len", 0, args.size(), false);
        return std::chrono::duration<double>(std::chrono::steady_clock::now() - startTime).count();
    }, "clock", env);
    registerNativeFunctions([](const std::vector<RuntimeValue>& args)-> RuntimeValue
    {
        throw Exit();
    }, "exit", env);
    registerNativeFunctions([](const std::vector<RuntimeValue>& args)-> RuntimeValue
    {
        throw Help();
    }, "help", env);
}

unique_ptr<ProgramNode> loadStdlib(const string& lib, InterpreterContext& ctx)
{
    if (!(lib == "string" || lib == "array" || lib == "math"))
        throw UndefinedVariable();
    const string filePath = std::format("{}/{}.som", PATH_TO_STDLIB, lib);
    std::ifstream is(filePath);
    ctx.currentFile = filePath;
    if (!is.is_open())
    {
        throw std::runtime_error("Failed to open file");
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
