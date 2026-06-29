#ifndef INTERPRETER_PARSER_H
#define INTERPRETER_PARSER_H

#include "ast.h"
#include "lexer.h"
#include <string_view>


constexpr bool DEBUG_PARSER = false;


class ParserError : public std::runtime_error
{
public:
    enum Category { SyntaxError, SemanticError };

    ParserError(const string& msg, Category category, int lineNo) : std::runtime_error{msg}, cat{category}, line{lineNo}
    {
    };

    string getCatStr() const
    {
        switch (cat)
        {
        case SyntaxError: return "SyntaxError";
        case SemanticError: return "SemanticError";
        }
    }

    Category cat;
    int line;
};

bool match(TokenType tkType, TokenStream& ts);
bool check(TokenType tkType, TokenStream& ts);
Token consume(TokenType tkType, string msg, TokenStream& ts);


void debugConsume(std::string_view parserName, const Token& t);
void debugEnter(std::string_view parserName);
void debugPeek(std::string_view parserName, const Token& t);
void debugNextPeek(std::string_view parserName, const Token& t);
void debugExit(std::string_view parserName);

auto parseUnionType(TokenStream& ts) -> unique_ptr<TypeNode>;
auto parsePrimitiveType(TokenStream& ts) -> unique_ptr<TypeNode>;

auto parseStatement(TokenStream& ts) -> unique_ptr<StatementNode>;

auto parseImportStatement(TokenStream& ts) -> unique_ptr<StatementNode>;
auto parseTryStatement(TokenStream& ts) -> unique_ptr<StatementNode>;
auto parseIfStatement(TokenStream& ts) -> unique_ptr<StatementNode>;
auto parseWhileStatement(TokenStream& ts) -> unique_ptr<StatementNode>;
auto parseForStatement(TokenStream& ts) -> unique_ptr<StatementNode>;

auto parseBlock(TokenStream& ts) -> unique_ptr<StatementNode>;
auto parseFunctionDeclaration(TokenStream& ts) -> unique_ptr<StatementNode>;
auto parseDeclaration(TokenStream& ts) -> unique_ptr<StatementNode>;
auto parseStruct(TokenStream& ts) -> unique_ptr<StatementNode>;
auto parseEnum(TokenStream& ts) -> unique_ptr<StatementNode>;
auto parseExpressionStatement(TokenStream& ts) -> unique_ptr<StatementNode>;

auto parseAssignment(TokenStream& ts) -> unique_ptr<ExpressionNode>;
auto parseLogicalOr(TokenStream& ts) -> unique_ptr<ExpressionNode>;
auto parseLogicalAnd(TokenStream& ts) -> unique_ptr<ExpressionNode>;
auto parseEquality(TokenStream& ts) -> unique_ptr<ExpressionNode>;
auto parseComparison(TokenStream& ts) -> unique_ptr<ExpressionNode>;
auto parseTerm(TokenStream& ts) -> unique_ptr<ExpressionNode>;
auto parseFactor(TokenStream& ts) -> unique_ptr<ExpressionNode>;
auto parseUnary(TokenStream& ts) -> unique_ptr<ExpressionNode>;
auto parsePrimary(TokenStream& ts) -> unique_ptr<ExpressionNode>;
auto parseMatchPattern(TokenStream& ts) -> unique_ptr<ExpressionNode>;
auto parseBlockExpression(TokenStream& ts) -> unique_ptr<ExpressionNode>;
auto parseOrPattern(TokenStream& ts) -> unique_ptr<PatternNode>;
auto parsePattern(TokenStream& ts) -> unique_ptr<PatternNode>;
auto parsePostFix(TokenStream& ts) -> unique_ptr<ExpressionNode>;


#endif //INTERPRETER_PARSER_H
