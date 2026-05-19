#include "lexer.h"

string TokenStream::getVarName()
{
    string name;
    char ch;
    is >> ch;
    if (isalpha(ch))
        while (true)
        {
            name += ch;
            is >> ch;
            if (!isalnum(ch))
            {
                is.putback(ch);
                break;
            }
            if (!is)
            {
                if (is.eof()) throw std::runtime_error("Expected ;");
            }
        }
    return name;
}

Token TokenStream::getNextToken()
{
    if (bufferCount == 2)
    {
        --bufferCount;
        Token t = buffer[0];
        buffer[0] = buffer[1];
        return t;
    }
    if (bufferCount == 1)
    {
        --bufferCount;
        return buffer[0];
    }
    return readFromStream();
}

Token TokenStream::readFromStream()
{
    char ch;
    is >> ch;
    if (isdigit(ch))
    {
        is.putback(ch);
        int value;
        is >> value;

        return Token{TokenType::Number, value};
    }
    if (!is)
    {
        if (is.eof())
        {
            return Token{TokenType::End};
        }
    }
    if (isalpha(ch))
    {
        is.putback(ch);
        string name = getVarName();
        if (name == "if") return Token{TokenType::If};

        return Token{TokenType::Identifier, 0, name};
    }
    return charToToken(ch);
}


Token TokenStream::peek()
{
    if (bufferCount == 0)
    {
        buffer[0] = readFromStream();
        ++bufferCount;
        return buffer[0];
    }
    if (bufferCount == 1)
    {
        return buffer[0];
    }
    if (bufferCount == 2)
    {
        return buffer[1];
    }
}

Token TokenStream::peekNext()
{
    if (bufferCount == 0)
    {
        buffer[0] = readFromStream();
        buffer[1] = readFromStream();
        bufferCount = 2;
    }
    if (bufferCount == 1)
    {
        buffer[1] = readFromStream();
        ++bufferCount;
    }
    return buffer[1];
}

Token TokenStream::charToToken(char ch)
{
    switch (ch)
    {
    case '+':
        return Token{TokenType::Plus};
    case '-':
        return Token{TokenType::Minus};
    case '*':
        return Token{TokenType::Multiply};
    case '/':
        return Token{TokenType::Divide};
    case '(':
        return Token{TokenType::OpenParen};
    case ')':
        return Token{TokenType::CloseParen};
    case ';':
        return Token{TokenType::Semicolon};
    case ',':
        return Token{TokenType::Comma};
    case '=':
        {
            int ch2 = is.peek();
            if (static_cast<char>(ch2) == '=')
            {
                char ch3;
                is >> ch3;
                return Token{TokenType::Equal};
            }
            return Token{TokenType::Assign};
        }
    case '<':
        {
            int ch2 = is.peek();
            // std::cout << getStringForType(t.type) << std::endl;
            if (static_cast<char>(ch2) == '=')
            {
                char ch3;
                is >> ch3;
                return Token{TokenType::LessEqual};
            }
            return Token{TokenType::Less};
        }
    case '>':
        {
            int ch2 = is.peek();

            if (static_cast<char>(ch2) == '=')
            {
                char ch3;
                is >> ch3;
                return Token{TokenType::GreaterEqual};
            }
            return Token{TokenType::Greater};
        }
    case '!':
        {
            int ch2 = is.peek();
            if (static_cast<char>(ch2) == '=')
            {
                char ch3;
                is >> ch3;
                return Token{TokenType::NotEqual};
            }
        }
    default:
        throw std::runtime_error("Unknown token!");
    }
}


string getStringForType(TokenType type)
{
    switch (type)
    {
    case TokenType::Assign:
        return "ASSIGN";
    case TokenType::Number:
        return "NUMBER";
    case TokenType::Plus:
        return "PLUS";
    case TokenType::Minus:
        return "MINUS";
    case TokenType::Multiply:
        return "MULTIPLY";
    case TokenType::Divide:
        return "DIVIDE";
    case TokenType::OpenParen:
        return "OPENPAREN";
    case TokenType::CloseParen:
        return "CLOSEPAREN";
    case TokenType::Identifier:
        return "VARIABLE";
    case TokenType::Semicolon:
        return "SEMICOLON";
    case TokenType::End:
        return "EOF";
    case TokenType::Comma:
        return "COMMA";
    case TokenType::Equal:
        return "Equal";
    case TokenType::NotEqual:
        return "NotEqual";
    case TokenType::Greater:
        return "Greater";
    case TokenType::GreaterEqual:
        return "GreaterEqual";
    case TokenType::Less:
        return "Less";
    case TokenType::LessEqual:
        return "LessEqual";
    case TokenType::If:
        return "If";
    }
}


string getSymbolForOp(TokenType op)
{
    switch (op)
    {
    case TokenType::Equal:
        return "==";
    case TokenType::NotEqual:
        return "!=";
    case TokenType::Greater:
        return ">";
    case TokenType::GreaterEqual:
        return ">=";
    case TokenType::Less:
        return "<";
    case TokenType::LessEqual:
        return "<=";
    case TokenType::Plus:
        return "+";
    case TokenType::Minus:
        return "-";
    case TokenType::Multiply:
        return "*";
    case TokenType::Divide:
        return "/";
    case TokenType::Assign:
        return "=";
    }
}
