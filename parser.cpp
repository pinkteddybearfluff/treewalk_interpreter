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
        Token t = ts.getNextToken();
        if constexpr (DEBUG_PARSER)
            debugConsume("match", t);
        return true;
    }
    return false;
}

Token consume(TokenType tkType, string msg, TokenStream& ts)
{
    if (!check(tkType, ts))
    {
        throw ParserError(msg, ParserError::SyntaxError, ts.getPrevious().line);
    }
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
    if (check(TokenType::End, ts))
        throw ParserError("unexpected EOF", ParserError::SyntaxError, ts.getLineNo());
    if (check(TokenType::Semicolon, ts))
    {
        match(TokenType::Semicolon, ts);
        if constexpr (DEBUG_PARSER)
            debugExit(parserName);
        return make_unique<EmptyNode>();
    }

    if (check(TokenType::Assign, ts))
        throw ParserError("'=': expected an lvalue for left operand", ParserError::SyntaxError, ts.getLineNo());

    if (check(TokenType::Let, ts))
    {
        unique_ptr<StatementNode> stmt = parseDeclaration(ts);
        consume(TokenType::Semicolon, "expected ';' after declaration", ts);
        if constexpr (DEBUG_PARSER) debugExit(parserName);
        return stmt;
    }
    if (check(TokenType::Import, ts))
    {
        unique_ptr<StatementNode> stmt = parseImportStatement(ts);
        if constexpr (DEBUG_PARSER) debugExit(parserName);
        return stmt;
    }
    if (check(TokenType::Try, ts))
    {
        unique_ptr<StatementNode> stmt = parseTryStatement(ts);
        if constexpr (DEBUG_PARSER) debugExit(parserName);
        return stmt;
    }
    if (check(TokenType::Throw, ts))
    {
        match(TokenType::Throw, ts);
        auto expr = parseEquality(ts);
        consume(TokenType::Semicolon, "expected ';' after throw", ts);
        return make_unique<ThrowNode>(std::move(expr));
    }
    if (check(TokenType::Struct, ts))
    {
        unique_ptr<StatementNode> stmt = parseStruct(ts);
        if constexpr (DEBUG_PARSER) debugExit(parserName);
        return stmt;
    }
    if (check(TokenType::Enum, ts))
    {
        unique_ptr<StatementNode> stmt = parseEnum(ts);
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
        throw ParserError("break statement not within a loop", ParserError::SemanticError, ts.getLineNo());
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
        throw ParserError("continue statement not within a loop", ParserError::SemanticError, ts.getLineNo());
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
            return make_unique<ReturnNode>(std::move(returnNode));
        }
        throw ParserError("return statement not within a function", ParserError::SemanticError, ts.getLineNo());
    }

    unique_ptr<StatementNode> expressionNode = parseExpressionStatement(ts);
    consume(TokenType::Semicolon, "expected ';' after expression statement", ts);
    if constexpr (DEBUG_PARSER) debugExit(parserName);
    return expressionNode;
}

auto parseImportStatement(TokenStream& ts) -> unique_ptr<StatementNode>
{
    string parserName = "parseImport";
    if constexpr (DEBUG_PARSER) debugEnter(parserName);
    match(TokenType::Import, ts);
    if (check(TokenType::String, ts))
    {
        Token t = consume(TokenType::String, "expected path to file after import", ts);
        consume(TokenType::As, "expected 'as' after filepath", ts);
        Token t2 = consume(TokenType::Identifier, "expected alias identifier for module", ts);
        return make_unique<ImportNode>(std::get<string>(t.literal), t2.name, ts.getLineNo());
    }
    if (check(TokenType::Identifier, ts))
    {
        Token t = consume(TokenType::Identifier, "expected module", ts);
        consume(TokenType::As, "expected 'as' after module", ts);
        Token t2 = consume(TokenType::Identifier, "expected alias identifier for module", ts);
        return make_unique<ImportNode>(t.name, t2.name, true, ts.getLineNo());
    }
}

