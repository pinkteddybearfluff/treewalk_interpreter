#ifndef INTERPRETER_AST_H
#define INTERPRETER_AST_H

#include <memory>
#include <string>
#include <map>

#include "lexer.h"
//UnaryNode
//NumberNode
//BinaryNode

using std::unique_ptr;
using std::make_unique;
using std::string;
using std::map;

class ExpressionNode
{
public:
    virtual int evaluateNode(map<string, int>& env) const =0;
    virtual void debugPrint(int indentLevel) const =0;
    virtual ~ExpressionNode() = default;
};

class IfNode : public ExpressionNode
{
public:
    IfNode(unique_ptr<ExpressionNode> c, unique_ptr<ExpressionNode> thenBranch) : condition{std::move(c)},
        statement{std::move(thenBranch)}
    {
    };
    int evaluateNode(map<string, int>& env) const override;
    void debugPrint(int indentLevel) const override;

private:
    unique_ptr<ExpressionNode> condition;
    unique_ptr<ExpressionNode> statement;
};

class UnaryNode : public ExpressionNode
{
public:
    UnaryNode(Token unOp, unique_ptr<ExpressionNode> n_node) : op{unOp}, child{std::move(n_node)}
    {
    };
    int evaluateNode(map<string, int>& env) const override;
    void debugPrint(int indentLevel) const override;

private:
    Token op;
    unique_ptr<ExpressionNode> child;
};

class NumberNode : public ExpressionNode
{
public:
    explicit NumberNode(int v) : value{v}
    {
    };
    int evaluateNode(map<string, int>& env) const override;
    void debugPrint(int indentLevel) const override;

private:
    int value;
};

class BinaryNode : public ExpressionNode
{
public:
    BinaryNode(Token biOp, unique_ptr<ExpressionNode> leftNode, unique_ptr<ExpressionNode> rightNode)
        : op{biOp}, left{std::move(leftNode)}, right{std::move(rightNode)}
    {
    };
    int evaluateNode(map<string, int>& env) const override;
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
    int evaluateNode(map<string, int>& env) const override;
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
    int evaluateNode(map<string, int>& env) const override;
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
    int evaluateNode(map<string, int>& env) const override;
    void debugPrint(int indentLevel) const override;

private
:
    string identifierName;
    vector<unique_ptr<ExpressionNode>> arguments;
};

void validateArity(int expected_arguments, int given_arguments, string f_name);

#endif //INTERPRETER_AST_H
