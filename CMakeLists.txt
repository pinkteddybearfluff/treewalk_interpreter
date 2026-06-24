cmake_minimum_required(VERSION 4.1)
project(interpreter)

set(CMAKE_CXX_STANDARD 20)

add_executable(laven
        main.cpp
        lexer.cpp
        lexer.h
        parser.cpp
        parser.h
        ast.cpp
        ast.h
        environment.cpp
        environment.h
        version.h
        stdlib.cpp
        stdlib.h
        RuntimeValue.cpp
        RuntimeValue.h
        RuntimeError.cpp
        RuntimeError.h
        utilities.h
        utilities.cpp
        secrets.h
        Random.h)