auto parseTryStatement(TokenStream& ts) -> unique_ptr<StatementNode>
{
    match(TokenType::Try, ts);
    auto tryStmt = parseBlock(ts);
    consume(TokenType::Catch, "expected catch after try", ts);
    consume(TokenType::OpenParen, "expected '(' after catch", ts);
    Token t = consume(TokenType::Identifier, "expected identifier in catch", ts);
    consume(TokenType::CloseParen, "expected ')' for '(' after catch", ts);
    auto catchStmt = parseBlock(ts);
    vector<CatchClause> catchClauses;
    catchClauses.emplace_back(t.name, std::move(catchStmt));
    return make_unique<TryCatch>(std::move(tryStmt), std::move(catchClauses));
}

auto parseStruct(TokenStream& ts) -> unique_ptr<StatementNode>
{
    match(TokenType::Struct, ts);
    string iden = consume(TokenType::Identifier, "expected identifier after struct", ts).name;
    consume(TokenType::OpenBrace, "expected '{' after struct identifier", ts);
    vector<string> fieldNames;
    vector<unique_ptr<StatementNode>> methods;
    while (true)
    {
        if (check(TokenType::Function, ts))
        {
            methods.push_back(parseFunctionDeclaration(ts));
        }
        else if (check(TokenType::Identifier, ts))
            fieldNames.push_back(consume(TokenType::Identifier, "expected identifier for field name", ts).name);
        else
            throw std::runtime_error("only identifiers and function statements allowed inside structs");
        if (match(TokenType::Comma, ts))
        {
            if (match(TokenType::CloseBrace, ts))break;
            continue;
        }
        if (match(TokenType::CloseBrace, ts))break;
    }
    return make_unique<StructNode>(std::move(iden), std::move(fieldNames), std::move(methods));
}

auto parseEnum(TokenStream& ts) -> unique_ptr<StatementNode>
{
    match(TokenType::Enum, ts);
    string iden = consume(TokenType::Identifier, "expected identifier after enum", ts).name;
    consume(TokenType::OpenBrace, "expected '{' after enum identifier", ts);
    vector<Variant> variants;
    while (true)
    {
        string fieldName = consume(TokenType::Identifier, "expected identifier", ts).name;

        vector<string> fields;
        if (match(TokenType::OpenParen, ts))
        {
            while (true)
            {
                fields.push_back(consume(TokenType::Identifier, "expected identifier", ts).name);
                if (match(TokenType::Comma, ts)) continue;
                else if (match(TokenType::CloseParen, ts))break;
                else throw std::runtime_error("expected ',' or '}'");
            }
        }

        variants.emplace_back(fieldName, fields);
        if (match(TokenType::Comma, ts))
        {
            if (match(TokenType::CloseBrace, ts))break;
            continue;
        }
        if (match(TokenType::CloseBrace, ts))break;
    }
    return make_unique<EnumNode>(std::move(iden), std::move(variants));
}

