//Expression
//Term
//Factor

#include <iostream>
#include "parser.h"
#include "ast.h"

using std::cin;
using std::cout;

constexpr bool DEBUG_PARSER = true;

void debugConsume(std::string_view parserName, const Token& t)
{
    cout << parserName << " consumes " << getStringForType(t.type) << std::endl;
}

void debugEnter(std::string_view parserName)
{
    cout << "Enter " << parserName << std::endl;
}

void debugExit(std::string_view parserName)
{
    cout << "Exit " << parserName << std::endl;
}

void debugPeek(std::string_view parserName, const Token& t)
{
    cout << parserName << " peeked " << getStringForType(t.type) << std::endl;
}

void debugNextPeek(std::string_view parserName, const Token& t)
{
    cout << parserName << " peeked next " << getStringForType(t.type) << std::endl;
}


unique_ptr<ExpressionNode> parseStatement(TokenStream& ts)
{
    if constexpr (DEBUG_PARSER)
        debugEnter("parseStatement");

    Token t1 = ts.peek();
    Token t2 = ts.peekNext();
    if constexpr (DEBUG_PARSER)
    {
        debugPeek("parseStatement", t1);
        debugNextPeek("parseStatement", t2);
    }
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
        if constexpr (DEBUG_PARSER)
            debugExit("parseStatement");
        return make_unique<AssignmentNode>(std::move(lvalue), std::move(rvalue));
    }

    if constexpr (DEBUG_PARSER)
        debugExit("parseStatement");

    return parseEquality(ts);
}


unique_ptr<ExpressionNode> parseEquality(TokenStream& ts)
{
    if constexpr (DEBUG_PARSER)
        debugEnter("parseEquality");
    auto lval = parseComparison(ts);
    while (true)
    {
        Token t = ts.peek();
        if constexpr (DEBUG_PARSER)
            debugPeek("parseEquality", t);

        if (t.type == TokenType::Equal || t.type == TokenType::NotEqual)
        {
            t = ts.getNextToken();
            if constexpr (DEBUG_PARSER)
                debugConsume("parseEquality", t);

            auto rval = parseComparison(ts);
            lval = make_unique<BinaryNode>(t, std::move(lval), std::move(rval));
        }
        else break;
    }

    if constexpr (DEBUG_PARSER)
        debugExit("parseEquality");
    return lval;
}

unique_ptr<ExpressionNode> parseComparison(TokenStream& ts)
{
    if constexpr (DEBUG_PARSER)
        debugEnter("parseComparison");
    auto lval = parseExpression(ts);
    while (true)
    {
        Token t = ts.peek();
        if constexpr (DEBUG_PARSER)
            debugPeek("parseComparison", t);

        if (t.type == TokenType::Greater || t.type == TokenType::Less)
        {
            t = ts.getNextToken();
            if constexpr (DEBUG_PARSER)
                debugConsume("parseComparison", t);
            auto rval = parseExpression(ts);
            lval = make_unique<BinaryNode>(t, std::move(lval), std::move(rval));
        }
        else if (t.type == TokenType::LessEqual || t.type == TokenType::GreaterEqual)
        {
            t = ts.getNextToken();
            auto rval = parseExpression(ts);
            lval = make_unique<BinaryNode>(t, std::move(lval), std::move(rval));
        }
        else break;
    }
    if constexpr (DEBUG_PARSER)
        debugExit("parseComparison");
    return lval;
}

unique_ptr<ExpressionNode> parseExpression(TokenStream& ts)
{
    if constexpr (DEBUG_PARSER)
        debugEnter("parseExpression");
    auto lval = parseTerm(ts);
    while (true)
    {
        Token t = ts.peek();
        if constexpr (DEBUG_PARSER)
            debugPeek("parseExpression", t);

        if (t.type == TokenType::Plus || t.type == TokenType::Minus)
        {
            t = ts.getNextToken();
            if constexpr (DEBUG_PARSER)
                debugConsume("parseExpression", t);

            unique_ptr<ExpressionNode> rval = parseTerm(ts);
            lval = make_unique<BinaryNode>(t, std::move(lval), std::move(rval));
        }
        else
            break;
    }

    if constexpr (DEBUG_PARSER)
        debugExit("parseExpression");
    return lval;
}

