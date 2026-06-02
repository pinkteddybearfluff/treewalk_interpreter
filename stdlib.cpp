//
// Created by wcosmo on 6/2/26.
//

#include "stdlib.h"

#include "ast.h"


void registerStdLib(Environment& env)
{
    env.declare(
        "print"
        ,
        {
            RuntimeValue(NativeFunction("print",
                                        [](
                                        const std::vector<RuntimeValue>& args)
                                        {
                                            for (auto& arg : args)
                                            {
                                                printRuntimeValue(arg);
                                                std::cout << " ";
                                            }
                                            return RuntimeValue();
                                        }
            ))
        }
    );
    env.declare(
        "println"
        ,
        {
            NativeFunction("println",
                           [](
                           const std::vector<RuntimeValue>& args)
                           {
                               for (auto& arg : args)
                               {
                                   printRuntimeValue(arg);
                                   std::cout << " ";
                               }
                               std::cout << "\n";
                               return RuntimeValue();
                           }
            )
        }
    );
    env.declare("size", {
                    NativeFunction{
                        "size", [](const std::vector<RuntimeValue>& args)
                        {
                            if (args[0].isArray())
                                return static_cast<double>(args[0].asArrayPtr()->size());
                            if (args[0].isString())
                                return static_cast<double>(args[0].asString().size());
                            throw RuntimeError("type error", {});
                        }
                    }
                });
    env.declare("type", {
                    NativeFunction{
                        "type", [](const std::vector<RuntimeValue>& args)
                        {
                            return args[0].description();
                        }
                    }
                });
    env.declare("read", {
                    NativeFunction{
                        "read", [](const std::vector<RuntimeValue>& args)
                        {
                            for (auto arg : args)
                            {
                                printRuntimeValue(arg);
                                std::cout << " ";
                            }
                            string input;
                            getline(std::cin, input, ' ');
                            return input;
                        }
                    }
                });
    env.declare("readln", {
                    NativeFunction{
                        "readln", [](const std::vector<RuntimeValue>& args)
                        {
                            for (auto arg : args)
                            {
                                printRuntimeValue(arg);
                                std::cout << " ";
                            }
                            string input;
                            getline(std::cin, input, '\n');
                            return input;
                        }
                    }
                });
    env.declare("push", {
                    NativeFunction{
                        "push", [](const std::vector<RuntimeValue>& args)
                        {
                            if (args[0].isArray())
                            {
                                args[0].asArrayPtr()->push_back(args[1]);
                                return RuntimeValue();
                            }
                            throw std::runtime_error("invalid operand for push");
                        }
                    }
                });
}

unique_ptr<ProgramNode> loadStdlib(const string& file_name, shared_ptr<Environment> env)
{
    std::ifstream is(std::format("../stdlib/{}", file_name));
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
    program->evaluateNode(env);
    return program;
}