unique_ptr<StatementNode> parseDeclaration(TokenStream& ts)
{
    string parserName = "parseDeclaration";
    if constexpr (DEBUG_PARSER) debugEnter(parserName);
    unique_ptr<TypeNode> type = nullptr;
    match(TokenType::Let, ts);
    unique_ptr<ExpressionNode> left = parseEquality(ts);
    if (left->isDeclarationTarget())
    {
        if (match(TokenType::Colon, ts))
        {
            type = parseUnionType(ts);
        }
        if (match(TokenType::Assign, ts))
        {
            unique_ptr<ExpressionNode> expr = parseLogicalOr(ts);
            if constexpr (DEBUG_PARSER) debugExit(parserName);
            if (type)
                return make_unique<DeclarationNode>(std::move(left), std::move(expr), std::move(type),
                                                    ts.getLineNo());
            return make_unique<DeclarationNode>(std::move(left), std::move(expr),
                                                ts.getLineNo());
        }

        if constexpr (DEBUG_PARSER) debugExit(parserName);
        if (type)
        {
            return make_unique<DeclarationNode>(std::move(left), std::move(type), ts.getLineNo());
        }
        return make_unique<DeclarationNode>(std::move(left), ts.getLineNo());
    }
    throw ParserError(std::format("cannot declare to {}", left->description()), ParserError::SyntaxError,
                      ts.getLineNo());
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

    auto lhs = parseLogicalOr(ts);
    if (match(TokenType::Assign, ts))
    {
        if (lhs->isAssignmentTarget())
        {
            auto rhs = parseAssignment(ts);

            if constexpr (DEBUG_PARSER) debugExit(parserName);
            return make_unique<AssignmentNode>(std::move(lhs), std::move(rhs), ts.getLineNo());
        }
        throw ParserError(std::format("cannot assign to {}", lhs->description()), ParserError::SyntaxError,
                          ts.getLineNo());
    }
    if (match(TokenType::PlusEqual, ts))
    {
        if (lhs->isAssignmentTarget())
        {
            auto rhs = parseAssignment(ts);
            if constexpr (DEBUG_PARSER) debugExit(parserName);
            return make_unique<CompoundAssignmentNode>(Token{.type = TokenType::PlusEqual, .line = ts.getLineNo()},
                                                       std::move(lhs), std::move(rhs), ts.getLineNo());
        }
    }
    if (match(TokenType::MinusEqual, ts))
    {
        if (lhs->isAssignmentTarget())
        {
            auto rhs = parseAssignment(ts);
            if constexpr (DEBUG_PARSER) debugExit(parserName);
            return make_unique<CompoundAssignmentNode>(Token{.type = TokenType::MinusEqual, .line = ts.getLineNo()},
                                                       std::move(lhs), std::move(rhs), ts.getLineNo());
        }
    }
    if (match(TokenType::MultiplyEqual, ts))
    {
        if (lhs->isAssignmentTarget())
        {
            auto rhs = parseAssignment(ts);
            if constexpr (DEBUG_PARSER) debugExit(parserName);
            return make_unique<CompoundAssignmentNode>(Token{.type = TokenType::MultiplyEqual, .line = ts.getLineNo()},
                                                       std::move(lhs), std::move(rhs), ts.getLineNo());
        }
    }
    if (match(TokenType::DivideEqual, ts))
    {
        if (lhs->isAssignmentTarget())
        {
            auto rhs = parseAssignment(ts);
            if constexpr (DEBUG_PARSER) debugExit(parserName);
            return make_unique<CompoundAssignmentNode>(Token{.type = TokenType::DivideEqual, .line = ts.getLineNo()},
                                                       std::move(lhs), std::move(rhs), ts.getLineNo());
        }
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
    consume(TokenType::CloseParen, "expected ')' after condition expression", ts);

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
    consume(TokenType::CloseParen, "expected ')' after condition statement", ts);

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
            consume(TokenType::CloseParen, "expected ')' after for '('", ts);
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
        consume(TokenType::CloseParen, "expected ')' after for '('", ts);
        body = parseStatement(ts);
        if constexpr (DEBUG_PARSER) debugExit(parserName);
        --loopLevel;
        return make_unique<ForNode>(std::move(body), nullptr, std::move(cond), std::move(expr));
    }
    if (match(TokenType::Let, ts))
    {
        if (match(TokenType::OpenBracket, ts))
        {
            vector<string> identifiers;
            while (true)
            {
                Token t = consume(TokenType::Identifier, "expected identifier after let", ts);
                identifiers.push_back(t.name);
                if (match(TokenType::Comma, ts)) continue;
                if (match(TokenType::CloseBracket, ts)) break;
            }
            consume(TokenType::In, "expected in", ts);
            unique_ptr<ExpressionNode> containerExp = parsePostFix(ts);
            consume(TokenType::CloseParen, "expected ')' after for each '('", ts);
            body = parseStatement(ts);
            --loopLevel;
            return make_unique<ForEachNode>(identifiers, std::move(containerExp),
                                            std::move(body));
        }
        else
        {
            Token t = consume(TokenType::Identifier, "expected identifier after let", ts);

            if (match(TokenType::In, ts))
            {
                unique_ptr<ExpressionNode> containerExp = parsePostFix(ts);
                consume(TokenType::CloseParen, "expected ')' after for each '('", ts);
                body = parseStatement(ts);
                --loopLevel;
                std::vector identifiers{t.name};
                return make_unique<ForEachNode>(identifiers, std::move(containerExp),
                                                std::move(body));
            }
            consume(TokenType::Assign, "expected '=' in initialization of loop variable", ts);
            auto rval = parseEquality(ts);
            init = make_unique<DeclarationNode>(make_unique<VariableNode>(t.name, ts.getLineNo()), std::move(rval),
                                                ts.getLineNo());
        }
    }
    else init = parseExpressionStatement(ts);
    consume(TokenType::Semicolon, "expected semicolon in for", ts);
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
    consume(TokenType::Semicolon, "expected semicolon in for", ts);
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
    consume(TokenType::CloseBrace, "expected '}' at end of block to match '{'", ts);
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
    unique_ptr<TypeNode> type = nullptr;
    bool defArgsStarted{false};
    consume(TokenType::OpenParen, "expected '(' after function name for parameters", ts);
    vector<Param> parameters{};

    if (!match(TokenType::CloseParen, ts))
    {
        while (true)
        {
            Param parameter;
            if (check(TokenType::Identifier, ts))
            {
                Token ti = consume(TokenType::Identifier, "expected parameter", ts);
                parameter.identifier = ti.name;
                if (match(TokenType::Colon, ts))
                    parameter.type = parseUnionType(ts);
                else
                    parameter.type = nullptr;
                if (!defArgsStarted)
                {
                    if (match(TokenType::Assign, ts))
                    {
                        parameter.defaultArg = parseEquality(ts);
                        defArgsStarted = true;
                    }
                    else
                        parameter.defaultArg = nullptr;
                }
                else
                {
                    consume(TokenType::Assign,
                            "missing '='; default value is required for all parameters to the right after one default value",
                            ts);
                    parameter.defaultArg = parseEquality(ts);
                }
                parameters.push_back(std::move(parameter));
            }
            else if (check(TokenType::Ellipsis, ts))
            {
                parameter.isVariadic = true;
                match(TokenType::Ellipsis, ts);
                parameter.identifier = consume(TokenType::Identifier, "expected identifier after '...'", ts).name;
                if (match(TokenType::Colon, ts))
                    parameter.type = parseUnionType(ts);
                else parameter.type = nullptr;
                consume(TokenType::CloseParen, "...args parameter should come after with normal parameters", ts);
                parameter.defaultArg = make_unique<ArrayNode>();
                parameters.push_back(std::move(parameter));
                break;
            }
            if (match(TokenType::CloseParen, ts))
            {
                break;
            }
            match(TokenType::Comma, ts);
        }
    }
    if (match(TokenType::Arrow, ts))
    {
        type = parseUnionType(ts);
    }

    unique_ptr<StatementNode> body = parseBlock(ts);

    if constexpr (DEBUG_PARSER) debugExit(parserName);
    --functionLevel;
    if (type)
    {
        return make_unique<FunctionDeclarationNode>(t.name, std::move(parameters), std::move(body),
                                                    std::move(type),
                                                    funcDeclarationLine);
    }
    return make_unique<FunctionDeclarationNode>(t.name, std::move(parameters), std::move(body),
                                                funcDeclarationLine);
}

