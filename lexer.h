#ifndef INTERPRETER_LEXER_H
#define INTERPRETER_LEXER_H

#include <iostream>
#include <string>
#include <vector>
#include "environment.h"

using std::string;
using std::vector;
using std::cout;

class LexerError : public std::runtime_error
{
public:
    LexerError(const string& msg, int lineNo) : std::runtime_error{msg}, line{lineNo}
    {
    };
    int line;
};

constexpr bool DEBUG_LEXER = false;

enum class TokenType
{
    Number,
    Equal,
    NotEqual,
    Greater,
    GreaterEqual,
    Less,
    LessEqual,
    Plus,
    Minus,
    Multiply,
    Divide,
    OpenParen,
    CloseParen,
    OpenBrace,
    CloseBrace,
    OpenBracket,
    CloseBracket,
    Comma,
    Identifier,
    Assign,
    Semicolon,
    If,
    Else,
    While,
    For,
    Let,
    End,
    String,
    Function,
    Return,
    Boolean,
    Null,
    Break,
    Continue,
    PlusEqual,
    MultiplyEqual,
    MinusEqual,
    DivideEqual,
    AndAnd,
    OrOr,
    Not,
    Modulo,
    Ellipsis,
    Import,
    Dot,
    As,
    Colon,
    Pipe,
    Arrow
};

struct Token
{
    TokenType type;
    std::variant<bool, double, string, std::monostate> literal;
    std::string name;
    int line;
};


class TokenStream
{
public:
    TokenStream(std::istream& istream) : is{istream}
    {
    };
    Token getNextToken();
    [[nodiscard]] Token getPrevious() const { return previous; }
    Token peek();
    Token peekNext();
    void debugPrintBuffer();
    int getLineNo() const { return lineNo; }

private:
    std::istream& is;
    Token buffer[2];
    Token previous;
    int bufferCount{0};
    int lineNo{1};
    Token readFromStream();
    Token charToToken(int ch);

    string getVarName();
    string getString();
    void consumeComments();
    void consumeMLComments();
};

string getStringForType(TokenType type);
string getSymbolForOp(TokenType op);

#endif //INTERPRETER_LEXER_H
