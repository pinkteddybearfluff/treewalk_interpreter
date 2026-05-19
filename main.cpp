#include <iostream>
#include <sstream>
#include <string>
#include <map>
#include "parser.h"
#include "ast.h"
#include "lexer.h"

//TODO: 1) add control flow
//TODO: 2) pretty print for nodes and tree
//TODO: 3) add helper functions such as match(Equal) and expect(CloseParen)

int main()
{
    try
    {
        std::map<string, int> env;
        // env["x"] = -4;
        std::string input = "a = 3;"
            "b = 6;"
            "max1 = a*(a>b)+b*(a<b);"
            "min1= a*(a<b)+ b*(b<a);"
            "max2 = max(a,b);"
            "min2 = min(a,b);"
            "avg = avg(a,b,max1,max2,min1,min2);";
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
