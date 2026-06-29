#ifndef INTERPRETER_RUNTIMEERROR_H
#define INTERPRETER_RUNTIMEERROR_H

#include <string>
#include <stdexcept>
#include <iostream>
#include "utilities.h"

using std::string;


enum class ErrorCategory
{
    NameError, ZeroDivisionError, TypeError, RedeclarationError, IndexError, ArityError, ValueError, RecursionError,
    AttributeError, ImportError, UninitializedError
};


enum class ErrorKind
{
    //NameError
    VariableUndefined,
    FunctionUndefined,

    //TypeError
    InvalidIndexType,
    OperandTypeMismatch,
    UnsupportedOperation,
    NotCallable,
    NotSubscriptable,
    NativeFunInvalidOperandType,

    //ArityError
    TooFewArguments,
    TooManyArguments,

    //ZeroDivisionError
    DivisionByZero,

    //IndexError
    IndexOutOfBounds,

    //RedeclarationError
    VariableRedeclaration,
    FunctionRedeclaration,

    //RecursionError
    MaxRecursionLimit,

    //AttributeError
    MissingAttribute,
    InvalidReceiver,

    //ImportError
    ModuleNotFound,

    //UnintializedError
    UninitializedVariable
};

string getErrorCategoryString(ErrorCategory category);

struct Diagnostic
{
    ErrorCategory category;
    ErrorKind kind;

    string identifier;

    string primary;
    string secondary;

    int currentLine{-1};
    int previousLine{-1};

    int expected{-1};
    int actual{-1};
    bool variadic{false};
};

struct ArityDiagnostic
{
    string identifier;
    ErrorKind kind;
    int expected{-1};
    int actual{-1};
    bool variadic{false};
};

class RuntimeError : public std::runtime_error
{
public:
    RuntimeError(const string& msg, Diagnostic diagnostic, const std::vector<string>& stackTrace) :
        std::runtime_error{msg},
        diagnostic{std::move(diagnostic)}, stackTrace{stackTrace}
    {
    };

    Diagnostic diagnostic;
    std::vector<string> stackTrace;
};

void printRuntimeError(const RuntimeError& re, const string& file);


class UnsupportedOperation
{
};

class UndefinedVariable
{
};

class Redeclaration
{
};

class DivisionByZero
{
};

class UnintializedVariable
{
};

class InvalidType
{
};
#endif //INTERPRETER_RUNTIMEERROR_H
