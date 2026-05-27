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

class FunctionDeclarationNode;
using FunctionTable = std::map<string, const FunctionDeclarationNode*>;


class StatementNode
{
public:
    virtual void evaluateNode(EnvironmentStack& scopes) const =0;
    virtual void debugPrint(int indentLevel) const = 0;
    virtual ~StatementNode() = default;
};


class ExpressionNode
{
public:
    virtual RuntimeValue evaluateNode(EnvironmentStack& scopes) const =0;
    virtual void debugPrint(int indentLevel) const =0;
    virtual ~ExpressionNode() = default;
};


class ProgramNode
{
public:
    void evaluateNode(EnvironmentStack& scopes);
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
    explicit VariableNode(std::string name) : identifierName{name}
    {
    };
    RuntimeValue evaluateNode(EnvironmentStack& scopes) const override;
    const string& getIdentifierName() const { return identifierName; };
    RuntimeValue& getReference(EnvironmentStack& scopes);
    void debugPrint(int indentLevel) const override;

private:
    string identifierName;
};

class NumberNode : public ExpressionNode
{
public:
    explicit NumberNode(double v) : value{v}
    {
    };
    RuntimeValue evaluateNode(EnvironmentStack& scopes) const override;
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
    RuntimeValue evaluateNode(EnvironmentStack& scopes) const override;

private:
    string value;
};

class BooleanNode : public ExpressionNode
{
public:
    BooleanNode(bool tv) : value{tv}
    {
    };
    void debugPrint(int indentLevel) const override;
    RuntimeValue evaluateNode(EnvironmentStack& scopes) const override;

private:
    bool value;
};

class ArrayNode : public ExpressionNode
{
public:
    ArrayNode(vector<unique_ptr<ExpressionNode>> array) :
        value{std::move(array)}
    {
    };
    void debugPrint(int indentLevel) const override;
    RuntimeValue evaluateNode(EnvironmentStack& scopes) const override;

private:
    vector<std::unique_ptr<ExpressionNode>> value;
};

class IndexNode : public ExpressionNode
{
public:
    IndexNode(unique_ptr<ExpressionNode> Exp, unique_ptr<ExpressionNode> iExp) : operand{std::move(Exp)}
        ,
        indexExp{std::move(iExp)}
    {
    };
    void debugPrint(int indentLevel) const override;
    RuntimeValue evaluateNode(EnvironmentStack& scopes) const override;
    RuntimeValue& getReference(EnvironmentStack& scopes);
    RuntimeValue getIndex(EnvironmentStack& scopes) const { return indexExp->evaluateNode(scopes); };

private:
    unique_ptr<ExpressionNode> operand;
    unique_ptr<ExpressionNode> indexExp;
};

class IndexAssignmentNode : public ExpressionNode
{
public:
    IndexAssignmentNode(string name, unique_ptr<ExpressionNode> iExp, unique_ptr<ExpressionNode> right) :
        identifier{name}, indexExp{std::move(iExp)}, rvalue{std::move(right)}
    {
    };
    void debugPrint(int indentLevel) const override;
    RuntimeValue evaluateNode(EnvironmentStack& scopes) const override;

private:
    string identifier;
    unique_ptr<ExpressionNode> indexExp;
    unique_ptr<ExpressionNode> rvalue;
};

class BinaryNode : public ExpressionNode
{
public:
    BinaryNode(Token biOp, unique_ptr<ExpressionNode> leftNode, unique_ptr<ExpressionNode> rightNode)
        : op{biOp}, left{std::move(leftNode)}, right{std::move(rightNode)}
    {
    };
    RuntimeValue evaluateNode(EnvironmentStack& scopes) const override;
    void debugPrint(int indentLevel) const override;

private:
    Token op;
    unique_ptr<ExpressionNode> left;
    unique_ptr<ExpressionNode> right;
};


class AssignmentNode : public ExpressionNode
{
public:
    AssignmentNode(unique_ptr<ExpressionNode> left, unique_ptr<ExpressionNode> right) :
        lvalue{std::move(left)}, rvalue{std::move(right)}
    {
    };
    RuntimeValue evaluateNode(EnvironmentStack& scopes) const override;
    void debugPrint(int indentLevel) const override;

private:
    unique_ptr<ExpressionNode> lvalue;
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
    RuntimeValue evaluateNode(EnvironmentStack& scopes) const override;
    void debugPrint(int indentLevel) const override;

private
:
    string identifierName;
    vector<unique_ptr<ExpressionNode>> arguments;
};


class ExpressionStatementNode : public StatementNode
{
public:
    void debugPrint(int indentLevel) const override;
    void evaluateNode(EnvironmentStack& scopes) const override;

    ExpressionStatementNode(unique_ptr<ExpressionNode> expression) : expressionStmt{std::move(expression)}
    {
    };

private:
    unique_ptr<ExpressionNode> expressionStmt;
};

class DeclarationNode : public StatementNode
{
public:
    DeclarationNode(unique_ptr<VariableNode> left, unique_ptr<ExpressionNode> right) : lvalue{std::move(left)},
        rvalue{std::move(right)}
    {
    };

    DeclarationNode(unique_ptr<VariableNode> left) : lvalue{std::move(left)}, rvalue(make_unique<NumberNode>(0))
    {
    };
    void evaluateNode(EnvironmentStack& scopes) const override;
    void debugPrint(int indentLevel) const override;

private:
    unique_ptr<VariableNode> lvalue;
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
    void evaluateNode(EnvironmentStack& scopes) const override;
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
    void evaluateNode(EnvironmentStack& scopes) const override;
    void debugPrint(int indentLevel) const override;

private:
    unique_ptr<ExpressionNode> condition;
    unique_ptr<StatementNode> statement;
};

class BreakNode : public StatementNode
{
public:
    BreakNode()
    {
    }

    void evaluateNode(EnvironmentStack& scopes) const override;
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
    void evaluateNode(EnvironmentStack& scopes) const override;
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
    void evaluateNode(EnvironmentStack& scopes) const override;

private:
    vector<unique_ptr<StatementNode>> statements;
};

class UnaryNode : public ExpressionNode
{
public:
    UnaryNode(Token unOp, unique_ptr<ExpressionNode> n_node) : op{unOp}, child{std::move(n_node)}
    {
    };
    RuntimeValue evaluateNode(EnvironmentStack& scopes) const override;
    void debugPrint(int indentLevel) const override;

private:
    Token op;
    unique_ptr<ExpressionNode> child;
};

void validateArity(int expected_arguments, int given_arguments, string f_name);

class FunctionDeclarationNode : public StatementNode
{
public:
    FunctionDeclarationNode(string fname, vector<string> para, unique_ptr<StatementNode> b)
        : identifier{fname}, parameters{std::move(para)}, body{std::move(b)}
    {
    };
    void debugPrint(int indentLevel) const override;
    void evaluateNode(EnvironmentStack& scopes) const override;
    int getParametersSize() const { return parameters.size(); };
    vector<string> getParameters() const { return parameters; };
    void evaluateBody(EnvironmentStack& scopes) const { body->evaluateNode(scopes); };

private:
    string identifier;
    vector<string> parameters;
    unique_ptr<StatementNode> body;
};


class ReturnNode : public StatementNode
{
public:
    ReturnNode(unique_ptr<ExpressionNode> statement) : returnStatement(std::move(statement))
    {
    };
    void debugPrint(int indentLevel) const override;
    void evaluateNode(EnvironmentStack& scopes) const override;

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
    void evaluateNode(EnvironmentStack& scopes) const override;
};

#endif //INTERPRETER_AST_H
