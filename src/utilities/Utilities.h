#ifndef INTERPRETER_UTILITIES_H
#define INTERPRETER_UTILITIES_H

#include <string>
#include <iostream>
#include <vector>

namespace color
{
    constexpr const char* reset = "\033[0m";

    constexpr const char* black = "\033[30m";
    constexpr const char* boldBlack = "\033[1;30m";

    constexpr const char* red = "\033[31m";
    constexpr const char* boldRed = "\033[1;31m";

    constexpr const char* green = "\033[32m";
    constexpr const char* boldGreen = "\033[1;32m";

    constexpr const char* yellow = "\033[33m";
    constexpr const char* boldYellow = "\033[1;33m";

    constexpr const char* blue = "\033[34m";
    constexpr const char* boldBlue = "\033[1;34m";

    constexpr const char* magenta = "\033[35m";
    constexpr const char* boldMagenta = "\033[1;35m";

    constexpr const char* cyan = "\033[36m";
    constexpr const char* boldCyan = "\033[1;36m";

    constexpr const char* white = "\033[37m";
    constexpr const char* boldWhite = "\033[1;37m";
}

using std::cin;

std::string readREPLinput();

std::string getFolder(const std::string& string);


#endif //INTERPRETER_UTILITIES_H
