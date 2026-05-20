#ifndef INTERPRETER_LEXER_H
#define INTERPRETER_LEXER_H

#include <iostream>
#include <string>
#include <vector>

using std::string;
using std::vector;
using std::cout;

constexpr bool DEBUG_LEXER = false;

//TODO: IMPLEMENT peekNext

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
    Comma,
    Identifier,
    Assign,
    Semicolon,
    If,
    End,
};

struct Token
{
    TokenType type;
    int value;
    std::string name;
};


class TokenStream
{
public:
    TokenStream(std::istream& istream) : is{istream}
    {
    };
    Token getNextToken();
    Token peek();
    Token peekNext();
    void debugPrintBuffer();

private:
    std::istream& is;
    Token buffer[2];
    int bufferCount{0};
    Token readFromStream();
    Token charToToken(char ch);

    string getVarName();
};

string getStringForType(TokenType type);
string getSymbolForOp(TokenType op);

#endif //INTERPRETER_LEXER_H
