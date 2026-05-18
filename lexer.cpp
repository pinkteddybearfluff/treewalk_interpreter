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
    // std::cout << "Is buffer.empty in getNext " << bufferTokens.isEmpty() << std::endl;
    if (bufferCount == 2)
    {
        // std::cout << "[BUFFER -> CONSUMED] " << getStringForType(bufferedToken.type) << " Name: " << bufferedToken.name
        //     << " Value: " << bufferedToken.value << std::endl;
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
        // std::cout << "[STREAM -> NUMBER] " << " Value: " << value << std::endl;

        return Token{TokenType::Number, value};
    }
    if (!is)
    {
        if (is.eof())
        {
            // std::cout << "[STREAM -> EOF] " <<
            //     std::endl;

            return Token{TokenType::End};
        }
    }
    if (isalpha(ch))
    {
        is.putback(ch);
        string name = getVarName();
        // std::cout << "[STREAM -> CHAR] " << " Name: " << name << std::endl;

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
        return Token{TokenType::Assign};
    default:
        std::cout << ch << std::endl;
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
    }
}
