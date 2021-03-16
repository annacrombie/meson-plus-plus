// SPDX-license-identifier: Apache-2.0
// Copyright © 2021 Intel Corporation

#include <gtest/gtest.h>
#include <memory>
#include <sstream>
#include <variant>

#include "node.hpp"
#include "parser.yy.hpp"
#include "scanner.hpp"

static std::unique_ptr<Frontend::AST::CodeBlock> parse(const std::string & in) {
    std::istringstream stream{in};
    auto block = std::make_unique<Frontend::AST::CodeBlock>();
    auto scanner = std::make_unique<Frontend::Scanner>(&stream);
    auto parser = std::make_unique<Frontend::Parser>(*scanner, block);
    parser->parse();
    return block;
}

TEST(parser, string) {
    auto block = parse("'foo'");
    ASSERT_EQ(block->statements.size(), 1);
    auto const & stmt = std::get<0>(block->statements[0]);
    ASSERT_TRUE(std::holds_alternative<std::unique_ptr<Frontend::AST::String>>(stmt->expr));
    ASSERT_EQ(stmt->as_string(), "'foo'");
}

TEST(parser, decminal_number) {
    auto block = parse("77");
    ASSERT_EQ(block->statements.size(), 1);
    auto const & stmt = std::get<0>(block->statements[0]);
    ASSERT_TRUE(std::holds_alternative<std::unique_ptr<Frontend::AST::Number>>(stmt->expr));
    ASSERT_EQ(stmt->as_string(), "77");
}

TEST(parser, octal_number) {
    auto block = parse("0o10");
    ASSERT_EQ(block->statements.size(), 1);
    auto const & stmt = std::get<0>(block->statements[0]);
    ASSERT_TRUE(std::holds_alternative<std::unique_ptr<Frontend::AST::Number>>(stmt->expr));
    ASSERT_EQ(stmt->as_string(), "8");
}

TEST(parser, hex_number) {
    auto block = parse("0xf");
    ASSERT_EQ(block->statements.size(), 1);
    auto const & stmt = std::get<0>(block->statements[0]);
    ASSERT_TRUE(std::holds_alternative<std::unique_ptr<Frontend::AST::Number>>(stmt->expr));
    ASSERT_EQ(stmt->as_string(), "15");
}

TEST(parser, identifier) {
    auto block = parse("foo");
    ASSERT_EQ(block->statements.size(), 1);
    auto const & stmt = std::get<0>(block->statements[0]);
    ASSERT_TRUE(std::holds_alternative<std::unique_ptr<Frontend::AST::Identifier>>(stmt->expr));
    ASSERT_EQ(stmt->as_string(), "foo");
}

TEST(parser, multiplication) {
    auto block = parse("5  * 4 ");
    ASSERT_EQ(block->statements.size(), 1);
    auto const & stmt = std::get<0>(block->statements[0]);
    ASSERT_TRUE(std::holds_alternative<std::unique_ptr<Frontend::AST::MultiplicativeExpression>>(stmt->expr));
    ASSERT_EQ(block->as_string(), "5 * 4");
}

TEST(parser, division) {
    auto block = parse("5 / 4 ");
    ASSERT_EQ(block->statements.size(), 1);
    auto const & stmt = std::get<0>(block->statements[0]);
    ASSERT_TRUE(std::holds_alternative<std::unique_ptr<Frontend::AST::MultiplicativeExpression>>(stmt->expr));
    ASSERT_EQ(block->as_string(), "5 / 4");
}

TEST(parser, addition) {
    auto block = parse("5 + 4 ");
    ASSERT_EQ(block->statements.size(), 1);
    auto const & stmt = std::get<0>(block->statements[0]);
    ASSERT_TRUE(std::holds_alternative<std::unique_ptr<Frontend::AST::AdditiveExpression>>(stmt->expr));
    ASSERT_EQ(block->as_string(), "5 + 4");
}

TEST(parser, subtraction) {
    auto block = parse("5 - 4 ");
    ASSERT_EQ(block->statements.size(), 1);
    auto const & stmt = std::get<0>(block->statements[0]);
    ASSERT_TRUE(std::holds_alternative<std::unique_ptr<Frontend::AST::AdditiveExpression>>(stmt->expr));
    ASSERT_EQ(block->as_string(), "5 - 4");
}

TEST(parser, mod) {
    auto block = parse("5 % 4 ");
    ASSERT_EQ(block->statements.size(), 1);
    auto const & stmt = std::get<0>(block->statements[0]);
    ASSERT_TRUE(std::holds_alternative<std::unique_ptr<Frontend::AST::MultiplicativeExpression>>(stmt->expr));
    ASSERT_EQ(block->as_string(), "5 % 4");
}

TEST(parser, unary_negate) {
    auto block = parse("- 5");
    ASSERT_EQ(block->statements.size(), 1);
    auto const & stmt = std::get<0>(block->statements[0]);
    ASSERT_TRUE(std::holds_alternative<std::unique_ptr<Frontend::AST::UnaryExpression>>(stmt->expr));
    ASSERT_EQ(block->as_string(), "-5");
}

