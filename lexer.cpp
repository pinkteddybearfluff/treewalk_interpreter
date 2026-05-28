#include "lexer.h"

string TokenStream::getVarName()
{
    string name;
    int ch = is.get();
    // ch = is.get();
    if (isalpha(ch))
        while (true)
        {
            if (ch != EOF)
                name += static_cast<char>(ch);
            ch = is.get();
            if (!isalnum(ch))
            {
                is.unget();
                break;
            }
            if (!is)
            {
                if (is.eof()) throw std::runtime_error("Expected ;");
            }
            if (isspace(ch)) break;
        }
    return name;
}

void TokenStream::consumeComments()
{
    string str;
    int ch = is.get();
    while (ch != '\n' && ch != EOF)
    {
        str += static_cast<char>(ch);
        ch = is.get();
    }
    is.unget();
}

void TokenStream::consumeMLComments()
{
    string str;
    int ch = is.get();
    while (true)
    {
        str += static_cast<char>(ch);
        ch = is.get();
        if (ch == '*' && is.peek() == '/')
        {
            is.get();
            return;
        }
        if (ch == EOF)
        {
            is.unget();
            return;
        }
    }
    is.unget();
}

string TokenStream::getString()
{
    string str;
    int ch = is.get();
    while (ch != '"')
    {
        str += static_cast<char>(ch);
        ch = is.get();
    }
    is.unget();
    return str;
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
    int ch = is.get();
    while (isspace(ch))ch = is.get();
    if (isdigit(ch))
    {
        is.unget();
        double value;
        is >> value;
        return Token{TokenType::Number, value};
    }
    if (ch == '/')
    {
        if (is.peek() == '/')
        {
            is.get();
            consumeComments();
            return readFromStream();
        }
        if (is.peek() == '*')
        {
            is.get();
            consumeMLComments();
            return readFromStream();
        }
    }

    if (ch == '"')
    {
        string str;
        str = getString();
        if (is.get() != '"') throw std::runtime_error("expected '\"' for string termination");
        return Token{TokenType::String, str};
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
        is.unget();
        string name = getVarName();
        if (name == "if") return Token{TokenType::If};
        if (name == "else") return Token{TokenType::Else};
        if (name == "while") return Token{TokenType::While};
        if (name == "let") return Token{TokenType::Let};
        if (name == "fn") return Token{TokenType::Function};
        if (name == "return") return Token{TokenType::Return};
        if (name == "true") return Token{TokenType::Boolean, true};
        if (name == "false") return Token{TokenType::Boolean, false};
        if (name == "break") return Token{TokenType::Break};
        if (name == "continue") return Token{TokenType::Continue};

        return Token{TokenType::Identifier, 0.0, name};
    }
    return charToToken(ch);
}


Token TokenStream::peek()
{
    if constexpr (DEBUG_LEXER) cout << "Enter peek\n";
    if (bufferCount == 0)
    {
        if constexpr (DEBUG_LEXER) debugPrintBuffer();
        buffer[0] = readFromStream();
        ++bufferCount;
        if constexpr (DEBUG_LEXER)
        {
            debugPrintBuffer();
            cout << "Exit peek" << std::endl;
        }
        return buffer[0];
    }
    if (bufferCount == 1)
    {
        if constexpr (DEBUG_LEXER)
        {
            debugPrintBuffer();
            cout << "Exit peek" << std::endl;
        }
        return buffer[0];
    }
    if (bufferCount == 2)
    {
        if constexpr (DEBUG_LEXER)
        {
            debugPrintBuffer();
            cout << "Exit peek" << std::endl;
        }
        return buffer[0];
    }
}

Token TokenStream::peekNext()
{
    if constexpr (DEBUG_LEXER) cout << "Enter peekNext\n";

    if (bufferCount == 0)
    {
        if constexpr (DEBUG_LEXER) debugPrintBuffer();
        buffer[0] = readFromStream();
        buffer[1] = readFromStream();
        bufferCount = 2;
        if constexpr (DEBUG_LEXER) debugPrintBuffer();
    }
    if (bufferCount == 1)
    {
        if constexpr (DEBUG_LEXER) debugPrintBuffer();
        buffer[1] = readFromStream();
        ++bufferCount;
        if constexpr (DEBUG_LEXER) debugPrintBuffer();
    }

    if constexpr (DEBUG_LEXER) cout << "Exit peekNext\n";
    return buffer[1];
}

Token TokenStream::charToToken(int ch)
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
    case '{':
        return Token{TokenType::OpenBrace};
    case '}':
        return Token{TokenType::CloseBrace};
    case '[':
        return Token{TokenType::OpenBracket};
    case ']':
        return Token{TokenType::CloseBracket};
    case ';':
        return Token{TokenType::Semicolon};
    case ',':
        return Token{TokenType::Comma};
    case '=':
        {
            int ch2 = is.peek();
            if (static_cast<char>(ch2) == '=')
            {
                is.get();
                return Token{TokenType::Equal};
            }
            return Token{TokenType::Assign};
        }
    case '<':
        {
            int ch2 = is.peek();
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
                is.get();
                return Token{TokenType::GreaterEqual};
            }
            return Token{TokenType::Greater};
        }
    case '!':
        {
            int ch2 = is.peek();
            if (static_cast<char>(ch2) == '=')
            {
                is.get();
                return Token{TokenType::NotEqual};
            }
        }
    default:
        cout << static_cast<char>(ch) << std::endl;
        throw std::runtime_error("Unknown token!");
    }
}


void TokenStream::debugPrintBuffer()
{
    cout << "Buffer count = " << bufferCount << '\n';
    if (bufferCount == 0) return;
    if (bufferCount == 1)
    {
        cout << "[Buffer: 0] = [" << getStringForType(buffer[0].type) << "]" << std::endl;
    }
    if (bufferCount == 2)
    {
        cout << "[Buffer: 0] = [" << getStringForType(buffer[0].type) << "]\n";

        cout << "[Buffer: 1] = [" << getStringForType(buffer[1].type) << "]" << std::endl;
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
    case TokenType::Else:
        return "Else";
    case TokenType::OpenBrace:
        return "OpenBrace";
    case TokenType::CloseBrace:
        return "CloseBrace";
    case TokenType::While:
        return "While";
    case TokenType::Let:
        return "Let";
    case TokenType::String:
        return "String";
    case TokenType::Function:
        return "Function";
    case TokenType::Return:
        return "Return";
    case TokenType::Boolean:
        return "Boolean";
    case TokenType::Break:
        return "Break";
    case TokenType::Continue:
        return "Continue";
    case TokenType::OpenBracket:
        return "OpenBracket";
    case TokenType::CloseBracket:
        return "CloseBracket";
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
