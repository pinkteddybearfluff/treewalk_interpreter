#ifndef INTERPRETER_LEXER_H
#define INTERPRETER_LEXER_H

#include <iostream>
#include <string>
#include <vector>

using std::string;
using std::vector;

//TODO: IMPLEMENT peekNext

enum class TokenType
{
    Number,
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
    End,
};

struct Token
{
    TokenType type;
    int value;
    std::string name;
};

class BufferStack
{
public:
    void push(const Token& t)
    {
        buffer.push_back(t);
    }

    Token back()
    {
        return buffer.back();
    }

    Token pop()
    {
        if (buffer.empty())
            throw std::runtime_error("Empty buffer");

        Token t = buffer.back();
        buffer.pop_back();
        return t;
    }

    bool isEmpty() const
    {
        return buffer.empty();
    }

private:
    std::vector<Token> buffer;
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

private:
    std::istream& is;
    Token buffer[2];
    int bufferCount{0};
    Token readFromStream();
    Token charToToken(char ch);

    string getVarName();
};

string getStringForType(TokenType type);

#endif //INTERPRETER_LEXER_H
