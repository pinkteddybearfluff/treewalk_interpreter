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
    try
    {
        std::ifstream is("/home/wcosmo/Desktop/Projects/myLang_test/main.som");
        if (!is.is_open())
        {
            throw std::runtime_error("Failed to open file");
        }

        EnvironmentStack env;

        TokenStream ts{is};
        int result{0};
        vector<unique_ptr<StatementNode>> nodes;
        while (true)
        {
            nodes.push_back(parseStatement(ts));
            // std::cout << "Result : " << result << std::endl;
            if (match(TokenType::End, ts))
            {
                break;
            }
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
        std::cerr << "error: " << e.what() << "\n";
    }
}
