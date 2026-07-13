#ifndef INTERPRETER_RUNTIMEERROR_H
#define INTERPRETER_RUNTIMEERROR_H

#include <string>
#include <stdexcept>
#include <iostream>
#include "../utilities/Utilities.h"
// #include "../visitors/EvaluateVisitor.h"

using std::string;

struct StackFrame;

enum class ErrorCategory
{
    NameError, ZeroDivisionError, TypeError, RedeclarationError, IndexError, ArityError, ValueError, RecursionError,
    AttributeError, ImportError, UninitializedError, AssertionError, PanicError,
    AlreadyExistsError, KeyError,
};


enum class ErrorCode
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
    InvalidLeftOperandForIs,
    InvalidRightOperandForIs,

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
    StructMemberNotFound,
    EnumMemberNotFound,
    EnumVariantNotFound,
    ObjectMethodNotFound,

    //ImportError
    ModuleNotFound,

    //UnintializedError
    UninitializedVariable,

    //AssertionError
    AssertionFailure,

    //PanicError
    PanicAbort,

    //AlreadyExistsError
    DuplicateMapKey,

    //KeyError
    MapKeyNotFound,
};

string getErrorCategoryString(ErrorCategory category);

struct Diagnostic
{
    ErrorCategory category;
    ErrorCode code;

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
    ErrorCode kind;
    int expected{-1};
    int actual{-1};
    bool variadic{false};
};

class RuntimeError : public std::runtime_error
{
public:
    RuntimeError(const string& msg, Diagnostic diagnostic, const std::vector<StackFrame>& stackTrace);

    Diagnostic diagnostic;
    std::vector<StackFrame> stackTrace;
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

class AssertionError
{
};

class PanicError
{
public:
    string msg;
};
#endif //INTERPRETER_RUNTIMEERROR_H
