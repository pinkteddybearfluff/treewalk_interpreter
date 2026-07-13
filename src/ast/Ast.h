#ifndef INTERPRETER_AST_H
#define INTERPRETER_AST_H

#include <memory>
#include <string>
#include <format>
#include <utility>
#include <cmath>
#include <fstream>
#include "../runtime/Environment.h"

#include "../lexer/Lexer.h"
#include "../error/RuntimeError.h"

constexpr bool DEBUG_AST = true;

using std::unique_ptr;
using std::make_unique;
using std::string;

class Visitor;
class TypeVisitor;
class PatternVisitor;

struct FormatContext;
class FunctionDeclarationNode;


class TypeNode
{
public:
    virtual ~TypeNode() = default;
    virtual void accept(TypeVisitor& visitor) =0;
};

class StatementNode
{
public:
    virtual string description() const { return "statement"; }
    virtual ~StatementNode() = default;
    virtual void accept(Visitor& visitor) =0;
};


class ExpressionNode
{
public:
    // evaluateNode only evaluates the ast and returns its result hence not mutating the ast and therefore const

    virtual RuntimeValue& getReference(InterpreterContext& ctx)
    {
        throw std::runtime_error("Expression is not assignable");
    }

    virtual ~ExpressionNode() = default;
    [[nodiscard]] virtual bool isAssignmentTarget() const { return false; };
    [[nodiscard]] virtual bool isDeclarationTarget() const { return false; };
    [[nodiscard]] virtual string description() const { return "Expression"; };
    [[nodiscard]] virtual string getIdentifierName() const { return "has no identifier"; }

    virtual void accept(Visitor& visitor) =0;

protected:
    int line{0};
};


class ProgramNode
{
public:
    void accept(Visitor& visitor);

    ProgramNode(vector<unique_ptr<StatementNode>> stmts) : statements{std::move(stmts)}
    {
    };

    vector<unique_ptr<StatementNode>> statements;
};


class NumberNode : public ExpressionNode
{
public:
    NumberNode(double v, int lineNo) : value{v}, line{lineNo}
    {
    };
    [[nodiscard]] string description() const override { return "literal"; };
    void accept(Visitor& visitor) override;

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
    [[nodiscard]] string description() const override { return "literal"; };

    void accept(Visitor& visitor) override;

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
    [[nodiscard]] string description() const override { return "literal"; };

    void accept(Visitor& visitor) override;

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
    [[nodiscard]] string description() const override { return "literal"; };

    void accept(Visitor& visitor) override;

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
    [[nodiscard]] string description() const override { return "literal"; };

    void accept(Visitor& visitor) override;

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
    MapNode(vector<Pair> keyValExp, int lineNo) : keyValPairs{std::move(keyValExp)}
                                                  , line{lineNo}
    {
    }

    void accept(Visitor& visitor) override;

    vector<Pair> keyValPairs;

protected:
    int line;
};

class VariableNode : public ExpressionNode
{
public:
    VariableNode(std::string name, int line) : identifierName{name}, line{line}
    {
    };
    [[nodiscard]] string getIdentifierName() const override { return identifierName; };
    //getReference gets the reference/lvalue of the identifier from the runtime environment

    [[nodiscard]] bool isAssignmentTarget() const override { return true; };
    [[nodiscard]] bool isDeclarationTarget() const override { return true; };
    [[nodiscard]] string description() const override { return "variable"; };

    void accept(Visitor& visitor) override;

    string identifierName;

protected:
    int line;
};

class IsNode : public ExpressionNode
{
public:
    IsNode(unique_ptr<ExpressionNode> value, unique_ptr<ExpressionNode> enumVariant, int lineNo) : value{
            std::move(value)
        },
        enumVariant{std::move(enumVariant)}, line{lineNo}
    {
    }

    void accept(Visitor& visitor) override;

    unique_ptr<ExpressionNode> value;
    unique_ptr<ExpressionNode> enumVariant;

protected:
    int line;
};

class UnaryNode : public ExpressionNode
{
public:
    UnaryNode(Token unOp, unique_ptr<ExpressionNode> n_node) : op{unOp}, child{std::move(n_node)}
    {
    };

    void accept(Visitor& visitor) override;

