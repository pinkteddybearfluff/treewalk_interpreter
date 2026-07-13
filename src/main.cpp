#include <iostream>
#include <sstream>
#include <fstream>
#include <string_view>
#include <chrono>
#include <format>
#include <ranges>
#include <sstream>
#include "parser/Parser.h"
#include "ast/Ast.h"
#include "lexer/Lexer.h"
#include "runtime/Environment.h"
#include "Version.h"
#include "stdlib/Stdlib.h"
#include "runtime/RuntimeValue.h"
#include "error/RuntimeError.h"
#include "utilities/Utilities.h"
#include "visitors/EvaluateVisitor.h"
#include "visitors/FormatVisitor.h"

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
    InterpreterContext ctx{
        std::make_shared<Environment>(), std::make_shared<Environment>(), std::make_shared<ModuleManager>()
    };

    shared_ptr<Environment> env = ctx.env;
    shared_ptr<ModuleManager> modules = ctx.module;

    shared_ptr<Environment> builtinEnv = ctx.builtin;
    builtinEnv->declare("__VERSION__", {string(LANGUAGE_VERSION), 0});
    builtinEnv->parent = nullptr;
    registerStdLib(*builtinEnv);
    ctx.env->parent = builtinEnv;
    // modules->loadedModules.insert(std::make_pair("coreSTDLIB", loadStdlib("core", ctx)));
    // importBuiltinStdlib("array", "ARRAYSTDLIB", ctx);
    // importBuiltinStdlib("map", "MAPSTDLIB", ctx);
    // importBuiltinStdlib("string", "STRINGSTDLIB", ctx);

    /*
     *  Environment hierarchy:
     *      Builtin Environment:
     *       |-->   Hosts: core native functions directly defined in the builtin env
     *       |-->   Hosts: imports STDLIB functions as module in the module environment.
     *                  Array, Map, and String stored as module, this module stored inside Builtin
     *
     *       |-->   Hosts: the main environment.
     *              Main Environment:
     *              |-->   Hosts: imports other STDLIB functions as module in the module environment
     *                          Math, and other ordinary stdlib functions.
     *              |-->   Hosts: imports other user defined modules as well in the module environment.
     */
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

                TokenStream ts{is};
                auto program = parseProgram(ts);

                std::ofstream formattedFile(file);
                FormatContext fmtCtx{0, formattedFile};

                return 0;
            }
        }
        string file = argv[1];
        ctx.currentFile = file;
        ctx.workingDir = getFolder(file);
        ctx.module->loadedModules.insert({file, ModuleCtx(nullptr, ctx.env)});
        REPL = false;
        std::ifstream is(file);

        if (!is.is_open())
        {
            throw std::runtime_error("Failed to open file");
        }
        try
        {
            TokenStream ts{is};
            auto program = parseProgram(ts);


            auto interpreter = EvaluateVisitor{ctx};
            auto formatter = FormatVisitor{};
            if constexpr (EVALUATE)
                program->accept(interpreter);
            if constexpr (FORMAT)
            {
                program->accept(formatter);
                cout << formatter.result();
            }
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
                    auto interpreter = EvaluateVisitor{ctx};
                    TokenStream ts{sis};
                    nodes.push_back(parseStatement(ts));
                    nodes.back()->accept(interpreter);
                    if (!interpreter.result.isNull())
                    {
                        if (interpreter.result.isNull())
                            cout << color::black;
                        else if (interpreter.result.isCallable())
                            cout << color::blue;
                        else cout << color::yellow;
                        if (interpreter.result.isString())
                        {
                            cout << "'";
                            printRuntimeValue(cout, interpreter.result);
                            cout << "'";
                        }
                        else printRuntimeValue(cout, interpreter.result);
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
