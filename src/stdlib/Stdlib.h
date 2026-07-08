//
// Created by wcosmo on 6/2/26.
//

#ifndef INTERPRETER_STDLIB_H
#define INTERPRETER_STDLIB_H

#include "../runtime/Environment.h"
#include "../lexer/Lexer.h"
#include "../parser/Parser.h"
#include "../ast/Ast.h"
#include "../utilities/Random.h"
#include <fstream>
#include <format>

void registerStdLib(Environment& env);
void registerStdLibMap(Environment& env);
void registerNativeFunctions(
    std::function<RuntimeValue(const vector<RuntimeValue>& args)> fn, string f_name, Environment& env);

unique_ptr<ProgramNode> loadStdlib(const string& file_name, InterpreterContext& ctx);

void importStdlibModule(const string& file_name, string alias, InterpreterContext& ctx);
void importBuiltinStdlib(const string& file_name, string alias, InterpreterContext& ctx);

namespace stdlib
{
    string stringify(const RuntimeValue& value);
    string numberToString(double number);
}

class Exit
{
};

class Help
{
};
#endif //INTERPRETER_STDLIB_H
