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

int functionLevel{0};
int loopLevel{0};
int parserDepth{0};

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
        throw ParserError(msg, ts.getLineNo());
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
    if (t.type == TokenType::Identifier)
    {
        cout << t.name << std::endl;
    }
}

void debugNextPeek(std::string_view parserName, const Token& t)
{
    cout << string(parserDepth * 2, ' ') << parserName << " peeked next " << getStringForType(t.type) << std::endl;
}


unique_ptr<StatementNode> parseStatement(TokenStream& ts)
{
    string parserName = "parseStatement";
    if constexpr (DEBUG_PARSER) debugEnter(parserName);

    if (check(TokenType::Semicolon, ts))
    {
        match(TokenType::Semicolon, ts);
        if constexpr (DEBUG_PARSER)
            debugExit(parserName);
        return make_unique<EmptyNode>();
    }

    if (check(TokenType::Assign, ts))
        throw ParserError("'=': expected an l-value for left operand", ts.getLineNo());

    if (check(TokenType::Let, ts))
    {
        unique_ptr<StatementNode> stmt = parseDeclaration(ts);
        consume(TokenType::Semicolon, "expected ';' after declaration", ts);
        if constexpr (DEBUG_PARSER) debugExit(parserName);
        return stmt;
    }
    if (check(TokenType::If, ts))
    {
        if constexpr (DEBUG_PARSER) debugExit(parserName);
        return parseIfStatement(ts);
    }
    if (check(TokenType::While, ts))
    {
        if constexpr (DEBUG_PARSER) debugExit(parserName);
        return parseWhileStatement(ts);
    }
    if (check(TokenType::For, ts))
    {
        if constexpr (DEBUG_PARSER) debugExit(parserName);
        return parseForStatement(ts);
    }
    if (check(TokenType::Break, ts))
    {
        cout << loopLevel << std::endl;
        if (loopLevel > 0)
        {
            match(TokenType::Break, ts);
            consume(TokenType::Semicolon, "expected ';' after break statement", ts);
            if constexpr (DEBUG_PARSER) debugExit(parserName);
            --loopLevel;
            return make_unique<BreakNode>();
        }
        throw ParserError("break statement not within a loop", ts.getLineNo());
    }
    if (check(TokenType::Continue, ts))
    {
        if (loopLevel > 0)
        {
            match(TokenType::Continue, ts);
            consume(TokenType::Semicolon, "expected ';' after continue statement", ts);
            if constexpr (DEBUG_PARSER) debugExit(parserName);
            return make_unique<ContinueNode>();
        }
        throw ParserError("continue statement not within a loop", ts.getLineNo());
    }
    if (check(TokenType::OpenBrace, ts))
    {
        if constexpr (DEBUG_PARSER) debugExit(parserName);
        return parseBlock(ts);
    }
    if (check(TokenType::Function, ts))
    {
        if constexpr (DEBUG_PARSER) debugExit(parserName);
        return parseFunctionDeclaration(ts);
    }
    if (check(TokenType::Return, ts))
    {
        if (functionLevel > 0)
        {
            match(TokenType::Return, ts);
            if (match(TokenType::Semicolon, ts)) return make_unique<ReturnNode>();
            unique_ptr<ExpressionNode> returnNode = parseEquality(ts);
            consume(TokenType::Semicolon, "expected ';' after return statement", ts);
            if constexpr (DEBUG_PARSER) debugExit(parserName);
            --functionLevel;
            return make_unique<ReturnNode>(std::move(returnNode));
        }
        throw ParserError("return statement not within a function", ts.getLineNo());
    }

    unique_ptr<StatementNode> expressionNode = parseExpressionStatement(ts);
    consume(TokenType::Semicolon, "expected ';' after statement" + std::to_string(ts.getLineNo()), ts);
    if constexpr (DEBUG_PARSER) debugExit(parserName);
    return expressionNode;
}

