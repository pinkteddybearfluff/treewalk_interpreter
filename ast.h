#ifndef INTERPRETER_AST_H
#define INTERPRETER_AST_H

#include <memory>
#include <string>
#include <format>
#include <utility>
#include <cmath>
#include "stacks.h"

#include "lexer.h"

constexpr bool DEBUG_AST = true;

using std::unique_ptr;
using std::make_unique;
using std::string;

class FunctionDeclarationNode;
using FunctionTable = std::map<string, const FunctionDeclarationNode*>;

struct EvalResult
{
    bool hasValue;
    RuntimeValue value;
};

enum class ErrorCategory
{
    NameError, ZeroDivisionError, TypeError, RedeclarationError, IndexError, ArityError, ValueError, RecursionError
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
    MaxRecursionLimit
};

string getErrorCategoryString(ErrorCategory category);
string getErrorKindString(ErrorKind kind);

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
};

class UnsupportedOperation
{
};

class RuntimeError : public std::runtime_error
{
public:
    RuntimeError(const string& msg, Diagnostic diagnostic) : std::runtime_error{msg}, diagnostic{std::move(diagnostic)}
    {
    };

    Diagnostic diagnostic;
};

class StatementNode
{
public:
    virtual EvalResult evaluateNode(shared_ptr<Environment> env) const =0;
    virtual void debugPrint(int indentLevel) const = 0;
    virtual ~StatementNode() = default;
};


class ExpressionNode
{
public:
    virtual EvalResult evaluateNode(shared_ptr<Environment> env) const =0;
    virtual void debugPrint(int indentLevel) const =0;
    virtual ~ExpressionNode() = default;
    [[nodiscard]] virtual bool isAssignmentTarget() const { return false; };
    [[nodiscard]] virtual bool isDeclarationTarget() const { return false; };
    [[nodiscard]] virtual string description() const { return "Expression"; };

protected:
    int line{0};
};


class ProgramNode
{
public:
    EvalResult evaluateNode(shared_ptr<Environment> env);
    void debugPrint(int indentLevel);

    ProgramNode(vector<unique_ptr<StatementNode>> stmts) : statements{std::move(stmts)}
    {
    };

private:
    vector<unique_ptr<StatementNode>> statements;
};

class VariableNode : public ExpressionNode
{
public:
    VariableNode(std::string name, int line) : identifierName{name}, line{line}
    {
    };
    EvalResult evaluateNode(shared_ptr<Environment> env) const override;
    [[nodiscard]] const string& getIdentifierName() const { return identifierName; };
    RuntimeValue& getReference(shared_ptr<Environment> env);
    // RuntimeValue& getFuncReference(FunctionTable& function_table);
    void debugPrint(int indentLevel) const override;
    [[nodiscard]] bool isAssignmentTarget() const override { return true; };
    [[nodiscard]] bool isDeclarationTarget() const override { return true; };
    [[nodiscard]] string description() const override { return "variable"; };

private:
    string identifierName;

protected:
    int line;
};

class NumberNode : public ExpressionNode
{
public:
    NumberNode(double v, int lineNo) : value{v}, line{lineNo}
    {
    };
    EvalResult evaluateNode(shared_ptr<Environment> env) const override;
    void debugPrint(int indentLevel) const override;
    [[nodiscard]] bool isAssignmentTarget() const override { return false; };
    [[nodiscard]] bool isDeclarationTarget() const override { return false; };
    [[nodiscard]] string description() const override { return "literal"; };

private:
    double value;

protected:
    int line;
};

class StringNode : public ExpressionNode
{
public:
    StringNode(string str, int lineNo) : value{str}, line{lineNo}
    {
    };
    void debugPrint(int indentLevel) const override;
    EvalResult evaluateNode(shared_ptr<Environment> env) const override;
    [[nodiscard]] bool isDeclarationTarget() const override { return false; };
    [[nodiscard]] bool isAssignmentTarget() const override { return false; };
    [[nodiscard]] string description() const override { return "literal"; };

private:
    string value;

protected:
    int line;
};

