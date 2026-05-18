#include <iostream>
#include <sstream>
#include <string>
#include <map>
#include "parser.h"
#include "ast.h"
#include "lexer.h"

//TODO: 1) add better error messages and more error coverage for functions and new error
//TODO: 2) add functions
//TODO: 3) add ==, !=, >, <
//TODO: 4) add control flow

int main()
{
    try
    {
        std::map<string, int> env;
        env["x"] = -4;
        std::string input = "avg(5,4,5;";
        std::istringstream is(input);
        TokenStream ts{is};
        unique_ptr<ExpressionNode> node;
        int result{0};
        while (true)
        {
            node = parseStatement(ts);
            Token t = ts.getNextToken();
            // std::cout << "After statement peek " << getStringForType(t.type) << std::endl;

            if (t.type != TokenType::Semicolon)
                throw std::runtime_error("syntax error : missing ';'");
            t = ts.peek();
            result = node->evaluateNode(env);
            std::cout << "Result : " << result << std::endl;

            // std::cout << "After statement peek " << getStringForType(t.type) << std::endl;
            if (t.type == TokenType::End)
            {
                break;
            }
            // node->debugPrint(0);
        }
        std::cout << env["x"];

        return 0;
    }
    catch (const std::exception& e)
    {
        std::cerr << "error: " << e.what() << "\n";
    }
}
