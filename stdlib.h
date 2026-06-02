//
// Created by wcosmo on 6/2/26.
//

#ifndef INTERPRETER_STDLIB_H
#define INTERPRETER_STDLIB_H

#include "stacks.h"
#include "lexer.h"
#include "parser.h"
#include "ast.h"
#include <iostream>
#include <fstream>
#include <format>

void registerStdLib(Environment& env);

unique_ptr<ProgramNode> loadStdlib(const string& file_name, shared_ptr<Environment> env);

#endif //INTERPRETER_STDLIB_H