    Token op;
    unique_ptr<ExpressionNode> child;
};

class BinaryNode : public ExpressionNode
{
public:
    BinaryNode(Token biOp, unique_ptr<ExpressionNode> leftNode, unique_ptr<ExpressionNode> rightNode, int lineNo)
        : op{biOp}, left{std::move(leftNode)}, right{std::move(rightNode)}, line{lineNo}
    {
    };
    [[nodiscard]] string description() const override { return "literal"; };

    void accept(Visitor& visitor) override;

    Token op;
    unique_ptr<ExpressionNode> left;
    unique_ptr<ExpressionNode> right;

protected:
    int line;
};

class IndexNode : public ExpressionNode
{
public:
    IndexNode(unique_ptr<ExpressionNode> Exp, unique_ptr<ExpressionNode> iExp, int lineNo) : operand{std::move(Exp)}
        ,
        subscript{std::move(iExp)}, line{lineNo}
    {
    };
    //getReference gets reference of the identifier from the environment and returns lvalue
    [[nodiscard]] bool isAssignmentTarget() const override { return true; };
    [[nodiscard]] string description() const override { return "array subscript"; };

    void accept(Visitor& visitor) override;

    unique_ptr<ExpressionNode> operand;
    unique_ptr<ExpressionNode> subscript;

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

    [[nodiscard]] string description() const override { return "function call"; };

    void accept(Visitor& visitor) override;

    unique_ptr<ExpressionNode> identifier;
    vector<unique_ptr<ExpressionNode>> arguments;

protected:
    int line;
};

class MemberAccessNode : public ExpressionNode
{
public:
    MemberAccessNode(unique_ptr<ExpressionNode> object, unique_ptr<ExpressionNode> m, int lineNo) : obj{
            std::move(object)
        },
        member{std::move(m)}, line{lineNo}
    {
    }


    string getObjName() const { return obj->getIdentifierName(); }
    string getMemberName() const { return member->getIdentifierName(); }

    void accept(Visitor& visitor) override;

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
    virtual void accept(PatternVisitor& visitor) =0;
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
    void accept(PatternVisitor& visitor) override;

    OrPattern(vector<unique_ptr<PatternNode>> pattern_nodes) : patterns{std::move(pattern_nodes)}
    {
    };

    vector<unique_ptr<PatternNode>> patterns;
};

class LiteralPattern : public PatternNode
{
public:
    bool matches(const RuntimeValue& value, InterpreterContext& ctx) const override
    {
        return this->value == value;
    };
    void accept(PatternVisitor& visitor) override;

    LiteralPattern(const RuntimeValue& val) : value{val}
    {
    };
    string type() const override { return "literal"; }

    RuntimeValue value;
};

class IdentifierPattern : public PatternNode
{
public:
    bool matches(const RuntimeValue& value, InterpreterContext& ctx) const override
    {
        return value == ctx.env->getReference(name).value;
    };
    void accept(PatternVisitor& visitor) override;

    IdentifierPattern(string name) : name{name}
    {
    }

    string type() const override { return "identifier"; }

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

    void accept(PatternVisitor& visitor) override;
    string type() const override { return "range"; }

    RangePattern(RuntimeValue start, RuntimeValue end, bool inclusive) : start{std::move(start)}, end{std::move(end)},
                                                                         inclusive{inclusive}
    {
    };

    RuntimeValue start;
    RuntimeValue end;
    bool inclusive;
};

class WildCardPattern : public PatternNode
{
public:
    bool matches(const RuntimeValue& value, InterpreterContext& ctx) const override { return true; }
    string type() const override { return "wildcard"; }
    void accept(PatternVisitor& visitor) override;
};

struct MatchArm
{
    unique_ptr<PatternNode> pattern;
    unique_ptr<ExpressionNode> expr;
};

class MatchPatternNode : public ExpressionNode
{
public:
    MatchPatternNode(unique_ptr<ExpressionNode> discriminant, vector<MatchArm> armsExpr, int lineNo) :
        scrutinee{std::move(discriminant)}, armsExpr{std::move(armsExpr)}, line{lineNo}
    {
    }

    void accept(Visitor& visitor) override;
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
    BlockExpressionNode(vector<unique_ptr<StatementNode>> stmts, unique_ptr<ExpressionNode> result) : statements{
            std::move(stmts)
        }, resultExpr{std::move(result)}
    {
    }

