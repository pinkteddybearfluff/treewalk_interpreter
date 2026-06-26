#ifndef INTERPRETER_AST_H
#define INTERPRETER_AST_H

#include <memory>
#include <string>
#include <format>
#include <utility>
#include <cmath>
#include <fstream>
#include "environment.h"

#include "lexer.h"
#include "RuntimeError.h"

constexpr bool DEBUG_AST = true;
constexpr bool EVALUATE = true;

using std::unique_ptr;
using std::make_unique;
using std::string;

struct FormatContext;
class FunctionDeclarationNode;

struct EvalResult
{
    bool hasValue;
    RuntimeValue value;
};

class TypeNode
{
public:
    virtual void format(FormatContext& ctx) const = 0;
    virtual void debugPrint(int indentLevel) const = 0;
    virtual ~TypeNode() = default;
};

class StatementNode
{
public:
    // evaluateNode only evaluates the ast and returns its result hence not mutating the ast and therefore const
    virtual EvalResult evaluateNode(InterpreterContext& ctx) const =0;
    virtual void format(FormatContext& ctx) const = 0;
    virtual void debugPrint(int indentLevel) const = 0;
    virtual string description() const { return "statement"; }
    virtual ~StatementNode() = default;
};


class ExpressionNode
{
public:
    // evaluateNode only evaluates the ast and returns its result hence not mutating the ast and therefore const
    virtual EvalResult evaluateNode(InterpreterContext& ctx) const =0;

    virtual RuntimeValue& getReference(InterpreterContext& ctx)
    {
        throw std::runtime_error("Expression is not assignable");
    }

    virtual void format(FormatContext& ctx) const = 0;
    virtual void debugPrint(int indentLevel) const =0;
    virtual ~ExpressionNode() = default;
    [[nodiscard]] virtual bool isAssignmentTarget() const { return true; };
    [[nodiscard]] virtual bool isDeclarationTarget() const { return false; };
    [[nodiscard]] virtual string description() const { return "Expression"; };
    [[nodiscard]] virtual string getIdentifierName() const { return "has no identifier"; }

protected:
    int line{0};
};


class ProgramNode
{
public:
    EvalResult evaluateNode(InterpreterContext& ctx) const;
    void debugPrint(int indentLevel) const;

    void format(FormatContext& ctx) const;

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
    //evaluateNode evaluates the rhs (here gets the rvalue of identifier)
    EvalResult evaluateNode(InterpreterContext& ctx) const override;
    [[nodiscard]] string getIdentifierName() const override { return identifierName; };
    //getReference gets the reference/lvalue of the identifier from the runtime environment
    RuntimeValue& getReference(InterpreterContext& ctx) override;
    void format(FormatContext& ctx) const override;

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
    EvalResult evaluateNode(InterpreterContext& ctx) const override;
    void format(FormatContext& ctx) const override;
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
    void format(FormatContext& ctx) const override;
    void debugPrint(int indentLevel) const override;
    EvalResult evaluateNode(InterpreterContext& ctx) const override;
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
    void format(FormatContext& ctx) const override;
    void debugPrint(int indentLevel) const override;
    EvalResult evaluateNode(InterpreterContext& ctx) const override;
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
    void format(FormatContext& ctx) const override;
    void debugPrint(int indentLevel) const override;
    EvalResult evaluateNode(InterpreterContext& ctx) const override;
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
    void format(FormatContext& ctx) const override;
    void debugPrint(int indentLevel) const override;
    EvalResult evaluateNode(InterpreterContext& ctx) const override;
    [[nodiscard]] bool isDeclarationTarget() const override { return false; };
    [[nodiscard]] bool isAssignmentTarget() const override { return false; };
    [[nodiscard]] string description() const override { return "literal"; };

private:
    vector<std::unique_ptr<ExpressionNode>> value;

protected:
    int line;
};

struct Pair
{
    unique_ptr<ExpressionNode> first;
    unique_ptr<ExpressionNode> second;
};

class MapNode : public ExpressionNode
{
public:
    EvalResult evaluateNode(InterpreterContext& ctx) const override;
    void debugPrint(int indentLevel) const override;
    void format(FormatContext& ctx) const override;

    MapNode(vector<Pair> keyValExp, int lineNo) : keyValPairs{std::move(keyValExp)}
                                                  , line{lineNo}
    {
    }

private:
    vector<Pair> keyValPairs;

protected:
    int line;
};

class StructNode : public StatementNode
{
public:
    EvalResult evaluateNode(InterpreterContext& ctx) const override;
    void debugPrint(int indentLevel) const override;
    void format(FormatContext& ctx) const override;
    void registerInEnv(InterpreterContext& ctx) const;

