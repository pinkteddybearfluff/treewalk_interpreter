cmake_minimum_required(VERSION 4.1)
project(interpreter)

set(CMAKE_CXX_STANDARD 20)

add_executable(laven
        src/main.cpp
        src/lexer/Lexer.cpp
        src/lexer/Lexer.h
        src/parser/Parser.cpp
        src/parser/Parser.h
        src/ast/Ast.cpp
        src/ast/Ast.h
        src/runtime/Environment.cpp
        src/runtime/Environment.h
        src/Version.h
        src/stdlib/Stdlib.cpp
        src/stdlib/Stdlib.h
        src/runtime/RuntimeValue.cpp
        src/runtime/RuntimeValue.h
        src/error/RuntimeError.cpp
        src/error/RuntimeError.h
        src/utilities/Utilities.h
        src/utilities/Utilities.cpp
        src/utilities/Random.h)