unique_ptr<StatementNode> parseDeclaration(TokenStream& ts)
{
    string parserName = "parseDeclaration";
    if constexpr (DEBUG_PARSER) debugEnter(parserName);

    match(TokenType::Let, ts);
    unique_ptr<ExpressionNode> left = parseEquality(ts);
    if (left->isDeclarationTarget())
    {
        if (match(TokenType::Assign, ts))
        {
            unique_ptr<ExpressionNode> expr = parseEquality(ts);
            if constexpr (DEBUG_PARSER) debugExit(parserName);
            return make_unique<DeclarationNode>(std::move(left), std::move(expr),
                                                ts.getLineNo());
        }
        if (!check(TokenType::Semicolon, ts))
            throw ParserError("expected ';' after declaration", ts.getLineNo());

        if constexpr (DEBUG_PARSER) debugExit(parserName);
        return make_unique<DeclarationNode>(std::move(left), ts.getLineNo());
    }
    throw ParserError(std::format("cannot declare to {}", left->description()), ts.getLineNo());
}

unique_ptr<StatementNode> parseExpressionStatement(TokenStream& ts)
{
    string parserName = "parseExpressionStatement";
    if constexpr (DEBUG_PARSER) debugEnter(parserName);
    unique_ptr<ExpressionNode> expr = parseAssignment(ts);
    if constexpr (DEBUG_PARSER) debugExit(parserName);
    return make_unique<ExpressionStatementNode>(std::move(expr));
}

unique_ptr<ExpressionNode> parseAssignment(TokenStream& ts)
{
    string parserName = "parseAssignment";
    if constexpr (DEBUG_PARSER) debugEnter(parserName);

    auto lhs = parseEquality(ts);
    if (match(TokenType::Assign, ts))
    {
        if (lhs->isAssignmentTarget())
        {
            auto rhs = parseEquality(ts);

            if constexpr (DEBUG_PARSER) debugExit(parserName);
            return make_unique<AssignmentNode>(std::move(lhs), std::move(rhs), ts.getLineNo());
        }
        throw ParserError(std::format("cannot assign to {}", lhs->description()), ts.getLineNo());
    }
    if constexpr (DEBUG_PARSER) debugExit(parserName);
    return lhs;
}

unique_ptr<StatementNode> parseIfStatement(TokenStream& ts)
{
    string parserName = "parseIfStatement";
    if constexpr (DEBUG_PARSER) debugEnter(parserName);
    match(TokenType::If, ts);
    consume(TokenType::OpenParen, "expected '(' after if", ts);
    unique_ptr<ExpressionNode> condition = parseAssignment(ts);
    consume(TokenType::CloseParen, "expected ')' after condition statement", ts);

    unique_ptr<StatementNode> thenStatement = parseStatement(ts);
    if (match(TokenType::Else, ts))
    {
        unique_ptr<StatementNode> elseStatement = parseStatement(ts);
        if constexpr (DEBUG_PARSER) debugExit(parserName);
        return make_unique<IfNode>(std::move(condition), std::move(thenStatement), std::move(elseStatement));
    }

    if constexpr (DEBUG_PARSER) debugExit(parserName);
    return make_unique<IfNode>(std::move(condition), std::move(thenStatement));
}

unique_ptr<StatementNode> parseWhileStatement(TokenStream& ts)
{
    ++loopLevel;
    string parserName = "parseWhileStatement";
    if constexpr (DEBUG_PARSER) debugEnter(parserName);
    match(TokenType::While, ts);
    consume(TokenType::OpenParen, "expected '(' after while", ts);
    unique_ptr<ExpressionNode> condition = parseAssignment(ts);
    consume(TokenType::CloseParen, "expected ')' after condition", ts);

    unique_ptr<StatementNode> statement = parseStatement(ts);

    if constexpr (DEBUG_PARSER) debugExit(parserName);
    --loopLevel;
    return make_unique<WhileNode>(std::move(condition), std::move(statement));
}

