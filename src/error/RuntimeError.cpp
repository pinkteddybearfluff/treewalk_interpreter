#include "RuntimeError.h"
#include "../visitors/EvaluateVisitor.h"

void printRuntimeError(const RuntimeError& re, const string& file)
{
    if (file != "repl")
        std::cerr << "File " << color::magenta << "\"" << file << "\"" << color::reset << ", line " << color::boldBlue
            << re.diagnostic.currentLine << color::reset << '\n';
    std::cerr << "stack trace:\n";
    for (const auto& func : re.stackTrace)
    {
        std::cerr << "\t\tat " << color::boldWhite << func.function_name << color::reset << "()\n";
    }
    std::cerr << color::boldRed << getErrorCategoryString(re.diagnostic.category) << ": " << color::reset;

    switch (re.diagnostic.code)
    {
    case ErrorCode::VariableUndefined:
    case ErrorCode::FunctionUndefined:
        std::cerr << "name '" << color::boldWhite << re.diagnostic.identifier << color::reset << "'" <<
            " is not defined"
            << "\n";
        break;
    case ErrorCode::InvalidIndexType:
        std::cerr << re.diagnostic.identifier << " indices must be " << color::boldWhite << re.diagnostic.primary <<
            color::reset <<
            ", not " << color::boldWhite << re.diagnostic.secondary << color::reset << "\n";
        break;
    case ErrorCode::UnsupportedOperation:
        std::cerr << "unsupported operand type(s) for "
            << color::boldWhite << re.diagnostic.identifier << color::reset << ": '" << color::boldGreen << re.
            diagnostic.primary << color::reset << "' and '"
            << color::boldBlue << re.diagnostic.secondary << color::reset << "'\n";
        break;
    case ErrorCode::NotSubscriptable:
        std::cerr << "'" << color::boldWhite << re.diagnostic.primary << color::reset <<
            "' object is not subscriptable\n";
        break;
    case ErrorCode::NotCallable:
        std::cerr << "'" << color::boldWhite << re.diagnostic.primary << color::reset <<
            "' object is not callable\n";
        break;
    case ErrorCode::TooManyArguments:
        std::cerr << "too many arguments to function '" << color::boldGreen << re.diagnostic.identifier <<
            color::reset <<
            "()' expected " << color::boldBlue
            << re.diagnostic.expected << color::reset << " have " << color::boldRed << re.diagnostic.actual <<
            color::reset << '\n';
        break;
    case ErrorCode::TooFewArguments:

        std::cerr << "too few arguments to function '" << color::boldGreen << re.diagnostic.identifier <<
            color::reset <<
            "()'" << ((re.diagnostic.variadic) ? " expected at least " : " expected ") << color::boldBlue
            << re.diagnostic.expected << color::reset << " have " << color::boldRed << re.diagnostic.actual <<
            color::reset << '\n';
        break;
    case ErrorCode::IndexOutOfBounds:
        std::cerr << re.diagnostic.primary << " index is out of range\n";
        break;
    case ErrorCode::DivisionByZero:
        std::cerr << "division by zero\n";
        break;
    case ErrorCode::VariableRedeclaration:
        std::cerr << "redeclaration of '" << color::boldWhite << re.diagnostic.identifier << color::reset << "'\n";
        std::cerr << color::boldCyan << "Note: " << color::reset << "'" << color::boldWhite << re.diagnostic.
            identifier << color::reset <<
            "' previously declared on line " << color::boldBlue << re.diagnostic.previousLine << color::reset <<
            "\n";
        break;
    case ErrorCode::FunctionRedeclaration:
        std::cerr << "redefinition of '" << color::boldGreen << re.diagnostic.identifier << color::reset << "()'\n";
        break;
    case ErrorCode::MaxRecursionLimit:
        std::cerr << "maximum recursion limit reached\n";
        break;
    case ErrorCode::OperandTypeMismatch:
        std::cerr << "bad operand type for unary " << re.diagnostic.identifier << ": '" << re.diagnostic.primary <<
            "'\n";
        break;
    case ErrorCode::MissingAttribute:
        std::cerr << "module '" << color::boldBlue << re.diagnostic.primary << color::reset << "' has no attribute '" <<
            color::boldGreen << re.diagnostic.secondary << color::reset << "'\n";
        break;
    case ErrorCode::InvalidReceiver:
        std::cerr << "type '" << re.diagnostic.identifier << "' does not support member access\n";
        break;
    case ErrorCode::ModuleNotFound:
        if (re.diagnostic.primary == "std")
        {
            std::cerr << "no standard library module named '" << color::boldBlue << re.diagnostic.identifier <<
                color::reset << "'\n";
        }
        else
        {
            std::cerr << "module not found at'" << color::magenta << re.diagnostic.identifier << color::reset << "' \n";
        }
        break;
    case ErrorCode::UninitializedVariable:
        std::cerr << "use of variable '" << re.diagnostic.identifier << "' before initialization\n";
        break;
    case ErrorCode::AssertionFailure:
        std::cerr << "\n";
        break;
    case ErrorCode::PanicAbort:
        std::cerr << re.diagnostic.identifier << "\n";
        break;
    }
}


string getErrorCategoryString(ErrorCategory category)
{
    switch (category)
    {
    case ErrorCategory::NameError:
        return "NameError";
    case ErrorCategory::ZeroDivisionError:
        return "ZeroDivisionError";
    case ErrorCategory::TypeError:
        return "TypeError";
    case ErrorCategory::RedeclarationError:
        return "RedeclarationError";
    case ErrorCategory::ArityError:
        return "ArityError";
    case ErrorCategory::ValueError:
        return "ValueError";
    case ErrorCategory::IndexError:
        return "IndexError";
    case ErrorCategory::RecursionError:
        return "RecursionError";
    case ErrorCategory::AttributeError:
        return "AttributeError";
    case ErrorCategory::ImportError:
        return "ImportError";
    case ErrorCategory::UninitializedError:
        return "UninitializedError";
    case ErrorCategory::AssertionError:
        return "AssertionError";
    case ErrorCategory::PanicError:
        return "PanicError";
    }
}

RuntimeError::RuntimeError(const string& msg, Diagnostic diagnostic, const std::vector<StackFrame>& stackTrace) :
    std::runtime_error{msg},
    diagnostic{std::move(diagnostic)}, stackTrace{stackTrace}
{
};