unique_ptr<ExpressionNode> parseLogicalOr(TokenStream& ts)
{
    string parserName = "parseLogicalOr";
    if constexpr (DEBUG_PARSER) debugEnter(parserName);
    auto lhs = parseLogicalAnd(ts);
    while (match(TokenType::OrOr, ts))
    {
        auto rhs = parseLogicalAnd(ts);
        lhs = make_unique<BinaryNode>(Token{.type = TokenType::OrOr, .line = ts.getLineNo()}, std::move(lhs),
                                      std::move(rhs), ts.getLineNo());
    }
    if constexpr (DEBUG_PARSER) debugExit(parserName);
    return lhs;
}

unique_ptr<ExpressionNode> parseLogicalAnd(TokenStream& ts)
{
    string parserName = "parseLogicalAnd";
    if constexpr (DEBUG_PARSER) debugEnter(parserName);
    auto lhs = parseEquality(ts);
    while (match(TokenType::AndAnd, ts))
    {
        auto rhs = parseEquality(ts);
        lhs = make_unique<BinaryNode>(Token{.type = TokenType::AndAnd, .line = ts.getLineNo()}, std::move(lhs),
                                      std::move(rhs), ts.getLineNo());
    }
    if constexpr (DEBUG_PARSER) debugExit(parserName);
    return lhs;
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

        if (t.type == TokenType::Plus || t.type == TokenType::Minus || t.type == TokenType::Modulo)
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

    if (check(TokenType::Plus, ts) || check(TokenType::Minus, ts) || check(TokenType::Not, ts))
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
    while (check(TokenType::OpenBracket, ts) || check(TokenType::OpenParen, ts) || check(TokenType::Dot, ts))
    {
        if (match(TokenType::OpenBracket, ts))
        {
            expr = make_unique<IndexNode>(std::move(expr), parseEquality(ts), ts.getLineNo());
            consume(TokenType::CloseBracket, "expected ']' after index expression", ts);
        }

        if (match(TokenType::Dot, ts))
        {
            expr = make_unique<MemberAccessNode>(std::move(expr), parsePrimary(ts), ts.getLineNo());
        }
        if (match(TokenType::OpenParen, ts))
        {
            if (match(TokenType::CloseParen, ts))
                expr = make_unique<FunctionCallNode>(std::move(expr), ts.getLineNo());
            else
            {
                vector<unique_ptr<ExpressionNode>> argumentNodes;
                while (true)
                {
                    argumentNodes.push_back(parseEquality(ts));

                    if (match(TokenType::CloseParen, ts))
                    {
                        expr = make_unique<FunctionCallNode>(std::move(expr),
                                                             std::move(argumentNodes), ts.getLineNo());
                        break;
                    }

                    if (match(TokenType::Comma, ts))
                    {
                        continue;
                    };
                }
            }
        }
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
        consume(TokenType::CloseBracket, "expected ']' to match '['" + string(1, static_cast<char>(ts.getLineNo())),
                ts);

        if constexpr (DEBUG_PARSER) debugExit(parserName);
        return make_unique<ArrayNode>(std::move(elements), ts.getLineNo());
    }
    if (t.type == TokenType::OpenParen)
    {
        unique_ptr<ExpressionNode> node = parseAssignment(ts);

        consume(TokenType::CloseParen, "expected ')' to match '('", ts);

        if constexpr (DEBUG_PARSER) debugExit(parserName);
        return node;
    }
    if (t.type == TokenType::Identifier)
    {
        string name = t.name;
        if constexpr (DEBUG_PARSER) debugExit(parserName);
        return make_unique<VariableNode>(name, ts.getLineNo());
    }
    if (t.type == TokenType::OpenBrace)
    {
        vector<Pair> pairs;
        while (true)
        {
            Token t3 = consume(TokenType::String, "expected string key", ts);
            consume(TokenType::Colon, "expected colon after key", ts);
            auto value = parsePrimary(ts);
            pairs.push_back({
                make_unique<StringNode>(std::get<string>(
                                            t3.literal), ts.getLineNo()),
                std::move(value)
            });
            if (match(TokenType::Comma, ts))
                continue;
            if (check(TokenType::CloseBrace, ts))
                break;
        }
        consume(TokenType::CloseBrace, "expected } for closing map", ts);
        return make_unique<MapNode>(std::move(pairs), ts.getLineNo());
    }
    if (t.type == TokenType::Match)
    {
        return parseMatchPattern(ts);
    }

    // Invalid Operand error
    throw ParserError("invalid primary" + getStringForType(t.type), ParserError::SyntaxError, ts.getLineNo());
}

