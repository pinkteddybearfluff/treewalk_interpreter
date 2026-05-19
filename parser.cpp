//Expression
//Term
//Factor

#include <iostream>
#include "parser.h"
#include "ast.h"

using std::cin;
using std::cout;

void debugConsume(std::string_view parserName, const Token& t)
{
    cout << string(parserDepth * 2, ' ') << parserName << " consumes " << getStringForType(t.type) << std::endl;
}

void debugEnter(std::string_view parserName)
{
    cout << string(parserDepth * 2, ' ') << "Enter " << parserName << std::endl;
    ++parserDepth;
}

void debugExit(std::string_view parserName)
{
    --parserDepth;
    cout << string(parserDepth * 2, ' ') << "Exit " << parserName << std::endl;
}

void debugPeek(std::string_view parserName, const Token& t)
{
    cout << string(parserDepth * 2, ' ') << parserName << " peeked " << getStringForType(t.type) << std::endl;
}

void debugNextPeek(std::string_view parserName, const Token& t)
{
    cout << string(parserDepth * 2, ' ') << parserName << " peeked next " << getStringForType(t.type) << std::endl;
}


unique_ptr<ExpressionNode> parseStatement(TokenStream& ts)
{
    string parserName = "parseStatement";
    if constexpr (DEBUG_PARSER) debugEnter(parserName);

    Token t1 = ts.peek();
    if constexpr (DEBUG_PARSER) debugPeek(parserName, t1);
    Token t2 = ts.peekNext();
    if constexpr (DEBUG_PARSER) debugNextPeek(parserName, t2);

    if (t1.type == TokenType::Assign)
        throw std::runtime_error("'=': expected an l-value for left operand");
    if (t1.type != TokenType::Identifier && t2.type == TokenType::Assign)
        throw std::runtime_error("'=': left operand must be l-value");
    if (t1.type == TokenType::Identifier && t2.type == TokenType::Assign)
    {
        unique_ptr<VariableNode> lvalue = make_unique<VariableNode>(t1.name);
        t1 = ts.getNextToken();
        if constexpr (DEBUG_PARSER) debugConsume(parserName, t1);
        t2 = ts.getNextToken();
        if constexpr (DEBUG_PARSER) debugConsume(parserName, t2);

        unique_ptr<ExpressionNode> rvalue = parseEquality(ts);

        if constexpr (DEBUG_PARSER) debugExit(parserName);
        return make_unique<AssignmentNode>(std::move(lvalue), std::move(rvalue));
    }

    if constexpr (DEBUG_PARSER) debugExit(parserName);
    return parseEquality(ts);
}


unique_ptr<ExpressionNode> parseEquality(TokenStream& ts)
{
    string parserName = "parseEquality";
    if constexpr (DEBUG_PARSER) debugEnter(parserName);

    auto lval = parseComparison(ts);
    while (true)
    {
        Token t = ts.peek();
        if constexpr (DEBUG_PARSER) debugPeek(parserName, t);

        if (t.type == TokenType::Equal || t.type == TokenType::NotEqual)
        {
            t = ts.getNextToken();
            if constexpr (DEBUG_PARSER) debugConsume(parserName, t);

            auto rval = parseComparison(ts);
            lval = make_unique<BinaryNode>(t, std::move(lval), std::move(rval));
        }
        else break;
    }

    if constexpr (DEBUG_PARSER) debugExit(parserName);
    return lval;
}

unique_ptr<ExpressionNode> parseComparison(TokenStream& ts)
{
    string parserName = "parseComparison";
    if constexpr (DEBUG_PARSER) debugEnter(parserName);

    auto lval = parseExpression(ts);
    while (true)
    {
        Token t = ts.peek();
        if constexpr (DEBUG_PARSER) debugPeek(parserName, t);

        if (t.type == TokenType::Greater || t.type == TokenType::Less || t.type == TokenType::GreaterEqual || t.type ==
            TokenType::LessEqual)
        {
            t = ts.getNextToken();
            if constexpr (DEBUG_PARSER) debugConsume(parserName, t);

            auto rval = parseExpression(ts);
            lval = make_unique<BinaryNode>(t, std::move(lval), std::move(rval));
        }
        else break;
    }

    if constexpr (DEBUG_PARSER) debugExit(parserName);
    return lval;
}

unique_ptr<ExpressionNode> parseExpression(TokenStream& ts)
{
    string parserName = "parseExpression";
    if constexpr (DEBUG_PARSER) debugEnter(parserName);

    auto lval = parseTerm(ts);
    while (true)
    {
        Token t = ts.peek();
        if constexpr (DEBUG_PARSER) debugPeek(parserName, t);

        if (t.type == TokenType::Plus || t.type == TokenType::Minus)
        {
            t = ts.getNextToken();
            if constexpr (DEBUG_PARSER) debugConsume(parserName, t);

            unique_ptr<ExpressionNode> rval = parseTerm(ts);
            lval = make_unique<BinaryNode>(t, std::move(lval), std::move(rval));
        }
        else
            break;
    }

    if constexpr (DEBUG_PARSER) debugExit(parserName);
    return lval;
}

