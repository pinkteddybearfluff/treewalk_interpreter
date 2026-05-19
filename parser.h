#ifndef INTERPRETER_PARSER_H
#define INTERPRETER_PARSER_H

#include "ast.h"
#include "lexer.h"

unique_ptr<ExpressionNode> parseStatement(TokenStream& ts);
unique_ptr<ExpressionNode> parseEquality(TokenStream& ts);
unique_ptr<ExpressionNode> parseComparison(TokenStream& ts);
unique_ptr<ExpressionNode> parseExpression(TokenStream& ts);
unique_ptr<ExpressionNode> parseTerm(TokenStream& ts);
unique_ptr<ExpressionNode> parseFactor(TokenStream& ts);


#endif //INTERPRETER_PARSER_H