auto parseMatchPattern(TokenStream& ts) -> unique_ptr<ExpressionNode>
{
    match(TokenType::Match, ts);
    bool hasParen{false};
    if (match(TokenType::OpenParen, ts)) hasParen = true;
    unique_ptr<ExpressionNode> scrutinee = parsePrimary(ts);
    if (hasParen)
        consume(TokenType::CloseParen, "expected ')' after match '(' scrutinee", ts);
    consume(TokenType::OpenBrace, "expected '{' for match", ts);
    vector<MatchArm> arms;
    bool defaultPresent{false};
    while (true)
    {
        auto pattern = parseOrPattern(ts);
        cout << pattern->type() << std::endl;
        if (pattern->type() == "wildcard")
        {
            defaultPresent = true;
            consume(TokenType::FatArrow, "expected '=>' after pattern", ts);
            if (check(TokenType::OpenBrace, ts))
            {
                auto expr = parseBlockExpression(ts);
                arms.push_back({std::move(pattern), std::move(expr)});
            }
            else
            {
                auto expr = parseEquality(ts);
                arms.push_back({std::move(pattern), std::move(expr)});
            }
            consume(TokenType::CloseBrace, "default must be at last", ts);
            break;
        }
        consume(TokenType::FatArrow, "expected '=>' after pattern", ts);
        if (check(TokenType::OpenBrace, ts))
        {
            auto expr = parseBlockExpression(ts);
            arms.push_back({std::move(pattern), std::move(expr)});
        }
        else
        {
            auto expr = parseEquality(ts);
            arms.push_back({std::move(pattern), std::move(expr)});
        }
        if (match(TokenType::Comma, ts)) continue;
        if (match(TokenType::CloseBrace, ts)) break;
    }
    if (!defaultPresent) throw std::runtime_error("default case missing");
    return make_unique<MatchPatternNode>(std::move(scrutinee), std::move(arms), ts.getLineNo());
}

