// SPDX-license-identifier: Apache-2.0
// Copyright © 2021 Intel Corporation

#pragma once

#include <cassert>
#include <cstdint>
#include <memory>
#include <string>
#include <tuple>
#include <unordered_map>
#include <variant>
#include <vector>

namespace Frontend::AST {

class AdditiveExpression;
class Assignment;
class Boolean;
class Identifier;
class MultiplicativeExpression;
class Number;
class String;
class Subscript;
class UnaryExpression;
class Relational;
class FunctionCall;
class MethodCall;
class Array;
class Dict;

using ExpressionV =
    std::variant<std::unique_ptr<AdditiveExpression>, std::unique_ptr<Assignment>, std::unique_ptr<Boolean>,
                 std::unique_ptr<Identifier>, std::unique_ptr<MultiplicativeExpression>,
                 std::unique_ptr<UnaryExpression>, std::unique_ptr<Number>, std::unique_ptr<String>,
                 std::unique_ptr<Subscript>, std::unique_ptr<Relational>, std::unique_ptr<FunctionCall>,
                 std::unique_ptr<MethodCall>, std::unique_ptr<Array>, std::unique_ptr<Dict>>;

using ExpressionList = std::vector<ExpressionV>;

class Number {
  public:
    Number(const int64_t & number) : value{number} {};
    Number(Number && n) noexcept : value{std::move(n.value)} {};
    Number(const Number &) = delete;
    ~Number(){};

    std::string as_string() const;

    int64_t value;
};

class Boolean {
  public:
    Boolean(const bool & b) : value{b} {};
    Boolean(Boolean && b) noexcept : value{std::move(b.value)} {};
    Boolean(const Boolean &) = delete;
    ~Boolean(){};

    std::string as_string() const;

    bool value;
};

class String {
  public:
    String(const std::string & str) : value{str} {};
    String(String && s) noexcept : value{std::move(s.value)} {};
    String(const String &) = delete;
    ~String(){};

    std::string as_string() const;

    std::string value;
};

class Identifier {
  public:
    Identifier(const std::string & str) : value{str} {};
    Identifier(Identifier && s) noexcept : value{std::move(s.value)} {};
    Identifier(const Identifier &) = delete;
    ~Identifier(){};

    std::string as_string() const;

    std::string value;
};

class Assignment {
  public:
    Assignment(ExpressionV && l, ExpressionV && r) : lhs{std::move(l)}, rhs{std::move(r)} {
        // TODO: add real error message? or would it be better for this to take an identifier?
        // Or is this really not a parsing issue, but a semantics issue?
        assert(std::holds_alternative<std::unique_ptr<Identifier>>(lhs));
    };
    Assignment(Assignment && a) noexcept : lhs{std::move(a.lhs)}, rhs{std::move(a.rhs)} {};
    Assignment(const Assignment &) = delete;
    ~Assignment(){};

    std::string as_string() const;

    ExpressionV lhs;
    ExpressionV rhs;
};

class Subscript {
  public:
    Subscript(ExpressionV && l, ExpressionV && r) : lhs{std::move(l)}, rhs{std::move(r)} {};
    Subscript(const Subscript &) = delete;
    Subscript(Subscript && a) noexcept : lhs{std::move(a.lhs)}, rhs{std::move(a.rhs)} {};
    ~Subscript(){};

    std::string as_string() const;

    ExpressionV lhs;
    ExpressionV rhs;
};

enum class UnaryOp {
    NEG,
};

class UnaryExpression {
  public:
    UnaryExpression(const UnaryOp & o, ExpressionV && r) : op{o}, rhs{std::move(r)} {};
    UnaryExpression(UnaryExpression && a) noexcept : op{std::move(a.op)}, rhs{std::move(a.rhs)} {};
    UnaryExpression(const UnaryExpression &) = delete;
    ~UnaryExpression(){};

    std::string as_string() const;

    UnaryOp op;
    ExpressionV rhs;
};

enum class MulOp {
    MUL,
    DIV,
    MOD,
};

class MultiplicativeExpression {
  public:
    MultiplicativeExpression(ExpressionV && l, const MulOp & o, ExpressionV && r)
        : lhs{std::move(l)}, op{o}, rhs{std::move(r)} {};
    MultiplicativeExpression(MultiplicativeExpression && a) noexcept
        : lhs{std::move(a.lhs)}, op{std::move(a.op)}, rhs{std::move(a.rhs)} {};
    MultiplicativeExpression(const MultiplicativeExpression &) = delete;
    ~MultiplicativeExpression(){};

    std::string as_string() const;

    ExpressionV lhs;
    MulOp op;
    ExpressionV rhs;
};

enum class AddOp {
    ADD,
    SUB,
};

class AdditiveExpression {
  public:
    AdditiveExpression(ExpressionV && l, const AddOp & o, ExpressionV && r)
        : lhs{std::move(l)}, op{o}, rhs{std::move(r)} {};
    AdditiveExpression(AdditiveExpression && a) noexcept
        : lhs{std::move(a.lhs)}, op{std::move(a.op)}, rhs{std::move(a.rhs)} {};
    AdditiveExpression(const AdditiveExpression &) = delete;
    ~AdditiveExpression(){};

