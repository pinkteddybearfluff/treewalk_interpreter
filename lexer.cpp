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
        if (ch == '\n')
        {
            ++lineNo;
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


    while (isspace(ch))
    {
        if (ch == '\n')
        {
            ++lineNo;
        }
        ch = is.get();
    }
    if (isdigit(ch))
    {
        is.unget();
        double value;
        is >> value;
        return Token{.type = TokenType::Number, .literal = value, .line = lineNo};
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
        return Token{.type = TokenType::String, .literal = str, .line = lineNo};
    }
    if (!is)
    {
        if (is.eof())
        {
            return Token{.type = TokenType::End, .line = lineNo};
        }
    }
    if (isalpha(ch))
    {
        is.unget();
        string name = getVarName();
        if (name == "if") return Token{.type = TokenType::If, .line = lineNo};
        if (name == "else") return Token{.type = TokenType::Else, .line = lineNo};
        if (name == "while") return Token{.type = TokenType::While, .line = lineNo};
        if (name == "let") return Token{.type = TokenType::Let, .line = lineNo};
        if (name == "fn") return Token{.type = TokenType::Function, .line = lineNo};
        if (name == "return") return Token{.type = TokenType::Return, .line = lineNo};
        if (name == "true") return Token{.type = TokenType::Boolean, .literal = true, .line = lineNo};
        if (name == "false") return Token{.type = TokenType::Boolean, .literal = false, .line = lineNo};
        if (name == "break") return Token{.type = TokenType::Break, .line = lineNo};
        if (name == "continue") return Token{.type = TokenType::Continue, .line = lineNo};
        if (name == "for") return Token{.type = TokenType::For, .line = lineNo};

        return Token{.type = TokenType::Identifier, .name = name, .line = lineNo};
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
        return Token{.type = TokenType::Plus, .line = lineNo};
    case '-':
        return Token{.type = TokenType::Minus, .line = lineNo};
    case '*':
        return Token{.type = TokenType::Multiply, .line = lineNo};
    case '/':
        return Token{.type = TokenType::Divide, .line = lineNo};
    case '(':
        return Token{.type = TokenType::OpenParen, .line = lineNo};
    case ')':
        return Token{.type = TokenType::CloseParen, .line = lineNo};
    case '{':
        return Token{.type = TokenType::OpenBrace, .line = lineNo};
    case '}':
        return Token{.type = TokenType::CloseBrace, .line = lineNo};
    case '[':
        return Token{.type = TokenType::OpenBracket, .line = lineNo};
    case ']':
        return Token{.type = TokenType::CloseBracket, .line = lineNo};
    case ';':
        return Token{.type = TokenType::Semicolon, .line = lineNo};
    case ',':
        return Token{.type = TokenType::Comma, .line = lineNo};
    case '=':
        {
            int ch2 = is.peek();
            if (static_cast<char>(ch2) == '=')
            {
                is.get();
                return Token{.type = TokenType::Equal, .line = lineNo};
            }
            return Token{.type = TokenType::Assign, .line = lineNo};
        }
    case '<':
        {
            int ch2 = is.peek();
            if (static_cast<char>(ch2) == '=')
            {
                char ch3;
                is >> ch3;
                return Token{.type = TokenType::LessEqual, .line = lineNo};
            }
            return Token{.type = TokenType::Less, .line = lineNo};
        }
    case '>':
        {
            int ch2 = is.peek();
            if (static_cast<char>(ch2) == '=')
            {
                is.get();
                return Token{.type = TokenType::GreaterEqual, .line = lineNo};
            }
            return Token{.type = TokenType::Greater, .line = lineNo};
        }
    case '!':
        {
            int ch2 = is.peek();
            if (static_cast<char>(ch2) == '=')
            {
                is.get();
                return Token{.type = TokenType::NotEqual, .line = lineNo};
            }
        }
    default:
        cout << static_cast<char>(ch) << std::endl;
        throw LexerError("invalid syntax " + string(1, ch), getLineNo());
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
    case TokenType::For:
        return "For";
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
