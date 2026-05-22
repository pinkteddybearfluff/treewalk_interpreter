#ifndef INTERPRETER_AST_H
#define INTERPRETER_AST_H

#include <memory>
#include <string>
#include "stacks.h"

#include "lexer.h"

constexpr bool DEBUG_AST = true;

using std::unique_ptr;
using std::make_unique;
using std::string;

class ExpressionNode
{
public:
    virtual double evaluateNode(EnvironmentStack& scopes) const =0;
    virtual void debugPrint(int indentLevel) const =0;
    virtual ~ExpressionNode() = default;
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
    double evaluateNode(EnvironmentStack& scopes) const override;
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
    double evaluateNode(EnvironmentStack& scopes) const override;
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
    double evaluateNode(EnvironmentStack& scopes) const override;

private:
    vector<unique_ptr<ExpressionNode>> statements;
};

class UnaryNode : public ExpressionNode
{
public:
    UnaryNode(Token unOp, unique_ptr<ExpressionNode> n_node) : op{unOp}, child{std::move(n_node)}
    {
    };
    double evaluateNode(EnvironmentStack& scopes) const override;
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
    double evaluateNode(EnvironmentStack& scopes) const override;
    void debugPrint(int indentLevel) const override;

private:
    double value;
};

class BinaryNode : public ExpressionNode
{
public:
    BinaryNode(Token biOp, unique_ptr<ExpressionNode> leftNode, unique_ptr<ExpressionNode> rightNode)
        : op{biOp}, left{std::move(leftNode)}, right{std::move(rightNode)}
    {
    };
    double evaluateNode(EnvironmentStack& scopes) const override;
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
    double evaluateNode(EnvironmentStack& scopes) const override;
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
    double evaluateNode(EnvironmentStack& scopes) const override;
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
    double evaluateNode(EnvironmentStack& scopes) const override;
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
    double evaluateNode(EnvironmentStack& scopes) const override;
    void debugPrint(int indentLevel) const override;

private
:
    string identifierName;
    vector<unique_ptr<ExpressionNode>> arguments;
};

void validateArity(int expected_arguments, int given_arguments, string f_name);

#endif //INTERPRETER_AST_H
