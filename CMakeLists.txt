cmake_minimum_required(VERSION 4.1)
project(interpreter)

set(CMAKE_CXX_STANDARD 20)

add_executable(interpreter
        main.cpp
        lexer.cpp
        lexer.h
        parser.cpp
        parser.h
        ast.cpp
        ast.h
        stacks.cpp
        stacks.h)