unique_ptr<StatementNode> parseForStatement(TokenStream& ts)
{
    ++loopLevel;
    string parserName = "parseForStatement";
    if constexpr (DEBUG_PARSER) debugEnter(parserName);

    match(TokenType::For, ts);
    consume(TokenType::OpenParen, "expected '(' after for", ts);
    unique_ptr<StatementNode> init;
    unique_ptr<ExpressionNode> cond;
    unique_ptr<ExpressionNode> expr;
    unique_ptr<StatementNode> body;
    if (match(TokenType::Semicolon, ts))
    {
        if (match(TokenType::Semicolon, ts))
        {
            if (match(TokenType::CloseParen, ts))
            {
                body = parseStatement(ts);
                --loopLevel;
                if constexpr (DEBUG_PARSER) debugExit(parserName);
                return make_unique<ForNode>(std::move(body));
            }
            expr = parseAssignment(ts);
            consume(TokenType::CloseParen, "expected ')' after for", ts);
            body = parseStatement(ts);
            --loopLevel;
            if constexpr (DEBUG_PARSER) debugExit(parserName);
            return make_unique<ForNode>(std::move(body), nullptr, nullptr, std::move(expr));
        }
        cond = parseAssignment(ts);
        consume(TokenType::Semicolon, "expected ';' after condition", ts);
        if (match(TokenType::CloseParen, ts))
        {
            body = parseStatement(ts);
            if constexpr (DEBUG_PARSER) debugExit(parserName);
            --loopLevel;
            return make_unique<ForNode>(std::move(body), nullptr, std::move(cond));
        }
        expr = parseAssignment(ts);
        consume(TokenType::CloseParen, "expected ')' after for", ts);
        body = parseStatement(ts);
        if constexpr (DEBUG_PARSER) debugExit(parserName);
        --loopLevel;
        return make_unique<ForNode>(std::move(body), nullptr, std::move(cond), std::move(expr));
    }
    if (match(TokenType::Let, ts))
        init = parseDeclaration(ts);
    else init = parseExpressionStatement(ts);
    consume(TokenType::Semicolon, "expected semicolon", ts);
    if (match(TokenType::Semicolon, ts))
    {
        if (match(TokenType::CloseParen, ts))
        {
            body = parseStatement(ts);
            if constexpr (DEBUG_PARSER) debugExit(parserName);
            --loopLevel;
            return make_unique<ForNode>(std::move(body), std::move(init));
        }
        expr = parseAssignment(ts);
        consume(TokenType::CloseParen, "expected ')' after for", ts);
        body = parseStatement(ts);
        if constexpr (DEBUG_PARSER) debugExit(parserName);
        --loopLevel;
        return make_unique<ForNode>(std::move(body), std::move(init), nullptr, std::move(expr));
    }
    cond = parseAssignment(ts);
    consume(TokenType::Semicolon, "expected semicolon", ts);
    if (match(TokenType::CloseParen, ts))
    {
        body = parseStatement(ts);
        if constexpr (DEBUG_PARSER) debugExit(parserName);
        --loopLevel;
        return make_unique<ForNode>(std::move(body), std::move(init), std::move(cond));
    }
    expr = parseAssignment(ts);
    consume(TokenType::CloseParen, "expected ')' after for", ts);
    body = parseStatement(ts);
    if constexpr (DEBUG_PARSER) debugExit(parserName);
    --loopLevel;
    return make_unique<ForNode>(std::move(body), std::move(init), std::move(cond), std::move(expr));
}

unique_ptr<StatementNode> parseBlock(TokenStream& ts)
{
    string parserName = "parseBlock";
    if constexpr (DEBUG_PARSER) debugEnter(parserName);

    match(TokenType::OpenBrace, ts);
    vector<unique_ptr<StatementNode>> statements;
    while (true)
    {
        statements.push_back(parseStatement(ts));
        if (check(TokenType::CloseBrace, ts)) break;
    }
    consume(TokenType::CloseBrace, "expected '}' for block", ts);
    if constexpr (DEBUG_PARSER) debugExit(parserName);

    return make_unique<BlockNode>(std::move(statements));
}

