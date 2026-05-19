#ifndef INTERPRETER_PARSER_H
#define INTERPRETER_PARSER_H

#include "ast.h"
#include "lexer.h"
#include <string_view>


inline int parserDepth = 0;
constexpr bool DEBUG_PARSER = true;


void debugConsume(std::string_view parserName, const Token& t);
void debugEnter(std::string_view parserName);
void debugPeek(std::string_view parserName, const Token& t);
void debugNextPeek(std::string_view parserName, const Token& t);
void debugExit(std::string_view parserName);


unique_ptr<ExpressionNode> parseStatement(TokenStream& ts);
unique_ptr<ExpressionNode> parseEquality(TokenStream& ts);
unique_ptr<ExpressionNode> parseComparison(TokenStream& ts);
unique_ptr<ExpressionNode> parseExpression(TokenStream& ts);
unique_ptr<ExpressionNode> parseTerm(TokenStream& ts);
unique_ptr<ExpressionNode> parseFactor(TokenStream& ts);


#endif //INTERPRETER_PARSER_H
