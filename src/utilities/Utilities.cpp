#include "Utilities.h"

std::string readREPLinput()
{
    std::string input;
    std::vector<char> stack;
    while (true)
    {
        int ch = cin.get();

        if (ch != '\n')
            input += std::string(1, static_cast<char>(ch));
        if (ch == '{' || ch == '(' || ch == '[')
        {
            stack.push_back(static_cast<char>(ch));
        }
        if (ch == '}')
        {
            if (stack.back() == '{') stack.pop_back();
            else break;
        }
        if (ch == ')')
        {
            if (stack.back() == '(') stack.pop_back();
            else break;
        }
        if (ch == ']')
        {
            if (stack.back() == '[') stack.pop_back();
            else break;
        }
        if (ch == '\n')
        {
            if (stack.empty()) break;
        }
    }
    return input;
}

std::string getFolder(const std::string& string)
{
    int i = string.rfind('/');
    return string.substr(0, i);
}