    std::string as_string() const;

    ExpressionV lhs;
    AddOp op;
    ExpressionV rhs;
};

enum class RelationalOp {
    LT,
    LE,
    EQ,
    NE,
    GE,
    GT,
    AND,
    OR,
    IN,
    NOT_IN,
};

static AST::RelationalOp to_relop(const std::string & s) {
    if (s == "<") {
        return AST::RelationalOp::LT;
    } else if (s == "<=") {
        return AST::RelationalOp::LE;
    } else if (s == "==") {
        return AST::RelationalOp::EQ;
    } else if (s == "!=") {
        return AST::RelationalOp::NE;
    } else if (s == ">=") {
        return AST::RelationalOp::GE;
    } else if (s == ">") {
        return AST::RelationalOp::GT;
    } else if (s == "and") {
        return AST::RelationalOp::AND;
    } else if (s == "or") {
        return AST::RelationalOp::OR;
    } else if (s == "in") {
        return AST::RelationalOp::IN;
    } else if (s == "not in") {
        return AST::RelationalOp::NOT_IN;
    }
    assert(false);
}

class Relational {
  public:
    Relational(ExpressionV && l, const std::string & o, ExpressionV && r)
        : lhs{std::move(l)}, op{to_relop(o)}, rhs{std::move(r)} {};
    Relational(Relational && a) noexcept : lhs{std::move(a.lhs)}, op{std::move(a.op)}, rhs{std::move(a.rhs)} {};
    Relational(const Relational &) = delete;
    ~Relational(){};

    std::string as_string() const;

    ExpressionV lhs;
    RelationalOp op;
    ExpressionV rhs;
};

// XXX: this isn't really true, it's really an identifier : expressionv
using KeywordPair = std::tuple<ExpressionV, ExpressionV>;
using KeywordList = std::vector<KeywordPair>;

class Arguments {
  public:
    Arguments() : positional{} {};
    Arguments(ExpressionList && v) : positional{std::move(v)}, keyword{} {};
    Arguments(KeywordList && k) : positional{}, keyword{std::move(k)} {};
    Arguments(ExpressionList && v, KeywordList && k) : positional{std::move(v)}, keyword{std::move(k)} {};
    Arguments(Arguments && a) noexcept : positional{std::move(a.positional)}, keyword{std::move(a.keyword)} {};
    Arguments(const Arguments &) = delete;
    ~Arguments(){};

    std::string as_string() const;

    ExpressionList positional;
    KeywordList keyword;
};

class FunctionCall {
  public:
    FunctionCall(ExpressionV && i, std::unique_ptr<Arguments> && a) : id{std::move(i)}, args{std::move(a)} {};
    FunctionCall(FunctionCall && a) noexcept : id{std::move(a.id)}, args{std::move(a.args)} {};
    FunctionCall(const FunctionCall &) = delete;
    ~FunctionCall(){};

    std::string as_string() const;

    ExpressionV id;
    std::unique_ptr<Arguments> args;
};

class MethodCall {
  public:
    MethodCall(ExpressionV && o, ExpressionV && i, std::unique_ptr<Arguments> && a)
        : object{std::move(o)}, id{std::move(i)}, args{std::move(a)} {};
    MethodCall(MethodCall && a) noexcept : object{std::move(a.object)}, id{std::move(a.id)}, args{std::move(a.args)} {};
    MethodCall(const MethodCall &) = delete;
    ~MethodCall(){};

    std::string as_string() const;

    ExpressionV object;
    ExpressionV id;
    std::unique_ptr<Arguments> args;
};

class Array {
  public:
    Array() : elements{} {};
    Array(ExpressionList && e) : elements{std::move(e)} {};
    Array(Array && a) noexcept : elements{std::move(a.elements)} {};
    Array(const Array &) = delete;
    ~Array(){};

    std::string as_string() const;

    ExpressionList elements;
};

class Dict {
  public:
    Dict() : elements{} {};
    Dict(KeywordList && l);
    Dict(Dict && a) : elements{std::move(a.elements)} {};
    Dict(const Dict &) = delete;
    ~Dict(){};

    std::string as_string() const;

    std::unordered_map<ExpressionV, ExpressionV> elements;
};

class Statement {
  public:
    Statement(ExpressionV && e) : expr{std::move(e)} {};
    Statement(Statement && a) noexcept : expr{std::move(a.expr)} {};
    Statement(const Statement &) = delete;
    ~Statement(){};

    std::string as_string() const;

    ExpressionV expr;
};

using StatementV = std::variant<std::unique_ptr<Statement>>;

class CodeBlock {
  public:
    CodeBlock() : statements{} {};
    CodeBlock(StatementV && stmt) : statements{} {
        statements.emplace_back(std::move(stmt));
    };
    CodeBlock(CodeBlock && b) noexcept : statements{std::move(b.statements)} {};
    CodeBlock(const CodeBlock &) = delete;
    ~CodeBlock(){};

    std::string as_string() const;

    // XXX: this should probably be a statement list
    std::vector<StatementV> statements;
};

} // namespace Frontend::AST