TEST(parser, assignment) {
    auto block = parse("x = 5 + 3");
    ASSERT_EQ(block->statements.size(), 1);
    auto const & stmt = std::get<0>(block->statements[0]);
    ASSERT_TRUE(std::holds_alternative<std::unique_ptr<Frontend::AST::Assignment>>(stmt->expr));
    ASSERT_EQ(block->as_string(), "x = 5 + 3");
}

TEST(parser, subscript) {
    auto block = parse("foo[bar + 1]");
    ASSERT_EQ(block->statements.size(), 1);
    auto const & stmt = std::get<0>(block->statements[0]);
    ASSERT_TRUE(std::holds_alternative<std::unique_ptr<Frontend::AST::Subscript>>(stmt->expr));
    ASSERT_EQ(block->as_string(), "foo[bar + 1]");
}

TEST(parser, subexpression) {
    auto block = parse("(4 * (5 + 3))");
    ASSERT_EQ(block->statements.size(), 1);
    auto const & stmt = std::get<0>(block->statements[0]);
    ASSERT_TRUE(std::holds_alternative<std::unique_ptr<Frontend::AST::MultiplicativeExpression>>(stmt->expr));
    // ASSERT_EQ(block->as_string(), "(4 * (5 +3))");
};

class RelationalToStringTests : public ::testing::TestWithParam<std::tuple<std::string, std::string>> {};

TEST_P(RelationalToStringTests, relational) {
    const auto & [input, expected] = GetParam();
    auto block = parse(input);
    ASSERT_EQ(block->statements.size(), 1);
    auto const & stmt = std::get<0>(block->statements[0]);
    ASSERT_TRUE(std::holds_alternative<std::unique_ptr<Frontend::AST::Relational>>(stmt->expr));
    ASSERT_EQ(block->as_string(), expected);
}
INSTANTIATE_TEST_CASE_P(RelationalParsingTests, RelationalToStringTests,
                        ::testing::Values(std::make_tuple("4<3", "4 < 3"), std::make_tuple("4>3", "4 > 3"),
                                          std::make_tuple("0 == true", "0 == true"),
                                          std::make_tuple("0 != true", "0 != true"),
                                          std::make_tuple("x or y", "x or y"), std::make_tuple("x and y", "x and y"),
                                          std::make_tuple("x in y", "x in y"),
                                          std::make_tuple("x not in y", "x not in y")));

class FunctionToStringTests : public ::testing::TestWithParam<std::tuple<std::string, std::string>> {};

TEST_P(FunctionToStringTests, arguments) {
    const std::string input = std::get<0>(GetParam());
    const std::string expected = std::get<1>(GetParam());
    auto block = parse(input);
    auto const & stmt = std::get<0>(block->statements[0]);
    ASSERT_TRUE(std::holds_alternative<std::unique_ptr<Frontend::AST::FunctionCall>>(stmt->expr));
    ASSERT_EQ(block->as_string(), expected);
}

INSTANTIATE_TEST_CASE_P(FunctionParsingTests, FunctionToStringTests,
                        ::testing::Values(std::make_tuple("func(  )", "func()"), std::make_tuple("func(a)", "func(a)"),
                                          std::make_tuple("func(a,b, c)", "func(a, b, c)"),
                                          std::make_tuple("func(x : 'f')", "func(x : 'f')"),
                                          std::make_tuple("func(x : 'f', y : 1)", "func(x : 'f', y : 1)"),
                                          std::make_tuple("func(a, b, x : 'f')", "func(a, b, x : 'f')")));

class MethodToStringTests : public ::testing::TestWithParam<std::tuple<std::string, std::string>> {};

TEST_P(MethodToStringTests, arguments) {
    const auto & [input, expected] = GetParam();
    auto block = parse(input);
    auto const & stmt = std::get<0>(block->statements[0]);
    ASSERT_TRUE(std::holds_alternative<std::unique_ptr<Frontend::AST::MethodCall>>(stmt->expr));
    ASSERT_EQ(block->as_string(), expected);
}

INSTANTIATE_TEST_CASE_P(
    MethodParsingTests, MethodToStringTests,
    ::testing::Values(std::make_tuple("o.m()", "o.m()"),
                      std::make_tuple("meson.get_compiler ( 'cpp' )", "meson.get_compiler('cpp')"),
                      std::make_tuple("meson.get_compiler ( 'cpp', 'c' )", "meson.get_compiler('cpp', 'c')"),
                      std::make_tuple("o.method(x : y, z : 1)", "o.method(x : y, z : 1)"),
                      std::make_tuple("o.method(a, b, x : y, z : 1)", "o.method(a, b, x : y, z : 1)")));


TEST(parser, array) {
    auto block = parse("[1, 2, 3, a, 'b']");
    auto const & stmt = std::get<0>(block->statements[0]);
    ASSERT_TRUE(std::holds_alternative<std::unique_ptr<Frontend::AST::Array>>(stmt->expr));
    ASSERT_EQ(block->as_string(), "[1, 2, 3, a, 'b']");
}
