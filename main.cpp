#include <iostream>
#include <sstream>
#include <fstream>
#include <string_view>
#include <chrono>
#include <format>
#include "parser.h"
#include "ast.h"
#include "lexer.h"
#include "stacks.h"

// The Laven Language Lavender

//TODO: 1) add better error handling with ParserError LexerError Runtime Evaluation Error
//Todo: 2)  virtual bool isAssignmentTarget() const;
//          virtual bool isDeclarationTarget() const;
//          virtual std::string description() const;
//Todo: 3) automate testing with python
//TODO: 3) add syntax highlighting
//TODO: 4) add +=, -=, *= /=


int main(int argc, char** argv)
{
    auto start = std::chrono::high_resolution_clock::now();
    auto parseCompleteFlag = std::chrono::high_resolution_clock::now();
    auto evalCompleteFlag = std::chrono::high_resolution_clock::now();
    EnvironmentStack env;
    // string file = argv[1];
    string file = "../test/test.som";
    std::ifstream is(file);
    if (!is.is_open())
    {
        throw std::runtime_error("Failed to open file");
    }

    TokenStream ts{is};
    try
    {
        int result{0};
        vector<unique_ptr<StatementNode>> nodes;

        while (!check(TokenType::End, ts))
        {
            nodes.push_back(parseStatement(ts));
        }

        parseCompleteFlag = std::chrono::high_resolution_clock::now();
        cout << std::format("<--------Parsing completed-------------> [Time:{}]",
                            std::chrono::duration<double>(parseCompleteFlag - start).count()) << '\n';
        unique_ptr<ProgramNode> program = make_unique<ProgramNode>(std::move(nodes));
        program->evaluateNode(env);

        evalCompleteFlag = std::chrono::high_resolution_clock::now();
        cout << std::format("<----------Evaluation complete---------> [Time:{}]",
                            std::chrono::duration<double>(evalCompleteFlag - start).count()) << '\n';
        if constexpr (DEBUG_AST)
            program->debugPrint(0);


        if constexpr (DEBUG_ENV)
            env.debugEnvPrint();
        auto end = std::chrono::high_resolution_clock::now();

        std::cout
            << std::format("[Total time taken: {}]", std::chrono::duration<double>(end - start).count()) << '\n';

        std::cout
            << std::format("[Time taken for parsing: {}]",
                           std::chrono::duration<double>(parseCompleteFlag - start).count()) << '\n';
        std::cout
            << std::format("Time taken for evaluation: {}]",
                           std::chrono::duration<double>(evalCompleteFlag - parseCompleteFlag).count()) << '\n';

        return 0;
    }
    catch (const LexerError& le)
    {
        std::cerr << "File " << color::magenta << "\"" << file << "\"" << color::reset << ", line " << color::boldBlue
            << le
            .line <<
            color::reset << '\n';
        std::cerr << color::boldRed << "SyntaxError: " << color::reset << le.what() << "\n";
        return 1;
    }
    catch (const ParserError& pe)
    {
        std::cerr << "File " << color::magenta << "\"" << file << "\"" << color::reset << ", line " << color::boldBlue
            << pe
            .line <<
            color::reset << '\n';
        std::cerr << color::boldRed << "SyntaxError: " << color::reset << pe.what() << "\n";
        return 2;
    }
    catch (const RuntimeError& re)
    {
        std::cerr << "File " << color::magenta << "\"" << file << "\"" << color::reset << ", line " << color::boldBlue
            << re.diagnostic.currentLine << color::reset << '\n';
        std::cerr << color::boldRed << getErrorCategoryString(re.diagnostic.category) << ": " << color::reset;

        switch (re.diagnostic.kind)
        {
        case ErrorKind::VariableUndefined:
        case ErrorKind::FunctionUndefined:
            std::cerr << "name '" << color::boldWhite << re.diagnostic.identifier << color::reset << "'" <<
                " is not defined"
                << "\n";
            break;
        case ErrorKind::InvalidIndexType:
            std::cerr << "array indices must be " << color::boldWhite << re.diagnostic.primary << color::reset <<
                ", not " << color::boldWhite << re.diagnostic.secondary << color::reset << "\n";
            break;
        case ErrorKind::UnsupportedOperation:
            std::cerr << "unsupported operand type(s) for "
                << color::boldWhite << re.diagnostic.identifier << color::reset << ": '" << color::boldGreen << re.
                diagnostic.primary << color::reset << "' and '"
                << color::boldBlue << re.diagnostic.secondary << color::reset << "'\n";
            break;
        case ErrorKind::NotSubscriptable:
            std::cerr << "'" << color::boldWhite << re.diagnostic.primary << color::reset <<
                "' object is not subscriptable\n";
            break;
        case ErrorKind::NotCallable:
            std::cerr << "'" << color::boldWhite << re.diagnostic.primary << color::reset <<
                "' object is not callable\n";
            break;
        case ErrorKind::TooManyArguments:
            std::cerr << "too many arguments to function '" << color::boldGreen << re.diagnostic.identifier <<
                color::reset <<
                "()' expected " << color::boldBlue
                << re.diagnostic.expected << color::reset << " have " << color::boldRed << re.diagnostic.actual <<
                color::reset << '\n';
            break;
        case ErrorKind::TooFewArguments:
            std::cerr << "too few arguments to function '" << color::boldGreen << re.diagnostic.identifier <<
                color::reset <<
                "()' expected " << color::boldBlue
                << re.diagnostic.expected << color::reset << " have " << color::boldRed << re.diagnostic.actual <<
                color::reset << '\n';
            break;
        case ErrorKind::IndexOutOfBounds:
            std::cerr << re.diagnostic.primary << " index is out of range\n";
            break;
        case ErrorKind::DivisionByZero:
            std::cerr << "division by zero\n";
            break;
        case ErrorKind::VariableRedeclaration:
            std::cerr << "redeclaration of '" << color::boldWhite << re.diagnostic.identifier << color::reset << "'\n";
            std::cerr << color::boldCyan << "Note: " << color::reset << "'" << color::boldWhite << re.diagnostic.
                identifier << color::reset <<
                "' previously declared on line " << color::boldBlue << re.diagnostic.previousLine << color::reset <<
                "\n";
            break;
        case ErrorKind::FunctionRedeclaration:
            std::cerr << "redefinition of '" << color::boldGreen << re.diagnostic.identifier << color::reset << "()'\n";
            break;
        }

        return 3;
    }
    catch (const std::exception& e)
    {
        std::cerr << "File " << color::magenta << "\"" << file << "\"" << color::reset << ", line " << color::boldBlue
            << ts
            .getLineNo() <<
            color::reset << '\n';
        std::cerr << "Line " << ts.getLineNo() << " error: " << e.what() << "\n";

        if constexpr (DEBUG_ENV)
            env.debugEnvPrint();
        return 4;
    }
}