class BooleanNode : public ExpressionNode
{
public:
    BooleanNode(bool tv, int lineNo) : value{tv}, line{lineNo}
    {
    };
    void debugPrint(int indentLevel) const override;
    EvalResult evaluateNode(shared_ptr<Environment> env) const override;
    [[nodiscard]] bool isDeclarationTarget() const override { return false; };
    [[nodiscard]] bool isAssignmentTarget() const override { return false; };
    [[nodiscard]] string description() const override { return "literal"; };

private:
    bool value;

protected:
    int line;
};


class NullNode : public ExpressionNode
{
public:
    NullNode(std::monostate null, int lineNo) : null{null}, line{lineNo}
    {
    };
    void debugPrint(int indentLevel) const override;
    EvalResult evaluateNode(shared_ptr<Environment> env) const override;
    [[nodiscard]] bool isDeclarationTarget() const override { return false; };
    [[nodiscard]] bool isAssignmentTarget() const override { return false; };
    [[nodiscard]] string description() const override { return "literal"; };

private:
    std::monostate null;

protected:
    int line;
};

class ArrayNode : public ExpressionNode
{
public:
    ArrayNode(vector<unique_ptr<ExpressionNode>> array, int lineNo) :
        value{std::move(array)}, line{lineNo}
    {
    };

    ArrayNode() : value{}
    {
    };
    void debugPrint(int indentLevel) const override;
    EvalResult evaluateNode(shared_ptr<Environment> env) const override;
    [[nodiscard]] bool isDeclarationTarget() const override { return false; };
    [[nodiscard]] bool isAssignmentTarget() const override { return false; };
    [[nodiscard]] string description() const override { return "literal"; };

private:
    vector<std::unique_ptr<ExpressionNode>> value;

protected:
    int line;
};

class IndexNode : public ExpressionNode
{
public:
    IndexNode(unique_ptr<ExpressionNode> Exp, unique_ptr<ExpressionNode> iExp, int lineNo) : operand{std::move(Exp)}
        ,
        indexExp{std::move(iExp)}, line{lineNo}
    {
    };
    void debugPrint(int indentLevel) const override;
    EvalResult evaluateNode(shared_ptr<Environment> env) const override;
    RuntimeValue& getReference(shared_ptr<Environment> env);
    // RuntimeValue getIndex(shared_ptr<Environment> env) const { return indexExp->evaluateNode(env).value; };
    [[nodiscard]] bool isDeclarationTarget() const override { return false; };
    [[nodiscard]] bool isAssignmentTarget() const override { return true; };
    [[nodiscard]] string description() const override { return "array subscript"; };

private:
    unique_ptr<ExpressionNode> operand;
    unique_ptr<ExpressionNode> indexExp;

protected:
    int line;
};


class BinaryNode : public ExpressionNode
{
public:
    BinaryNode(Token biOp, unique_ptr<ExpressionNode> leftNode, unique_ptr<ExpressionNode> rightNode, int lineNo)
        : op{biOp}, left{std::move(leftNode)}, right{std::move(rightNode)}, line{lineNo}
    {
    };
    EvalResult evaluateNode(shared_ptr<Environment> env) const override;
    void debugPrint(int indentLevel) const override;
    [[nodiscard]] bool isDeclarationTarget() const override { return false; };
    [[nodiscard]] bool isAssignmentTarget() const override { return false; };
    [[nodiscard]] string description() const override { return "literal"; };

private:
    Token op;
    unique_ptr<ExpressionNode> left;
    unique_ptr<ExpressionNode> right;

protected:
    int line;
};


class AssignmentNode : public ExpressionNode
{
public:
    AssignmentNode(unique_ptr<ExpressionNode> left, unique_ptr<ExpressionNode> right, int lineNo) :
        lvalue{std::move(left)}, rvalue{std::move(right)}, line{lineNo}
    {
    };
    EvalResult evaluateNode(shared_ptr<Environment> env) const override;
    void debugPrint(int indentLevel) const override;

    [[nodiscard]] bool isDeclarationTarget() const override { return false; };
    [[nodiscard]] bool isAssignmentTarget() const override { return true; };
    [[nodiscard]] string description() const override { return "literal"; };

private:
    unique_ptr<ExpressionNode> lvalue;
    unique_ptr<ExpressionNode> rvalue;

protected:
    int line;
};