auto parseBlockExpression(TokenStream& ts) -> unique_ptr<ExpressionNode>
{
    match(TokenType::OpenBrace, ts);
    vector<unique_ptr<StatementNode>> statements;
    while (true)
    {
        if (match(TokenType::Yield, ts))
        {
            auto resultExpr = parseLogicalOr(ts);
            consume(TokenType::Semicolon, "expected ';' after yield statement", ts);
            consume(TokenType::CloseBrace, "expected '}' at end of block to match '{'", ts);
            return make_unique<BlockExpressionNode>(std::move(statements), std::move(resultExpr));
            break;
        }
        statements.push_back(parseStatement(ts));
        if (check(TokenType::CloseBrace, ts)) break;
    }
    throw std::runtime_error("No yield statement in block expression");
}

auto parseOrPattern(TokenStream& ts) -> unique_ptr<PatternNode>
{
    vector<unique_ptr<PatternNode>> patterns;
    while (true)
    {
        patterns.push_back(parsePattern(ts));
        if (match(TokenType::Pipe, ts)) continue;
        if (check(TokenType::FatArrow, ts)) break;
    }
    if (patterns.size() == 1)
    {
        return std::move(patterns[0]);
    }
    return make_unique<OrPattern>(std::move(patterns));
}

auto parsePattern(TokenStream& ts) -> unique_ptr<PatternNode>
{
    Token t = ts.peek();
    if (t.type == TokenType::Number)
    {
        t = ts.getNextToken();
        cout << getStringForType(t.type) << "  " << getStringForType(ts.peek().type) << std::endl;
        if (match(TokenType::DotDot, ts))
        {
            Token t2 = ts.getNextToken();
            return make_unique<RangePattern>(RuntimeValue(std::get<double>(t.literal)),
                                             RuntimeValue(std::get<double>(t2.literal)), false);
        }
        if (match(TokenType::DotDotEqual, ts))
        {
            Token t2 = ts.getNextToken();
            return make_unique<RangePattern>(RuntimeValue(std::get<double>(t.literal)),
                                             RuntimeValue(std::get<double>(t2.literal)), true);
        }
        return make_unique<LiteralPattern>(RuntimeValue(std::get<double>(t.literal)));
    }
    if (t.type == TokenType::String)
    {
        t = ts.getNextToken();
        return make_unique<LiteralPattern>(RuntimeValue(std::get<string>(t.literal)));
    }
    if (t.type == TokenType::Boolean)
    {
        t = ts.getNextToken();
        return make_unique<LiteralPattern>(RuntimeValue(std::get<bool>(t.literal)));
    }
    if (t.type == TokenType::Null)
    {
        t = ts.getNextToken();
        return make_unique<LiteralPattern>(RuntimeValue());
    }
    if (t.type == TokenType::Identifier)
    {
        t = ts.getNextToken();
        if (t.name == "_")
            return make_unique<WildCardPattern>();
        return make_unique<IdentifierPattern>(t.name);
    }
    throw std::runtime_error("Invalid pattern" + getStringForType(t.type));
}

