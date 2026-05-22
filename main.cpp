#include <iostream>
#include <sstream>
#include <fstream>
#include <string>
#include <map>
#include "parser.h"
#include "ast.h"
#include "lexer.h"
#include "stacks.h"


//TODO: 2) add syntax highlighting


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
        // std::string input = "let g = 1;"
        //     "if(g == 1){ let g=2;}";
        // std::istringstream is(input);
        TokenStream ts{is};
        int result{0};
        while (true)
        {
            unique_ptr<ExpressionNode> node = parseStatement(ts);
            result = node->evaluateNode(env);
            std::cout << "Result : " << result << std::endl;

            if constexpr (DEBUG_AST) node->debugPrint(1);

            if (match(TokenType::End, ts))
            {
                break;
            }
        }

        env.debugEnvPrint();

        return 0;
    }
    catch (const std::exception& e)
    {
        std::cerr << "error: " << e.what() << "\n";
    }
}