class CompoundAssignmentNode : public ExpressionNode
{
public:
    CompoundAssignmentNode(Token op, unique_ptr<ExpressionNode> left, unique_ptr<ExpressionNode> right, int lineNo) :
        op{op}, lvalue{std::move(left)}, rvalue{std::move(right)}, line{lineNo}
    {
    };
    EvalResult evaluateNode(shared_ptr<Environment> env) const override;
    void debugPrint(int indentLevel) const override;

    [[nodiscard]] bool isDeclarationTarget() const override { return false; };
    [[nodiscard]] bool isAssignmentTarget() const override { return true; };
    [[nodiscard]] string description() const override { return "literal"; };

private:
    Token op;
    unique_ptr<ExpressionNode> lvalue;
    unique_ptr<ExpressionNode> rvalue;

protected:
    int line;
};


class FunctionCallNode : public ExpressionNode
{
public:
    FunctionCallNode(unique_ptr<ExpressionNode> var, int lineNo) : identifier{std::move(var)}, line{lineNo}
    {
    };

    FunctionCallNode(unique_ptr<ExpressionNode> var, vector<unique_ptr<ExpressionNode>> args, int lineNo) : identifier{
            std::move(var)
        }, arguments{std::move(args)}, line{lineNo}
    {
    };
    EvalResult evaluateNode(shared_ptr<Environment> env) const override;
    // vector<unique_ptr<ExpressionNode>>& getReference(EnvironmentStack& scopes) const;
    void debugPrint(int indentLevel) const override;

    [[nodiscard]] bool isDeclarationTarget() const override { return false; };
    [[nodiscard]] bool isAssignmentTarget() const override { return false; };
    [[nodiscard]] string description() const override { return "function call"; };

private
:
    unique_ptr<ExpressionNode> identifier;
    vector<unique_ptr<ExpressionNode>> arguments;

protected:
    int line;
};


class ExpressionStatementNode : public StatementNode
{
public:
    void debugPrint(int indentLevel) const override;
    EvalResult evaluateNode(shared_ptr<Environment> env) const override;

    ExpressionStatementNode(unique_ptr<ExpressionNode> expression) : expressionStmt{std::move(expression)}
    {
    };

private:
    unique_ptr<ExpressionNode> expressionStmt;
};

class DeclarationNode : public StatementNode
{
public:
    DeclarationNode(unique_ptr<ExpressionNode> left, unique_ptr<ExpressionNode> right, int lineNo) :
        lvalue{std::move(left)},
        rvalue{std::move(right)}, line{lineNo}
    {
    };

    DeclarationNode(unique_ptr<ExpressionNode> left, int lineNo) : lvalue{std::move(left)},
                                                                   rvalue(make_unique<NumberNode>(0, line)),
                                                                   line{lineNo}
    {
    };
    EvalResult evaluateNode(shared_ptr<Environment> env) const override;
    void debugPrint(int indentLevel) const override;

protected:
    int line;

private:
    unique_ptr<ExpressionNode> lvalue;
    unique_ptr<ExpressionNode> rvalue;
};


class IfNode : public StatementNode
{
public:
    IfNode(unique_ptr<ExpressionNode> c, unique_ptr<StatementNode> thenBranch) : condition{std::move(c)},
        thenStatement{std::move(thenBranch)}
    {
    };

    IfNode(unique_ptr<ExpressionNode> c, unique_ptr<StatementNode> thenBranch,
           unique_ptr<StatementNode> elseBranch)
        : condition{std::move(c)}, thenStatement{std::move(thenBranch)}, elseStatement{std::move(elseBranch)}
    {
    };
    EvalResult evaluateNode(shared_ptr<Environment> env) const override;
    void debugPrint(int indentLevel) const override;

private:
    unique_ptr<ExpressionNode> condition;
    unique_ptr<StatementNode> thenStatement;
    unique_ptr<StatementNode> elseStatement;
};