unique_ptr<StatementNode> parseFunctionDeclaration(TokenStream& ts)
{
    ++functionLevel;
    string parserName = "parseFunctionDeclaration";
    if constexpr (DEBUG_PARSER) debugEnter(parserName);
    match(TokenType::Function, ts);
    int funcDeclarationLine = ts.getLineNo();
    Token t = consume(TokenType::Identifier, "expected identifier after fn", ts);
    consume(TokenType::OpenParen, "expected '(' after function name", ts);
    vector<string> parameters;

    if (!match(TokenType::CloseParen, ts))
    {
        while (true)
        {
            Token ti = consume(TokenType::Identifier, "expected parameter", ts);
            parameters.push_back(ti.name);
            if (match(TokenType::CloseParen, ts))
            {
                break;
            }
            match(TokenType::Comma, ts);
        }
    }

    unique_ptr<StatementNode> body = parseBlock(ts);

    if constexpr (DEBUG_PARSER) debugExit(parserName);
    --functionLevel;
    return make_unique<FunctionDeclarationNode>(t.name, parameters, std::move(body), funcDeclarationLine);
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
            lval = make_unique<BinaryNode>(t, std::move(lval), std::move(rval), ts.getLineNo());
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

    auto lval = parseTerm(ts);
    while (true)
    {
        Token t = ts.peek();
        if constexpr (DEBUG_PARSER) debugPeek(parserName, t);

        if (t.type == TokenType::Greater || t.type == TokenType::Less || t.type == TokenType::GreaterEqual || t.type ==
            TokenType::LessEqual)
        {
            t = ts.getNextToken();
            if constexpr (DEBUG_PARSER) debugConsume(parserName, t);

            auto rval = parseTerm(ts);
            lval = make_unique<BinaryNode>(t, std::move(lval), std::move(rval), ts.getLineNo());
        }
        else break;
    }

    if constexpr (DEBUG_PARSER) debugExit(parserName);
    return lval;
}

unique_ptr<ExpressionNode> parseTerm(TokenStream& ts)
{
    string parserName = "parseTerm";
    if constexpr (DEBUG_PARSER) debugEnter(parserName);

    auto lval = parseFactor(ts);
    while (true)
    {
        Token t = ts.peek();
        if constexpr (DEBUG_PARSER) debugPeek(parserName, t);

        if (t.type == TokenType::Plus || t.type == TokenType::Minus)
        {
            t = ts.getNextToken();
            if constexpr (DEBUG_PARSER) debugConsume(parserName, t);

            unique_ptr<ExpressionNode> rval = parseFactor(ts);
            lval = make_unique<BinaryNode>(t, std::move(lval), std::move(rval), ts.getLineNo());
        }
        else
            break;
    }

    if constexpr (DEBUG_PARSER) debugExit(parserName);
    return lval;
}

unique_ptr<ExpressionNode> parseFactor(TokenStream& ts)
{
    string parserName = "parseFactor";
    if constexpr (DEBUG_PARSER) debugEnter(parserName);

    unique_ptr<ExpressionNode> lval = parseUnary(ts);
    while (true)
    {
        Token t = ts.peek();
        if constexpr (DEBUG_PARSER) debugPeek(parserName, t);

        if (t.type == TokenType::Multiply || t.type == TokenType::Divide)
        {
            t = ts.getNextToken();
            if constexpr (DEBUG_PARSER) debugConsume(parserName, t);

            unique_ptr<ExpressionNode> rval = parseUnary(ts);
            lval = make_unique<BinaryNode>(t, std::move(lval), std::move(rval), ts.getLineNo());
        }
        else break;
    }

    if constexpr (DEBUG_PARSER) debugExit(parserName);
    return lval;
}

unique_ptr<ExpressionNode> parseUnary(TokenStream& ts)
{
    string parserName = "parseUnary";
    if constexpr (DEBUG_PARSER) debugEnter(parserName);

    if (check(TokenType::Plus, ts) || check(TokenType::Minus, ts))
    {
        Token t = ts.getNextToken();
        unique_ptr<ExpressionNode> expr = parsePrimary(ts);
        if constexpr (DEBUG_PARSER) debugConsume(parserName, t);
        return make_unique<UnaryNode>(t, std::move(expr));
    }

    auto expr = parsePostFix(ts);
    if constexpr (DEBUG_PARSER) debugExit(parserName);
    return expr;
}

unique_ptr<ExpressionNode> parsePostFix(TokenStream& ts)
{
    string parserName = "parsePostFix";
    if constexpr (DEBUG_PARSER) debugEnter(parserName);
    auto expr = parsePrimary(ts);
    while (match(TokenType::OpenBracket, ts))
    {
        expr = make_unique<IndexNode>(std::move(expr), parseEquality(ts), ts.getLineNo());
        consume(TokenType::CloseBracket, "expected ']' after index", ts);
    }
    if constexpr (DEBUG_PARSER) debugExit(parserName);

    return expr;
}