    StructNode(string name, vector<string> fields, vector<unique_ptr<StatementNode>> methodStatement) :
        typeName(name),
        fieldNames{fields},
        methodsStmt{std::move(methodStatement)}
    {
    }

private:
    string typeName;
    vector<string> fieldNames;
    vector<unique_ptr<StatementNode>> methodsStmt;
};

class IndexNode : public ExpressionNode
{
public:
    IndexNode(unique_ptr<ExpressionNode> Exp, unique_ptr<ExpressionNode> iExp, int lineNo) : operand{std::move(Exp)}
        ,
        indexExp{std::move(iExp)}, line{lineNo}
    {
    };
    void format(FormatContext& ctx) const override;
    void debugPrint(int indentLevel) const override;
    //evaluateNode evaluates the node of the current ast and returns rvalue
    EvalResult evaluateNode(InterpreterContext& ctx) const override;
    //getReference gets reference of the identifier from the environment and returns lvalue
    RuntimeValue& getReference(InterpreterContext& ctx) override;
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
    EvalResult evaluateNode(InterpreterContext& ctx) const override;
    void format(FormatContext& ctx) const override;
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
    EvalResult evaluateNode(InterpreterContext& ctx) const override;
    void format(FormatContext& ctx) const override;
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
    EvalResult evaluateNode(InterpreterContext& ctx) const override;
    void format(FormatContext& ctx) const override;
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
    EvalResult evaluateNode(InterpreterContext& ctx) const override;
    // vector<unique_ptr<ExpressionNode>>& getReference(EnvironmentStack& scopes) const;
    void format(FormatContext& ctx) const override;
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

class MemberAccessNode : public ExpressionNode
{
public:
    EvalResult evaluateNode(InterpreterContext& ctx) const override;
    void format(FormatContext& ctx) const override;
    void debugPrint(int indentLevel) const override;

    MemberAccessNode(unique_ptr<ExpressionNode> object, unique_ptr<ExpressionNode> m, int lineNo) : obj{
            std::move(object)
        },
        member{std::move(m)}, line{lineNo}
    {
    }

    RuntimeValue& getReference(InterpreterContext& ctx) override;
    RuntimeValue getObjVal(InterpreterContext& ctx) { return obj->evaluateNode(ctx).value; }

    string getObjName() const { return obj->getIdentifierName(); }

private:
    // string name;
    unique_ptr<ExpressionNode> obj;
    unique_ptr<ExpressionNode> member;

protected:
    int line;
};

class PatternNode
{
public:
    virtual ~PatternNode() = default;
    virtual string type() const =0;
    virtual bool matches(const RuntimeValue& value, InterpreterContext& ctx) const =0;
};

class OrPattern : public PatternNode
{
public:
    string type() const override { return "or"; }

    bool matches(const RuntimeValue& value, InterpreterContext& ctx) const override
    {
        for (const auto& pattern : patterns)
        {
            if (pattern->matches(value, ctx))return true;
            return false;
        }
    };

    OrPattern(vector<unique_ptr<PatternNode>> pattern_nodes) : patterns{std::move(pattern_nodes)}
    {
    };

private:
    vector<unique_ptr<PatternNode>> patterns;
};

class LiteralPattern : public PatternNode
{
public:
    bool matches(const RuntimeValue& value, InterpreterContext& ctx) const override
    {
        return this->value == value;
    };

    LiteralPattern(const RuntimeValue& val) : value{val}
    {
    };
    string type() const override { return "literal"; }

private:
    RuntimeValue value;
};

class IdentifierPattern : public PatternNode
{
public:
    bool matches(const RuntimeValue& value, InterpreterContext& ctx) const override
    {
        return value == ctx.env->getReference(name).value;
    };

    IdentifierPattern(string name) : name{name}
    {
    }

    string type() const override { return "identifier"; }

private:
    string name;
};

class RangePattern : public PatternNode
{
public:
    bool matches(const RuntimeValue& value, InterpreterContext& ctx) const override
    {
        if (inclusive)
        {
            if (Operator::lessEqual(value, end).asBoolean() && Operator::greaterEqual(value, start).asBoolean())
                return
                    true;
            else return false;
        }
        if (Operator::less(value, end).asBoolean() && Operator::greaterEqual(value, start).asBoolean())return true;

        return false;
    }

    string type() const override { return "range"; }

    RangePattern(RuntimeValue start, RuntimeValue end, bool inclusive) : start{std::move(start)}, end{std::move(end)},
                                                                         inclusive{inclusive}
    {
    };

private:
    RuntimeValue start;
    RuntimeValue end;
    bool inclusive;
};

class WildCardPattern : public PatternNode
{
public:
    bool matches(const RuntimeValue& value, InterpreterContext& ctx) const override { return true; }
    string type() const override { return "wildcard"; }
};

struct MatchArm
{
    unique_ptr<PatternNode> pattern;
    unique_ptr<ExpressionNode> expr;
};

class MatchPatternNode : public ExpressionNode
{
public:
    EvalResult evaluateNode(InterpreterContext& ctx) const override;
    void debugPrint(int indentLevel) const override;
    void format(FormatContext& ctx) const override;

