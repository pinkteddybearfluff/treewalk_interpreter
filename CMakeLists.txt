cmake_minimum_required(VERSION 4.1)
project(interpreter)

set(CMAKE_CXX_STANDARD 17)

add_executable(interpreter
        main.cpp
        lexer.cpp
        lexer.h
        parser.cpp
        parser.h
        ast.cpp
        ast.h
        stacks.cpp
        stacks.cpp
        stacks.h)