unique_ptr<ExpressionNode> parsePrimary(TokenStream& ts)
{
    string parserName = "parsePrimary";
    if constexpr (DEBUG_PARSER) debugEnter(parserName);

    Token t = ts.getNextToken();
    if constexpr (DEBUG_PARSER) debugConsume(parserName, t);

    if (t.type == TokenType::String)
    {
        if constexpr (DEBUG_PARSER) debugExit(parserName);
        return make_unique<StringNode>(std::get<string>(t.literal), ts.getLineNo());
    }
    if (t.type == TokenType::Boolean)
    {
        if constexpr (DEBUG_PARSER) debugExit(parserName);
        return make_unique<BooleanNode>(std::get<bool>(t.literal), ts.getLineNo());
    }
    if (t.type == TokenType::Number)
    {
        if constexpr (DEBUG_PARSER) debugExit(parserName);
        return make_unique<NumberNode>(std::get<double>(t.literal), ts.getLineNo());
    }
    if (t.type == TokenType::Null)
    {
        if constexpr (DEBUG_PARSER) debugExit(parserName);
        return make_unique<NullNode>(std::get<std::monostate>(t.literal), ts.getLineNo());
    }
    if (t.type == TokenType::OpenBracket)
    {
        vector<unique_ptr<ExpressionNode>> elements;
        if (match(TokenType::CloseBracket, ts)) return make_unique<ArrayNode>();
        while (true)
        {
            elements.push_back(parseAssignment(ts));
            if (check(TokenType::CloseBracket, ts)) break;
            if (check(TokenType::Comma, ts)) match(TokenType::Comma, ts);
        }
        consume(TokenType::CloseBracket, "Expected ']'" + string(1, static_cast<char>(ts.getLineNo())), ts);

        if constexpr (DEBUG_PARSER) debugExit(parserName);
        return make_unique<ArrayNode>(std::move(elements), ts.getLineNo());
    }
    if (t.type == TokenType::OpenParen)
    {
        unique_ptr<ExpressionNode> node = parseAssignment(ts);

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
                return make_unique<FunctionCallNode>(make_unique<VariableNode>(name, ts.getLineNo()), ts.getLineNo());
            }
            // Error Case 5 : func(;
            if (match(TokenType::Semicolon, ts)) throw ParserError("expected ')' before ';'", ts.getLineNo());

            vector<unique_ptr<ExpressionNode>> argumentNodes;
            while (true)
            {
                // Error Case 2 and 3: func(Exp , ,); and func(, Exp);
                if (match(TokenType::Comma, ts))
                    throw ParserError("expected expression before ',' token", ts.getLineNo());

                argumentNodes.push_back(parseEquality(ts));

                // Correct Case 2: func(Exp, Exp);
                if (match(TokenType::CloseParen, ts))
                {
                    if constexpr (DEBUG_LEXER) debugExit(parserName);
                    return make_unique<FunctionCallNode>(make_unique<VariableNode>(name, ts.getLineNo()),
                                                         std::move(argumentNodes), ts.getLineNo());
                }

                // Error Case 4: func(Exp, Exp;
                if (match(TokenType::Semicolon, ts)) throw ParserError("expected ')' before ';'", ts.getLineNo());

                if (match(TokenType::Comma, ts))
                {
                    // Error Case 6: func(Exp, );
                    if (match(TokenType::CloseParen, ts))
                        throw ParserError("expected expression before ')'", ts.getLineNo());
                    // Correct Case 3; func(Exp, Exp, ...);
                    continue;
                };

                // Error Case 1: func(Exp Exp);
                throw ParserError("expected ',' before expression", ts.getLineNo());
            }
        }

        if constexpr (DEBUG_PARSER) debugExit(parserName);
        return make_unique<VariableNode>(name, ts.getLineNo());
    }

    // Invalid Operand error
    throw ParserError("invalid primary" + getStringForType(t.type), ts.getLineNo());
}