    void accept(Visitor& visitor) override;

    vector<unique_ptr<StatementNode>> statements;
    unique_ptr<ExpressionNode> resultExpr;
};

class AssignmentNode : public ExpressionNode
{
public:
    AssignmentNode(unique_ptr<ExpressionNode> left, unique_ptr<ExpressionNode> right, int lineNo) :
        lvalue{std::move(left)}, rvalue{std::move(right)}, line{lineNo}
    {
    };

    [[nodiscard]] bool isAssignmentTarget() const override { return true; };
    [[nodiscard]] string description() const override { return "literal"; };

    void accept(Visitor& visitor) override;

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

    [[nodiscard]] bool isAssignmentTarget() const override { return true; };
    [[nodiscard]] string description() const override { return "literal"; };

    void accept(Visitor& visitor) override;

    Token op;
    unique_ptr<ExpressionNode> lvalue;
    unique_ptr<ExpressionNode> rvalue;

protected:
    int line;
};

class ExpressionStatementNode : public StatementNode
{
public:
    string description() const override { return "ExpressionStatement"; }
    void accept(Visitor& visitor) override;

    ExpressionStatementNode(unique_ptr<ExpressionNode> expression) : expressionStmt{std::move(expression)}
    {
    };

    unique_ptr<ExpressionNode> expressionStmt;
};

class StructDeclarationNode : public StatementNode
{
public:
    string description() const override { return "struct"; }
    void registerInEnv(InterpreterContext& ctx) const;
    void accept(Visitor& visitor) override;

    StructDeclarationNode(string name, vector<string> fields, vector<unique_ptr<StatementNode>> methodStatement) :
        typeName(name),
        fieldNames{fields},
        methodsStmt{std::move(methodStatement)}
    {
    }


    string typeName;
    vector<string> fieldNames;
    vector<unique_ptr<StatementNode>> methodsStmt;
};

class EnumDeclarationNode : public StatementNode
{
public:
    void registerInEnv(InterpreterContext& ctx) const;
    void accept(Visitor& visitor) override;

    EnumDeclarationNode(string type, vector<Variant> fields) : typeName{std::move(type)}, variants{std::move(fields)}
    {
    }


    string typeName;
    vector<Variant> variants;
};

class DeclarationNode : public StatementNode
{
public:
    DeclarationNode(unique_ptr<ExpressionNode> left, unique_ptr<ExpressionNode> right, int lineNo) :
        lvalue{std::move(left)},
        rvalue{std::move(right)}, line{lineNo}
    {
    };
    void accept(Visitor& visitor) override;

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
    string description() const override { return "Declaration"; }


    unique_ptr<ExpressionNode> lvalue;
    unique_ptr<ExpressionNode> rvalue;
    bool hasInitialValue{true};

    unique_ptr<TypeNode> type{nullptr};

protected:
    int line;
};

struct Param
{
    string identifier;
    unique_ptr<TypeNode> type{nullptr};
    unique_ptr<ExpressionNode> defaultArg{nullptr};
    bool isVariadic{false};
};

class FunctionDeclarationNode : public StatementNode
{
public:
    FunctionDeclarationNode(string fname, vector<Param> para, unique_ptr<StatementNode> b, int lineNo)
        : identifier{fname}, parameters{std::move(para)}, body{std::move(b)}, line{lineNo}
    {
    }

    FunctionDeclarationNode(string fname, vector<Param> para, unique_ptr<StatementNode> b, unique_ptr<TypeNode> type,
                            int lineNo)
        : identifier{fname}, parameters{std::move(para)}, body{std::move(b)}, type{std::move(type)}, line{lineNo}
    {
    }

    void registerInCustomEnv(std::map<string, shared_ptr<FunctionObject>>& methodEnv, InterpreterContext& ctx);

    void accept(Visitor& visitor) override;
    string identifier;
    vector<Param> parameters;
    unique_ptr<StatementNode> body;
    unique_ptr<TypeNode> type{nullptr};
    int line;
};