class WhileNode : public StatementNode
{
public:
    WhileNode(unique_ptr<ExpressionNode> c, unique_ptr<StatementNode> whileBranch) : condition{std::move(c)},
        statement{std::move(whileBranch)}
    {
    };
    EvalResult evaluateNode(shared_ptr<Environment> env) const override;
    void debugPrint(int indentLevel) const override;

private:
    unique_ptr<ExpressionNode> condition;
    unique_ptr<StatementNode> statement;
};

class ForNode : public StatementNode
{
public:
    ForNode(unique_ptr<StatementNode> stmt, unique_ptr<StatementNode> init = nullptr,
            unique_ptr<ExpressionNode> cond = nullptr, unique_ptr<ExpressionNode> expr = nullptr) :
        initializer{std::move(init)}, condition{std::move(cond)}, expr{std::move(expr)}, statement{std::move(stmt)}
    {
    };
    EvalResult evaluateNode(shared_ptr<Environment> env) const override;
    void debugPrint(int indentLevel) const override;

private:
    unique_ptr<StatementNode> initializer;
    unique_ptr<ExpressionNode> condition;
    unique_ptr<ExpressionNode> expr;
    unique_ptr<StatementNode> statement;
};

class BreakNode : public StatementNode
{
public:
    BreakNode()
    {
    }

    EvalResult evaluateNode(shared_ptr<Environment> env) const override;
    void debugPrint(int indentLevel) const override;
};

class BreakSignal
{
};

class ContinueNode : public StatementNode
{
public:
    ContinueNode()
    {
    };
    EvalResult evaluateNode(shared_ptr<Environment> env) const override;
    void debugPrint(int indentLevel) const override;
};

class ContinueSignal
{
};

class BlockNode : public StatementNode
{
public:
    BlockNode(vector<unique_ptr<StatementNode>> ss) : statements{std::move(ss)}
    {
    };
    void debugPrint(int indentLevel) const override;
    EvalResult evaluateNode(shared_ptr<Environment> env) const override;

private:
    vector<unique_ptr<StatementNode>> statements;
};

class UnaryNode : public ExpressionNode
{
public:
    UnaryNode(Token unOp, unique_ptr<ExpressionNode> n_node) : op{unOp}, child{std::move(n_node)}
    {
    };
    EvalResult evaluateNode(shared_ptr<Environment> env) const override;
    void debugPrint(int indentLevel) const override;

private:
    Token op;
    unique_ptr<ExpressionNode> child;
};

void validateArity(int expected_arguments, int given_arguments, string f_name, int callLine);

class FunctionDeclarationNode : public StatementNode
{
public:
    FunctionDeclarationNode(string fname, vector<string> para, unique_ptr<StatementNode> b, bool variadic,
                            string variadicParamName, int lineNo)
        : identifier{fname}, parameters{std::move(para)}, body{std::move(b)}, variadic{variadic},
          variadicParamName{variadicParamName}, line{lineNo}

    {
    };
    void debugPrint(int indentLevel) const override;
    EvalResult evaluateNode(shared_ptr<Environment> env) const override;
    int getParametersSize() const { return parameters.size(); };
    vector<string> getParameters() const { return parameters; };
    bool isVariadic() const { return variadic; };
    string variadicParamName;

private:
    string identifier;
    vector<string> parameters;
    unique_ptr<StatementNode> body;
    bool variadic{false};
    int line;
};


class ReturnNode : public StatementNode
{
public:
    ReturnNode(unique_ptr<ExpressionNode> statement = nullptr) : returnStatement(std::move(statement))
    {
    };
    void debugPrint(int indentLevel) const override;
    EvalResult evaluateNode(shared_ptr<Environment> env) const override;

private:
    unique_ptr<ExpressionNode> returnStatement;
};


class ReturnSignal
{
public:
    ReturnSignal(RuntimeValue val) : value{val}
    {
    };
    RuntimeValue value;
};

class EmptyNode : public StatementNode
{
public:
    void debugPrint(int indentLevel) const override;
    EvalResult evaluateNode(shared_ptr<Environment> env) const override;
};


#endif //INTERPRETER_AST_H