    MatchPatternNode(unique_ptr<ExpressionNode> discriminant, vector<MatchArm> armsExpr, int lineNo) :
        scrutinee{std::move(discriminant)}, armsExpr{std::move(armsExpr)}, line{lineNo}
    {
    }

private:
    unique_ptr<ExpressionNode> scrutinee;
    vector<MatchArm> armsExpr;

protected:
    int line;
};

class YieldSignal
{
public:
    RuntimeValue value;
};


class BlockExpressionNode : public ExpressionNode
{
public :
    EvalResult evaluateNode(InterpreterContext& ctx) const override;
    void debugPrint(int indentLevel) const override;
    void format(FormatContext& ctx) const override;

    BlockExpressionNode(vector<unique_ptr<StatementNode>> stmts, unique_ptr<ExpressionNode> result) : statements{
            std::move(stmts)
        }, resultExpr{std::move(result)}
    {
    }

private:
    vector<unique_ptr<StatementNode>> statements;
    unique_ptr<ExpressionNode> resultExpr;
};

class ExpressionStatementNode : public StatementNode
{
public:
    void format(FormatContext& ctx) const override;
    void debugPrint(int indentLevel) const override;
    EvalResult evaluateNode(InterpreterContext& ctx) const override;
    string description() const override { return "ExpressionStatement"; }

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

    DeclarationNode(unique_ptr<ExpressionNode> left, unique_ptr<ExpressionNode> right, unique_ptr<TypeNode> type,
                    int lineNo) :
        lvalue{std::move(left)},
        rvalue{std::move(right)},
        type{std::move(type)},
        line{lineNo}
    {
    };

    DeclarationNode(unique_ptr<ExpressionNode> left, int lineNo) : lvalue{std::move(left)},
                                                                   rvalue(nullptr),
                                                                   hasInitialValue{false},
                                                                   line{lineNo}
    {
    };

    DeclarationNode(unique_ptr<ExpressionNode> left, unique_ptr<TypeNode> type, int lineNo) : lvalue{std::move(left)},
        rvalue(nullptr), type{std::move(type)},
        line{lineNo}
    {
    };
    EvalResult evaluateNode(InterpreterContext& ctx) const override;
    void format(FormatContext& ctx) const override;
    void debugPrint(int indentLevel) const override;
    string description() const override { return "Declaration"; }

private:
    unique_ptr<ExpressionNode> lvalue;
    unique_ptr<ExpressionNode> rvalue;
    bool hasInitialValue{true};

    unique_ptr<TypeNode> type{nullptr};

protected:
    int line;
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
    EvalResult evaluateNode(InterpreterContext& ctx) const override;
    void format(FormatContext& ctx) const override;
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
    EvalResult evaluateNode(InterpreterContext& ctx) const override;
    void format(FormatContext& ctx) const override;
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
    EvalResult evaluateNode(InterpreterContext& ctx) const override;
    void format(FormatContext& ctx) const override;
    void debugPrint(int indentLevel) const override;

private:
    unique_ptr<StatementNode> initializer;
    unique_ptr<ExpressionNode> condition;
    unique_ptr<ExpressionNode> expr;
    unique_ptr<StatementNode> statement;
};

class ForEachNode : public StatementNode
{
public:
    void debugPrint(int indentLevel) const override;
    EvalResult evaluateNode(InterpreterContext& ctx) const override;
    void format(FormatContext& ctx) const override;

    ForEachNode(vector<string> init, unique_ptr<ExpressionNode> container, unique_ptr<StatementNode> stmt) :
        identifiers{init}, containerExp{std::move(container)}, statement{std::move(stmt)}
    {
    };

private:
    vector<string> identifiers;
    unique_ptr<ExpressionNode> containerExp;
    unique_ptr<StatementNode> statement;
};

class BreakNode : public StatementNode
{
public:
    BreakNode()
    {
    }

