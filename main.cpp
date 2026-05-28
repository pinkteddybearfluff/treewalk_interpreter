#include <iostream>
#include <sstream>
#include <fstream>
#include "parser.h"
#include "ast.h"
#include "lexer.h"
#include "stacks.h"

// The Laven Language Lavender

//TODO: 1) add better error handling with added string
//TODO: 2) add user defined function
//TODO: 3) add syntax highlighting
//TODO: 4) add +=, -=, *= /=


int main()
{
    EnvironmentStack env;
    string file = "/home/wcosmo/Desktop/Projects/myLang_test/main.som";
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

        unique_ptr<ProgramNode> program = make_unique<ProgramNode>(std::move(nodes));
        program->evaluateNode(env);

        if constexpr (DEBUG_AST)
            program->debugPrint(0);


        if constexpr (DEBUG_ENV)
            env.debugEnvPrint();

        return 0;
    }
    catch (const std::exception& e)
    {
        std::cerr << "Line " << ts.getLineNo() << " error: " << e.what() << "\n";

        if constexpr (DEBUG_ENV)
            env.debugEnvPrint();
    }
}