unique_ptr<ExpressionNode> parseTerm(TokenStream& ts)
{
    if constexpr (DEBUG_PARSER)
        debugEnter("parseTerm");

    unique_ptr<ExpressionNode> lval = parseFactor(ts);
    while (true)
    {
        Token t = ts.peek();
        if constexpr (DEBUG_PARSER)
            debugPeek("parseTerm", t);
        if (t.type == TokenType::Multiply || t.type == TokenType::Divide)
        {
            t = ts.getNextToken();
            if constexpr (DEBUG_PARSER)
                debugConsume("parseTerm", t);
            unique_ptr<ExpressionNode> rval = parseFactor(ts);
            lval = make_unique<BinaryNode>(t, std::move(lval), std::move(rval));
        }
        else break;
    }

    if constexpr (DEBUG_PARSER)
        debugExit("parseTerm");
    return lval;
}

unique_ptr<ExpressionNode> parseFactor(TokenStream& ts)
{
    if constexpr (DEBUG_PARSER)
        debugEnter("parseFactor");

    Token t = ts.getNextToken();
    if constexpr (DEBUG_PARSER)
        debugConsume("parseTerm", t);

    if (t.type == TokenType::Number)
    {
        if constexpr (DEBUG_PARSER)
            debugExit("parseFactor");
        return make_unique<NumberNode>(t.value);
    }
    if (t.type == TokenType::Minus)
    {
        if constexpr (DEBUG_PARSER)
            debugExit("parseFactor");
        return make_unique<UnaryNode>(t, parseFactor(ts));
    }
    if (t.type == TokenType::Plus)
    {
        if constexpr (DEBUG_PARSER)
            debugExit("parseFactor");
        return make_unique<UnaryNode>(t, parseFactor(ts));
    }
    if (t.type == TokenType::OpenParen)
    {
        unique_ptr<ExpressionNode> node = parseEquality(ts);
        t = ts.getNextToken();
        if constexpr (DEBUG_PARSER)
            debugConsume("parseFactor", t);
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
        if constexpr (DEBUG_PARSER)
            debugPeek("parseFactor", t);
        if (t.type == TokenType::OpenParen)
        {
            t = ts.getNextToken();
            if constexpr (DEBUG_PARSER)
                debugConsume("parseFactor", t);
            t = ts.peek();
            if constexpr (DEBUG_PARSER)
                debugPeek("parseFactor", t);
            if (t.type == TokenType::CloseParen)
            {
                t = ts.getNextToken();
                if constexpr (DEBUG_PARSER)
                    debugConsume("parseFactor", t);
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
                if constexpr (DEBUG_PARSER)
                    debugPeek("parseFactor", t);
                // ERROR: func(Expression , ,);
                if (t.type == TokenType::Comma)
                    throw std::runtime_error("expected expression before ',' token");

                argumentNodes.push_back(parseExpression(ts));
                t = ts.peek();
                if constexpr (DEBUG_PARSER)
                    debugPeek("parseFactor", t);
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
                    if constexpr (DEBUG_PARSER)
                        debugConsume("parseFactor", t);
                }
                else break;
                t = ts.peek();
                if constexpr (DEBUG_PARSER)
                    debugPeek("parseFactor", t);
                if (t.type == TokenType::CloseParen)
                    // ERROR: func(Expression, )
                    throw std::runtime_error("expected expression before ')' token");
            }

            t = ts.peek();
            if constexpr (DEBUG_PARSER)
                debugPeek("parseFactor", t);
            if (t.type == TokenType::CloseParen)
            {
                t = ts.getNextToken();
                if constexpr (DEBUG_PARSER)
                    debugConsume("parseFactor", t);
                return make_unique<FunctionCallNode>(name, std::move(argumentNodes));
            }
            else
            // Bracket not closed before SEMICOLON
                throw std::runtime_error("expected ')' before ';' token");
        }
        return make_unique<VariableNode>(name);
    }
    if (t.type == TokenType::If)
    {
        t = ts.peek();
        if constexpr (DEBUG_PARSER)
            debugPeek("parseFactor", t);
        unique_ptr<ExpressionNode> condition;
        if (t.type == TokenType::OpenParen)
        {
            t = ts.getNextToken();
            if constexpr (DEBUG_PARSER)
                debugConsume("parseFactor", t);
            condition = parseStatement(ts);
        }
        t = ts.getNextToken();
        if constexpr (DEBUG_PARSER)
            debugConsume("parseFactor", t);
        if (t.type != TokenType::CloseParen)
        {
            throw std::runtime_error("expected ')' after condition");
        }
        unique_ptr<ExpressionNode> statement = parseStatement(ts);
        t = ts.peek();
        if constexpr (DEBUG_PARSER)
        {
            debugPeek("parseFactor", t);
            debugExit("parseFactor");
        }

        return make_unique<IfNode>(std::move(condition), std::move(statement));
    }
    // Invalid Operand error
    throw std::runtime_error("invalid operand");
}