    EvalResult evaluateNode(InterpreterContext& ctx) const override;
    void format(FormatContext& ctx) const override;
    void debugPrint(int indentLevel) const override;
    string description() const override { return "Break"; }
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
    EvalResult evaluateNode(InterpreterContext& ctx) const override;
    void format(FormatContext& ctx) const override;
    void debugPrint(int indentLevel) const override;
    string description() const override { return "Continue"; }
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
    void format(FormatContext& ctx) const override;
    EvalResult evaluateNode(InterpreterContext& ctx) const override;
    string description() const override { return "block"; }

private:
    vector<unique_ptr<StatementNode>> statements;
};

class UnaryNode : public ExpressionNode
{
public:
    UnaryNode(Token unOp, unique_ptr<ExpressionNode> n_node) : op{unOp}, child{std::move(n_node)}
    {
    };
    EvalResult evaluateNode(InterpreterContext& ctx) const override;
    void format(FormatContext& ctx) const override;
    void debugPrint(int indentLevel) const override;

private:
    Token op;
    unique_ptr<ExpressionNode> child;
};

struct Param
{
    string identifier;
    unique_ptr<TypeNode> type{nullptr};
};

class FunctionDeclarationNode : public StatementNode
{
public:
    FunctionDeclarationNode(string fname, vector<Param> para, unique_ptr<StatementNode> b, bool variadic,
                            Param variadicParam, int lineNo)
        : identifier{fname}, parameters{std::move(para)}, body{std::move(b)}, variadic{variadic},
          variadicParam{std::move(variadicParam)}, line{lineNo}
    {
    }

    FunctionDeclarationNode(string fname, vector<Param> para, unique_ptr<StatementNode> b, bool variadic,
                            Param variadicParam, unique_ptr<TypeNode> type, int lineNo)
        : identifier{fname}, parameters{std::move(para)}, body{std::move(b)}, variadic{variadic},
          variadicParam{std::move(variadicParam)}, type{std::move(type)}, line{lineNo}
    {
    }

    void format(FormatContext& ctx) const override;
    void debugPrint(int indentLevel) const override;
    EvalResult evaluateNode(InterpreterContext& ctx) const override;
    void registerInEnv(InterpreterContext& ctx) const;
    void registerInCustomEnv(std::map<string, shared_ptr<FunctionObject>>& methodEnv);
    int getParametersSize() const { return parameters.size(); };
    bool isVariadic() const { return variadic; };
    Param variadicParam;

private:
    string identifier;
    vector<Param> parameters;
    unique_ptr<StatementNode> body;
    bool variadic{false};
    unique_ptr<TypeNode> type{nullptr};
    int line;
};


class ReturnNode : public StatementNode
{
public:
    ReturnNode(unique_ptr<ExpressionNode> statement = nullptr) : returnStatement(std::move(statement))
    {
    };
    void format(FormatContext& ctx) const override;
    void debugPrint(int indentLevel) const override;
    string description() const override { return "Return"; }
    EvalResult evaluateNode(InterpreterContext& ctx) const override;

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
    void format(FormatContext& ctx) const override;
    void debugPrint(int indentLevel) const override;
    EvalResult evaluateNode(InterpreterContext& ctx) const override;
};

class ImportNode : public StatementNode
{
public:
    void format(FormatContext& ctx) const override;
    void debugPrint(int indentLevel) const override;
    EvalResult evaluateNode(InterpreterContext& ctx) const override;

    ImportNode(string fP, string alias, int lineNo) : file{fP}, alias{alias}, line{lineNo}
    {
    }

    ImportNode(string fP, string alias, bool isStdLib, int lineNo) : file{fP}, alias{alias}, isStdLib{isStdLib},
                                                                     line{lineNo}
    {
    }

private:
    string file;
    string alias;
    bool isStdLib{false};

protected:
    int line;
};

void terminateLine(string description, std::ofstream& os);
void terminateWithNL(string description, std::ofstream& os);

class PrimitiveType : public TypeNode
{
public:
    explicit PrimitiveType(string t) : type{t}
    {
    }

    void format(FormatContext& ctx) const override;
    void debugPrint(int indentLevel) const override;

private:
    string type;
};

class ArrayType : public TypeNode
{
public:
    explicit ArrayType(unique_ptr<TypeNode> nestedT) : nestedType{std::move(nestedT)}
    {
    }

    void format(FormatContext& ctx) const override;
    void debugPrint(int indentLevel) const override;

private:
    unique_ptr<TypeNode> nestedType;
};

class TupleType : public TypeNode
{
public:
    explicit TupleType(vector<unique_ptr<TypeNode>> types) : types{std::move(types)}
    {
    }

    void format(FormatContext& ctx) const override;
    void debugPrint(int indentLevel) const override;

private:
    vector<unique_ptr<TypeNode>> types;
};

class UnionType : public TypeNode
{
public:
    explicit UnionType(vector<unique_ptr<TypeNode>> types) : types{std::move(types)}
    {
    }

    void format(FormatContext& ctx) const override;
    void debugPrint(int indentLevel) const override;

private:
    vector<unique_ptr<TypeNode>> types;
};

struct FormatContext
{
    int indentLevel{0};
    std::ostream& os;

    void indent() { ++indentLevel; }
    void dedent() { --indentLevel; }

    void writeIndent()
    {
        os << string(indentLevel * 4, ' ');
    }
};

#endif //INTERPRETER_AST_H
