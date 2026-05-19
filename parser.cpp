//Expression
//Term
//Factor

#include <iostream>
#include "parser.h"
#include "ast.h"

using std::cin;
using std::cout;

unique_ptr<ExpressionNode> parseStatement(TokenStream& ts)
{
    // cout << "parseExpression" << std::endl;
    Token t1 = ts.peek();
    Token t2 = ts.peekNext();
    // cout << static_cast<int>(t.type) << std::endl;
    if (t1.type == TokenType::Assign)
        throw std::runtime_error("'=': expected an l-value for left operand");
    if (t1.type != TokenType::Identifier && t2.type == TokenType::Assign)
        throw std::runtime_error("'=': left operand must be l-value");
    if (t1.type == TokenType::Identifier && t2.type == TokenType::Assign)
    {
        unique_ptr<VariableNode> lvalue = make_unique<VariableNode>(t1.name);
        ts.getNextToken();
        ts.getNextToken();
        unique_ptr<ExpressionNode> rvalue = parseEquality(ts);
        return make_unique<AssignmentNode>(std::move(lvalue), std::move(rvalue));
    }
    return parseEquality(ts);
}


unique_ptr<ExpressionNode> parseEquality(TokenStream& ts)
{
    auto lval = parseComparison(ts);
    while (true)
    {
        std::cout << "IMP TEST" << std::endl;
        Token t = ts.peek();
        std::cout << "IMP TEST CLOSED" << std::endl;
        std::cout << "ParseEquality " << getStringForType(t.type) << std::endl;

        if (t.type == TokenType::Equal || t.type == TokenType::NotEqual)
        {
            ts.getNextToken();
            auto rval = parseComparison(ts);
            lval = make_unique<BinaryNode>(t, std::move(lval), std::move(rval));
        }
        else break;
    }
    return lval;
}

unique_ptr<ExpressionNode> parseComparison(TokenStream& ts)
{
    auto lval = parseExpression(ts);
    while (true)
    {
        Token t = ts.peek();
        std::cout << "ParseComparison " << getStringForType(t.type) << std::endl;
        if (t.type == TokenType::Greater || t.type == TokenType::Less)
        {
            t = ts.getNextToken();
            auto rval = parseExpression(ts);
            lval = make_unique<BinaryNode>(t, std::move(lval), std::move(rval));
        }
        else if (t.type == TokenType::LessEqual || t.type == TokenType::GreaterEqual)
        {
            // std::cout << "Test1" << std::endl;
            t = ts.getNextToken();
            std::cout << "Test: " << getStringForType(t.type) << std::endl;
            // std::cout << "Test2" << std::endl;
            auto rval = parseExpression(ts);
            lval = make_unique<BinaryNode>(t, std::move(lval), std::move(rval));
        }
        else break;
    }
    return lval;
}

unique_ptr<ExpressionNode> parseExpression(TokenStream& ts)
{
    cout << "parseExpression" << std::endl;

    auto lval = parseTerm(ts);
    while (true)
    {
        Token t = ts.peek();
        // cout << "t.type in expression peek = " << getStringForType(t.type) << t.value << std::endl;
        if (t.type == TokenType::Plus || t.type == TokenType::Minus)
        {
            t = ts.getNextToken();
            unique_ptr<ExpressionNode> rval = parseTerm(ts);
            lval = make_unique<BinaryNode>(t, std::move(lval), std::move(rval));
        }
        else
            break;
    }
    return lval;
}

unique_ptr<ExpressionNode> parseTerm(TokenStream& ts)
{
    cout << "parseTerm" << std::endl;

    unique_ptr<ExpressionNode> lval = parseFactor(ts);
    while (true)
    {
        Token t = ts.peek();
        if (t.type == TokenType::Multiply || t.type == TokenType::Divide)
        {
            t = ts.getNextToken();
            unique_ptr<ExpressionNode> rval = parseFactor(ts);
            lval = make_unique<BinaryNode>(t, std::move(lval), std::move(rval));
        }
        else break;
    }
    return lval;
}

unique_ptr<ExpressionNode> parseFactor(TokenStream& ts)
{
    cout << "parseFactor1" << std::endl;
    Token t = ts.getNextToken();
    cout << "parseFactor2" << std::endl;
    cout << "t.type in parseFactor" << getStringForType(t.type) << std::endl;
    if (t.type == TokenType::Number)
    {
        return make_unique<NumberNode>(t.value);
    }
    if (t.type == TokenType::Minus)
    {
        return make_unique<UnaryNode>(t, parseFactor(ts));
    }
    if (t.type == TokenType::Plus)
    {
        return make_unique<UnaryNode>(t, parseFactor(ts));
    }
    if (t.type == TokenType::OpenParen)
    {
        unique_ptr<ExpressionNode> node = parseEquality(ts);
        t = ts.getNextToken();
        if (t.type != TokenType::CloseParen)
        {
            cout << getStringForType(t.type) << std::endl;
            throw std::runtime_error("missing ')'");
        }
        return node;
    }
    if (t.type == TokenType::Identifier)
    {
        string name = t.name;
        t = ts.peek();
        if (t.type == TokenType::OpenParen)
        {
            t = ts.getNextToken();
            t = ts.peek();
            if (t.type == TokenType::CloseParen)
            {
                t = ts.getNextToken();
                return make_unique<FunctionCallNode>(name);
            }
            if (t.type == TokenType::Semicolon)
            {
                throw std::runtime_error("expected ')' before ';' token");
            }

            vector<unique_ptr<ExpressionNode>> argumentNodes;
            while (true)
            {
                t = ts.peek();
                // ERROR: func(Expression , ,);
                if (t.type == TokenType::Comma)
                    throw std::runtime_error("expected expression before ',' token");

                argumentNodes.push_back(parseExpression(ts));
                t = ts.peek();
                cout << "t.type " << getStringForType(t.type) << std::endl;
                if (t.type != TokenType::Comma && t.type != TokenType::CloseParen)
                {
                    // ERROR: func(Expression;
                    if (t.type == TokenType::Semicolon)
                        throw std::runtime_error("expected ')' before ';' token in function");
                    else
                        // ERROR: func(Expression Expression)
                        throw std::runtime_error("expected ',' before expression");
                }
                if (t.type == TokenType::Comma)
                {
                    t = ts.getNextToken();
                }
                else break;
                t = ts.peek();
                if (t.type == TokenType::CloseParen)
                    // ERROR: func(Expression, )
                    throw std::runtime_error("expected expression before ')' token");
            }

            t = ts.peek();
            if (t.type == TokenType::CloseParen)
            {
                t = ts.getNextToken();
                return make_unique<FunctionCallNode>(name, std::move(argumentNodes));
            }
            else
            // Bracket not closed before SEMICOLON
                throw std::runtime_error("expected ')' before ';' token");
        }
        return make_unique<VariableNode>(name);
    }
    // Invalid Operand error
    throw std::runtime_error("invalid operand");
}
