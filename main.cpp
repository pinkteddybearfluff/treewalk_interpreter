#include <iostream>
#include <sstream>
#include <fstream>
#include <string_view>
#include <chrono>
#include <format>
#include <ranges>
#include <sstream>
#include "parser.h"
#include "ast.h"
#include "lexer.h"
#include "environment.h"
#include "version.h"
#include "stdlib.h"
#include "RuntimeValue.h"
#include "RuntimeError.h"
#include "utilities.h"
#include "secrets.h"

// The Laven Language Lavender

//TODO: 1) add better error handling with ParserError LexerError Runtime Evaluation Error
//Todo: 2) add centralized binary operations handled by RuntimeValue.
//Todo: 3) Improve functionCallNode;
//Todo: 4) Implement Call abstractions
//Todo: 5) Have Lvalue for VariableNode and IndexNode
//Todo: 7) automate testing with python
//TODO: 8) add syntax highlighting


auto main(int argc, char** argv) -> int
{
    auto start = std::chrono::high_resolution_clock::now();
    auto parseCompleteFlag = std::chrono::high_resolution_clock::now();
    auto evalCompleteFlag = std::chrono::high_resolution_clock::now();

    //shared_ptr so to make Environment a linked structure
    InterpreterContext ctx{std::make_shared<Environment>(), std::make_shared<ModuleManager>()};

    shared_ptr<Environment> env = ctx.env;
    shared_ptr<ModuleManager> modules = ctx.module;

    env->declare("__VERSION__", {string(LANGUAGE_VERSION), 0});
    env->parent = nullptr;
    registerStdLib(*env);
    vector<unique_ptr<ProgramNode>> loadedLibraries;
    loadedLibraries.push_back(loadStdlib("array", ctx));

    bool REPL{true};

    if (argv[1])
    {
        std::string_view cmd = argv[1];
        if (cmd == "--version" || cmd == "-v")
        {
            cout << "Laven " << LANGUAGE_VERSION << "\n";
            return 0;
        }
        if (cmd == "fmt" || cmd == "format")
        {
            if (argv[2])
            {
                string file = argv[2];
                std::ifstream is(file);
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

                std::ofstream formattedFile(file);
                FormatContext fmtCtx{0, formattedFile};

                program->format(fmtCtx);
                return 0;
            }
        }
        string file = argv[1];
        ctx.currentFile = file;
        ctx.workingDir = getFolder(file);
        ctx.module->loadedModules.insert({file, nullptr});
        REPL = false;
        std::ifstream is(file);

        if (!is.is_open())
        {
            throw std::runtime_error("Failed to open file");
        }
        try
        {
            int result{0};
            vector<unique_ptr<StatementNode>> nodes;

            TokenStream ts{is};
            while (!check(TokenType::End, ts))
            {
                nodes.push_back(parseStatement(ts));
            }

            parseCompleteFlag = std::chrono::high_resolution_clock::now();
            // cout << std::format("<--------Parsing completed-------------> [Time:{}]",
            //                     std::chrono::duration<double>(parseCompleteFlag - start).count()) << '\n';
            unique_ptr<ProgramNode> program = make_unique<ProgramNode>(std::move(nodes));
            evalCompleteFlag = std::chrono::high_resolution_clock::now();
            // cout << std::format("<----------Evaluation complete---------> [Time:{}]",
            //                     std::chrono::duration<double>(evalCompleteFlag - start).count()) << '\n';
            if constexpr (DEBUG_AST)
                program->debugPrint(0);


            program->evaluateNode(ctx);
            // FormatContext fmtCtx;
            //
            // program->format(fmtCtx);

            // if constexpr (DEBUG_ENV);
            // env.debugEnvPrint();
            auto end = std::chrono::high_resolution_clock::now();

            // std::cout
            //     << std::format("[Total time taken: {}]",
            //                    std::chrono::duration<double>(end - start).count()) << '\n';
            //
            // std::cout
            //     << std::format("[Time taken for parsing: {}]",
            //                    std::chrono::duration<double>(parseCompleteFlag - start).count()) << '\n';
            // std::cout
            //     << std::format("Time taken for evaluation: {}]",
            //                    std::chrono::duration<double>(evalCompleteFlag - parseCompleteFlag).count()) << '\n';

            return 0;
        }
        catch (const LexerError& le)
        {
            std::cerr << "File " << color::magenta << "\"" << ctx.currentFile << "\"" << color::reset << ", line " <<
                color::boldBlue
                << le
                .line <<
                color::reset << '\n';
            std::cerr << color::boldRed << "SyntaxError: " << color::reset << le.what() << "\n";
            return 1;
        }
        catch (const ParserError& pe)
        {
            std::cerr << "File " << color::magenta << "\"" << ctx.currentFile << "\"" << color::reset << ", line " <<
                color::boldBlue
                << pe
                .line <<
                color::reset << '\n';
            std::cerr << color::boldRed << "SyntaxError: " << color::reset << pe.what() << "\n";
            return 2;
        }
        catch (const RuntimeError& re)
        {
            printRuntimeError(re, ctx.currentFile);
            return 3;
        }
        catch (const std::exception& e)
        {
            std::cerr << "File " << color::magenta << "\"" << ctx.currentFile << "\"" << color::reset << ", line " <<
                color::boldBlue
                << 0 <<
                color::reset << '\n';
            std::cerr << "Line " << 0 << " error: " << e.what() << "\n";

            if constexpr (DEBUG_ENV);
            // env.debugEnvPrint();
            return 4;
        }
    }
    else
    {
        vector<unique_ptr<StatementNode>> nodes;

        cout << "Welcome to Laven v" << LANGUAGE_VERSION << "\n";
        while (true)
        {
            cout << color::magenta << ">>> " << color::reset;
            try
            {
                if (string input = readREPLinput(); !input.empty())
                {
                    std::istringstream sis{input};

                    TokenStream ts{sis};
                    nodes.push_back(parseStatement(ts));
                    if (auto [hasValue, value] = nodes.back()->evaluateNode(ctx); hasValue)
                    {
                        if (value.isNull())
                            cout << color::black;
                        else if (value.isCallable())
                            cout << color::blue;
                        else cout << color::yellow;
                        if (value.isString())
                        {
                            cout << "'";
                            printRuntimeValue(value);
                            cout << "'";
                        }
                        else printRuntimeValue(value);
                        cout << "\n";
                        cout << color::reset;
                    }
                }
            }
            catch (const RuntimeError& re)
            {
                printRuntimeError(re, "repl");
            }
            catch (const LexerError& le)
            {
                std::cerr << color::boldRed << "SyntaxError: " << color::reset << le.what() << "\n";
            }
            catch (const ParserError& pe)
            {
                std::cerr << color::boldRed << pe.getCatStr() << ": " << color::reset << pe.what() << "\n";
            }
            catch (Help)
            {
                cout << "Help:\n";
            }
            catch (Exit)
            {
                return 0;
            }
            catch (const std::exception& e)
            {
                std::cerr << "Error: " << e.what() << "\n";
            }
        }
    }
}
