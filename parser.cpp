//Function Correct Cases:   1) func();
//                          2) func(Exp);
//                          3) func(Exp, Exp, ..., Exp);

// Function Error Cases:    1) func(Exp Exp);  ----> Missing Comma between Expression
//                          2) func(Exp, ,);   ----> Missing Expression before Commas
//                          3) func(,);        ----> Missing Expression before Comma
//                          4) func(Exp;       ----> Missing CloseParen before SemiColon
//                          5) func(;          ----> Missing CloseParen before SemiColon
//                          6) func(Exp, )     ----> Missing Expression before CloseParen


#include <iostream>
#include "parser.h"
#include "ast.h"

using std::cin;
using std::cout;

// check: Is the next token of this type?
// match: If next token matches, consume it and return true.
// consume: Consume token of this type or throw error.

bool check(TokenType tkType, TokenStream& ts)
{
    Token t = ts.peek();
    if constexpr (DEBUG_PARSER)
        debugPeek("check", t);
    return t.type == tkType;
}

bool match(TokenType tkType, TokenStream& ts)
{
    if (check(tkType, ts))
    {
        ts.getNextToken();
        return true;
    }
    return false;
}

Token consume(TokenType tkType, string msg, TokenStream& ts)
{
    if (!check(tkType, ts))
        throw std::runtime_error(msg);
    return ts.getNextToken();
}

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

    // Token t1 = ts.peek();
    // if constexpr (DEBUG_PARSER) debugPeek(parserName, t1);

    if (check(TokenType::Assign, ts))
        throw std::runtime_error("'=': expected an l-value for left operand");

    if (ts.peekNext().type == TokenType::Assign)
    {
        Token t = consume(TokenType::Identifier, "'=': left operand must be l-value", ts);

        unique_ptr<VariableNode> lvalue = make_unique<VariableNode>(t.name);

        match(TokenType::Assign, ts);

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

        consume(TokenType::CloseParen, "missing ')'", ts);

        if constexpr (DEBUG_PARSER) debugExit(parserName);
        return node;
    }
    if (t.type == TokenType::Identifier)
    {
        string name = t.name;

        if (match(TokenType::OpenParen, ts))
        {
            // Correct Case 1: func();
            if (match(TokenType::CloseParen, ts))
            {
                if constexpr (DEBUG_PARSER) debugExit(parserName);
                return make_unique<FunctionCallNode>(name);
            }
            // Error Case 5 : func(;
            if (match(TokenType::Semicolon, ts)) throw std::runtime_error("expected ')' before ';'");

            vector<unique_ptr<ExpressionNode>> argumentNodes;
            while (true)
            {
                // Error Case 2 and 3: func(Exp , ,); and func(, Exp);
                if (match(TokenType::Comma, ts))
                    throw std::runtime_error("expected expression before ',' token");

                argumentNodes.push_back(parseExpression(ts));

                // Correct Case 2: func(Exp, Exp);
                if (match(TokenType::CloseParen, ts))
                {
                    if constexpr (DEBUG_LEXER) debugExit(parserName);
                    return make_unique<FunctionCallNode>(name, std::move(argumentNodes));
                }

                // Error Case 4: func(Exp, Exp;
                if (match(TokenType::Semicolon, ts)) throw std::runtime_error("expected ')' before ';'");

                if (match(TokenType::Comma, ts))
                {
                    // Error Case 6: func(Exp, );
                    if (match(TokenType::CloseParen, ts))
                        throw std::runtime_error("expected expression before ')'");
                    // Correct Case 3; func(Exp, Exp, ...);
                    continue;
                };

                // Error Case 1: func(Exp Exp);
                throw std::runtime_error("expected ',' before expression");
            }
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