auto parseUnionType(TokenStream& ts) -> unique_ptr<TypeNode>
{
    vector<unique_ptr<TypeNode>> types;
    while (true)
    {
        // cout << getStringForType(ts.peek().type) << std::endl;

        types.push_back(parsePrimitiveType(ts));
        // cout << getStringForType(ts.peek().type) << std::endl;
        if (!match(TokenType::Pipe, ts)) break;
        // else break;
    }
    if (types.size() == 1)
    {
        return std::move(types[0]);
    }
    return make_unique<UnionType>(std::move(types));
}

auto parsePrimitiveType(TokenStream& ts) -> unique_ptr<TypeNode>
{
    if (check(TokenType::Identifier, ts))
    {
        Token t = consume(TokenType::Identifier, "expected type", ts);
        if (t.name == "number")
        {
            return make_unique<PrimitiveType>("number");
        }
        if (t.name == "str")
        {
            return make_unique<PrimitiveType>("str");
        }
        if (t.name == "bool")
        {
            return make_unique<PrimitiveType>("bool");
        }
        if (t.name == "Array")
        {
            consume(TokenType::OpenBracket, "expected '[' after Array type", ts);
            auto type = parseUnionType(ts);
            consume(TokenType::CloseBracket, "expected '[' to close ']' after Array type", ts);
            return make_unique<ArrayType>(std::move(type));
        }
        if (t.name == "Tuple")
        {
            consume(TokenType::OpenBracket, "expected '[' after Tuple type", ts);
            vector<unique_ptr<TypeNode>> types;
            while (true)
            {
                types.push_back(parseUnionType(ts));
                if (check(TokenType::CloseBracket, ts))break;
                else consume(TokenType::Comma, "expected ',' after type in Tuple", ts);
            }
            consume(TokenType::CloseBracket, "expected '[' to close ']' after Tuple type", ts);
            return make_unique<TupleType>(std::move(types));
        }
    }
    if (check(TokenType::Null, ts))
    {
        match(TokenType::Null, ts);
        return make_unique<PrimitiveType>("null");
    }

    throw std::runtime_error("Invalid type name");
}