unique_ptr<ExpressionNode> parseTerm(TokenStream& ts)
{
    string parserName = "parseTerm";
    if constexpr (DEBUG_PARSER) debugEnter(parserName);

    unique_ptr<ExpressionNode> lval = parseFactor(ts);
    while (true)
    {
        Token t = ts.peek();
        if constexpr (DEBUG_PARSER) debugPeek(parserName, t);

        if (t.type == TokenType::Multiply || t.type == TokenType::Divide)
        {
            t = ts.getNextToken();
            if constexpr (DEBUG_PARSER) debugConsume(parserName, t);

            unique_ptr<ExpressionNode> rval = parseFactor(ts);
            lval = make_unique<BinaryNode>(t, std::move(lval), std::move(rval));
        }
        else break;
    }

    if constexpr (DEBUG_PARSER) debugExit(parserName);
    return lval;
}

unique_ptr<ExpressionNode> parseFactor(TokenStream& ts)
{
    string parserName = "parseFactor";
    if constexpr (DEBUG_PARSER) debugEnter(parserName);

    Token t = ts.getNextToken();
    if constexpr (DEBUG_PARSER) debugConsume(parserName, t);

    if (t.type == TokenType::Number)
    {
        if constexpr (DEBUG_PARSER) debugExit(parserName);
        return make_unique<NumberNode>(t.value);
    }
    if (t.type == TokenType::Minus)
    {
        if constexpr (DEBUG_PARSER) debugExit(parserName);
        return make_unique<UnaryNode>(t, parseFactor(ts));
    }
    if (t.type == TokenType::Plus)
    {
        if constexpr (DEBUG_PARSER) debugExit(parserName);
        return make_unique<UnaryNode>(t, parseFactor(ts));
    }
    if (t.type == TokenType::OpenParen)
    {
        unique_ptr<ExpressionNode> node = parseEquality(ts);
        t = ts.getNextToken();
        if constexpr (DEBUG_PARSER) debugConsume(parserName, t);

        if (t.type != TokenType::CloseParen)
        {
            cout << getStringForType(t.type) << std::endl;
            throw std::runtime_error("missing ')'");
        }
        if constexpr (DEBUG_PARSER) debugExit(parserName);
        return node;
    }
    if (t.type == TokenType::Identifier)
    {
        string name = t.name;
        t = ts.peek();
        if constexpr (DEBUG_PARSER) debugPeek(parserName, t);

        if (t.type == TokenType::OpenParen)
        {
            t = ts.getNextToken();
            if constexpr (DEBUG_PARSER) debugConsume(parserName, t);
            t = ts.peek();
            if constexpr (DEBUG_PARSER) debugPeek(parserName, t);

            if (t.type == TokenType::CloseParen)
            {
                t = ts.getNextToken();
                if constexpr (DEBUG_PARSER) debugConsume(parserName, t);

                if constexpr (DEBUG_PARSER) debugExit(parserName);
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
                if constexpr (DEBUG_PARSER) debugPeek(parserName, t);

                // ERROR: func(Expression , ,);
                if (t.type == TokenType::Comma)
                    throw std::runtime_error("expected expression before ',' token");

                argumentNodes.push_back(parseExpression(ts));
                t = ts.peek();
                if constexpr (DEBUG_PARSER) debugPeek(parserName, t);

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
                    if constexpr (DEBUG_PARSER) debugConsume(parserName, t);
                }
                else break;
                t = ts.peek();
                if constexpr (DEBUG_PARSER) debugPeek(parserName, t);

                if (t.type == TokenType::CloseParen)
                    // ERROR: func(Expression, )
                    throw std::runtime_error("expected expression before ')' token");
            }

            t = ts.peek();
            if constexpr (DEBUG_PARSER) debugPeek(parserName, t);
            if (t.type == TokenType::CloseParen)
            {
                t = ts.getNextToken();
                if constexpr (DEBUG_PARSER) debugConsume(parserName, t);
                return make_unique<FunctionCallNode>(name, std::move(argumentNodes));
            }
            else
            // Bracket not closed before SEMICOLON
                throw std::runtime_error("expected ')' before ';' token");
        }
        if constexpr (DEBUG_PARSER) debugExit(parserName);
        return make_unique<VariableNode>(name);
    }
    if (t.type == TokenType::If)
    {
        t = ts.peek();
        if constexpr (DEBUG_PARSER) debugPeek(parserName, t);

        unique_ptr<ExpressionNode> condition;
        if (t.type == TokenType::OpenParen)
        {
            t = ts.getNextToken();
            if constexpr (DEBUG_PARSER) debugConsume(parserName, t);
            condition = parseStatement(ts);
        }
        t = ts.getNextToken();
        if constexpr (DEBUG_PARSER) debugConsume(parserName, t);

        if (t.type != TokenType::CloseParen)
        {
            throw std::runtime_error("expected ')' after condition");
        }
        unique_ptr<ExpressionNode> statement = parseStatement(ts);
        t = ts.peek();
        if constexpr (DEBUG_PARSER) debugPeek(parserName, t);

        if constexpr (DEBUG_PARSER) debugExit(parserName);
        return make_unique<IfNode>(std::move(condition), std::move(statement));
    }
    // Invalid Operand error
    throw std::runtime_error("invalid operand");
}
