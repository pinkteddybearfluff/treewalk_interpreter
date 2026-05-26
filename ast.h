#ifndef INTERPRETER_AST_H
#define INTERPRETER_AST_H

#include <memory>
#include <string>
#include "stacks.h"

#include "lexer.h"


constexpr bool DEBUG_AST = false;

using std::unique_ptr;
using std::make_unique;
using std::string;

class FunctionDeclarationNode;
using FunctionTable = std::map<string, const FunctionDeclarationNode*>;


class ExpressionNode
{
public:
    virtual Type evaluateNode(EnvironmentStack& scopes) const =0;
    virtual void debugPrint(int indentLevel) const =0;
    virtual ~ExpressionNode() = default;
};


class ProgramNode
{
public:
    Type evaluateNode(EnvironmentStack& scopes);
    void debugPrint(int indentLevel);

    ProgramNode(vector<unique_ptr<ExpressionNode>> stmts) : statements{std::move(stmts)}
    {
    };

private:
    vector<unique_ptr<ExpressionNode>> statements;
};

class IfNode : public ExpressionNode
{
public:
    IfNode(unique_ptr<ExpressionNode> c, unique_ptr<ExpressionNode> thenBranch) : condition{std::move(c)},
        thenStatement{std::move(thenBranch)}
    {
    };

    IfNode(unique_ptr<ExpressionNode> c, unique_ptr<ExpressionNode> thenBranch,
           unique_ptr<ExpressionNode> elseBranch)
        : condition{std::move(c)}, thenStatement{std::move(thenBranch)}, elseStatement{std::move(elseBranch)}
    {
    };
    Type evaluateNode(EnvironmentStack& scopes) const override;
    void debugPrint(int indentLevel) const override;

private:
    unique_ptr<ExpressionNode> condition;
    unique_ptr<ExpressionNode> thenStatement;
    unique_ptr<ExpressionNode> elseStatement;
};

class WhileNode : public ExpressionNode
{
public:
    WhileNode(unique_ptr<ExpressionNode> c, unique_ptr<ExpressionNode> whileBranch) : condition{std::move(c)},
        statement{std::move(whileBranch)}
    {
    };
    Type evaluateNode(EnvironmentStack& scopes) const override;
    void debugPrint(int indentLevel) const override;

private:
    unique_ptr<ExpressionNode> condition;
    unique_ptr<ExpressionNode> statement;
};

class BlockNode : public ExpressionNode
{
public:
    BlockNode(vector<unique_ptr<ExpressionNode>> ss) : statements{std::move(ss)}
    {
    };
    void debugPrint(int indentLevel) const override;
    Type evaluateNode(EnvironmentStack& scopes) const override;

private:
    vector<unique_ptr<ExpressionNode>> statements;
};

class UnaryNode : public ExpressionNode
{
public:
    UnaryNode(Token unOp, unique_ptr<ExpressionNode> n_node) : op{unOp}, child{std::move(n_node)}
    {
    };
    Type evaluateNode(EnvironmentStack& scopes) const override;
    void debugPrint(int indentLevel) const override;

private:
    Token op;
    unique_ptr<ExpressionNode> child;
};

class NumberNode : public ExpressionNode
{
public:
    explicit NumberNode(double v) : value{v}
    {
    };
    Type evaluateNode(EnvironmentStack& scopes) const override;
    void debugPrint(int indentLevel) const override;

private:
    double value;
};

class StringNode : public ExpressionNode
{
public:
    StringNode(string str) : value{str}
    {
    };
    void debugPrint(int indentLevel) const override;
    Type evaluateNode(EnvironmentStack& scopes) const override;

private:
    string value;
};

class BinaryNode : public ExpressionNode
{
public:
    BinaryNode(Token biOp, unique_ptr<ExpressionNode> leftNode, unique_ptr<ExpressionNode> rightNode)
        : op{biOp}, left{std::move(leftNode)}, right{std::move(rightNode)}
    {
    };
    Type evaluateNode(EnvironmentStack& scopes) const override;
    void debugPrint(int indentLevel) const override;

private:
    Token op;
    unique_ptr<ExpressionNode> left;
    unique_ptr<ExpressionNode> right;
};

class VariableNode : public ExpressionNode
{
public:
    explicit VariableNode(std::string name) : identifierName{name}
    {
    };
    Type evaluateNode(EnvironmentStack& scopes) const override;
    const string& getIdentifierName() const { return identifierName; };
    void debugPrint(int indentLevel) const override;

private:
    string identifierName;
};

class AssignmentNode : public ExpressionNode
{
public:
    AssignmentNode(unique_ptr<VariableNode> left, unique_ptr<ExpressionNode> right) :
        lvalue{std::move(left)}, rvalue{std::move(right)}
    {
    };
    Type evaluateNode(EnvironmentStack& scopes) const override;
    void debugPrint(int indentLevel) const override;

private:
    unique_ptr<VariableNode> lvalue;
    unique_ptr<ExpressionNode> rvalue;
};

class DeclarationNode : public ExpressionNode
{
public:
    DeclarationNode(unique_ptr<VariableNode> left, unique_ptr<ExpressionNode> right) : lvalue{std::move(left)},
        rvalue{std::move(right)}
    {
    };

    DeclarationNode(unique_ptr<VariableNode> left) : lvalue{std::move(left)}, rvalue(make_unique<NumberNode>(0))
    {
    };
    Type evaluateNode(EnvironmentStack& scopes) const override;
    void debugPrint(int indentLevel) const override;

private:
    unique_ptr<VariableNode> lvalue;
    unique_ptr<ExpressionNode> rvalue;
};

class FunctionCallNode : public ExpressionNode
{
public:
    explicit FunctionCallNode(string name) : identifierName{name}
    {
    };

    FunctionCallNode(string name, vector<unique_ptr<ExpressionNode>> parameters) : identifierName{name},
        arguments{std::move(parameters)}
    {
    };
    Type evaluateNode(EnvironmentStack& scopes) const override;
    void debugPrint(int indentLevel) const override;

private
:
    string identifierName;
    vector<unique_ptr<ExpressionNode>> arguments;
};

void validateArity(int expected_arguments, int given_arguments, string f_name);

class FunctionDeclarationNode : public ExpressionNode
{
public:
    FunctionDeclarationNode(string fname, vector<string> para, unique_ptr<ExpressionNode> b)
        : identifier{fname}, parameters{std::move(para)}, body{std::move(b)}
    {
    };
    void debugPrint(int indentLevel) const override;
    Type evaluateNode(EnvironmentStack& scopes) const override;
    int getParametersSize() const { return parameters.size(); };
    vector<string> getParameters() const { return parameters; };
    void evaluateBody(EnvironmentStack& scopes) const { body->evaluateNode(scopes); };

private:
    string identifier;
    vector<string> parameters;
    unique_ptr<ExpressionNode> body;
};

class ReturnSignal
{
public:
    ReturnSignal(Type val) : value{val}
    {
    };
    Type value;
};

class ReturnNode : public ExpressionNode
{
public:
    ReturnNode(unique_ptr<ExpressionNode> statement) : returnStatement(std::move(statement))
    {
    };
    void debugPrint(int indentLevel) const override;
    Type evaluateNode(EnvironmentStack& scopes) const override;

private:
    unique_ptr<ExpressionNode> returnStatement;
};

#endif //INTERPRETER_AST_H
