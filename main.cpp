#include <iostream>
#include <sstream>
#include <string>
#include <map>
#include "parser.h"
#include "ast.h"
#include "lexer.h"

//TODO: 1) add control flow
//TODO: 2) add parser helper utility functions: 1) match(Token); 2) consume(Token); 3) check(Token); 4) expect(Token); 5) assert(Token);

int main()
{
    try
    {
        std::map<string, int> env;
        env["a"] = -4;
        std::string input =
            "if(a==-4)b=3;";
        std::istringstream is(input);
        TokenStream ts{is};
        unique_ptr<ExpressionNode> node;
        int result{0};
        while (true)
        {
            node = parseStatement(ts);
            Token t = ts.getNextToken();

            if (t.type != TokenType::Semicolon)
                throw std::runtime_error("missing ';'");
            t = ts.peek();
            result = node->evaluateNode(env);
            std::cout << "Result : " << result << std::endl;

            if constexpr (DEBUG_AST) node->debugPrint(1);

            if (t.type == TokenType::End)
            {
                break;
            }
        }
        for (const auto& var : env)
        {
            std::cout << var.first << " = " << var.second << '\n';
        }

        return 0;
    }
    catch (const std::exception& e)
    {
        std::cerr << "error: " << e.what() << "\n";
    }
}