class IfNode : public StatementNode
{
public:
    IfNode(unique_ptr<ExpressionNode> c, unique_ptr<StatementNode> thenBranch) : condition{std::move(c)},
        thenStatement{std::move(thenBranch)}
    {
    };
    void accept(Visitor& visitor) override;

    IfNode(unique_ptr<ExpressionNode> c, unique_ptr<StatementNode> thenBranch,
           unique_ptr<StatementNode> elseBranch)
        : condition{std::move(c)}, thenStatement{std::move(thenBranch)}, elseStatement{std::move(elseBranch)}
    {
    };


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
    void accept(Visitor& visitor) override;

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
    void accept(Visitor& visitor) override;

    unique_ptr<StatementNode> initializer;
    unique_ptr<ExpressionNode> condition;
    unique_ptr<ExpressionNode> expr;
    unique_ptr<StatementNode> statement;
};

class ForEachNode : public StatementNode
{
public:
    ForEachNode(vector<string> init, unique_ptr<ExpressionNode> container, unique_ptr<StatementNode> stmt) :
        identifiers{init}, containerExp{std::move(container)}, statement{std::move(stmt)}
    {
    };

    void accept(Visitor& visitor) override;
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

    void accept(Visitor& visitor) override;
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
    void accept(Visitor& visitor) override;
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
    string description() const override { return "block"; }

    void accept(Visitor& visitor) override;
    vector<unique_ptr<StatementNode>> statements;
};


class ReturnNode : public StatementNode
{
public:
    ReturnNode(unique_ptr<ExpressionNode> statement = nullptr) : returnStatement(std::move(statement))
    {
    };
    string description() const override { return "Return"; }
    void accept(Visitor& visitor) override;

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
    void accept(Visitor& visitor) override;
};

class ThrowNode : public StatementNode
{
public:
    ThrowNode(unique_ptr<ExpressionNode> expr) : throwValExp{std::move(expr)}
    {
    }

    void accept(Visitor& visitor) override;

    unique_ptr<ExpressionNode> throwValExp;
};

struct ThrowSignal
{
    RuntimeValue val;
};

struct CatchClause
{
    string name;
    unique_ptr<StatementNode> blockStatement;
};

class TryCatch : public StatementNode
{
public:
    TryCatch(unique_ptr<StatementNode> tryStmt, vector<CatchClause> catchClauses) : tryStatement{std::move(tryStmt)},
        catches{std::move(catchClauses)}
    {
    }

    void accept(Visitor& visitor) override;
    unique_ptr<StatementNode> tryStatement;
    vector<CatchClause> catches;
};

class ImportNode : public StatementNode
{
public:
    ImportNode(string fP, string alias, int lineNo) : file{fP}, alias{alias}, line{lineNo}
    {
    }

    ImportNode(string fP, string alias, bool isStdLib, int lineNo) : file{fP}, alias{alias}, isStdLib{isStdLib},
                                                                     line{lineNo}
    {
    }

    void accept(Visitor& visitor) override;

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

    void accept(TypeVisitor& visitor) override;
    string type;
};

class ArrayType : public TypeNode
{
public:
    explicit ArrayType(unique_ptr<TypeNode> nestedT) : nestedType{std::move(nestedT)}
    {
    }

    void accept(TypeVisitor& visitor) override;
    unique_ptr<TypeNode> nestedType;
};

class MapType : public TypeNode
{
public:
    MapType(unique_ptr<TypeNode> key, unique_ptr<TypeNode> value) : key{std::move(key)}, value{std::move(value)}
    {
    }

    void accept(TypeVisitor& visitor) override;
    unique_ptr<TypeNode> key;
    unique_ptr<TypeNode> value;
};

class ProgramDefinedType : public TypeNode
{
public:
    explicit ProgramDefinedType(string name) : typeName{name}
    {
    }

    void accept(TypeVisitor& visitor) override;
    string typeName;
};

class TupleType : public TypeNode
{
public:
    explicit TupleType(vector<unique_ptr<TypeNode>> types) : types{std::move(types)}
    {
    }

    void accept(TypeVisitor& visitor) override;
    vector<unique_ptr<TypeNode>> types;
};

class UnionType : public TypeNode
{
public:
    explicit UnionType(vector<unique_ptr<TypeNode>> types) : types{std::move(types)}
    {
    }

    void accept(TypeVisitor& visitor) override;
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
