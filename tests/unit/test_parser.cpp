#include <catch2/catch_test_macros.hpp>
#include <memory>

#include "../src/parser.h"
#include "../src/symbol_table.h"

using namespace eqsat;

TEST_CASE("Parse simple variable patterns", "[parser]")
{
    SymbolTable symbols;
    Parser parser(symbols);

    SECTION("Parse a single variable")
    {
        auto expr = parser.parse_sexpr("?x");
        REQUIRE(expr != nullptr);
        REQUIRE(expr->is_variable());
        REQUIRE(symbols.get_string(expr->symbol) == "x");
    }

    SECTION("Parse variables with different names")
    {
        auto expr1 = parser.parse_sexpr("?a");
        auto expr2 = parser.parse_sexpr("?foo");
        auto expr3 = parser.parse_sexpr("?var123");

        REQUIRE(expr1->is_variable());
        REQUIRE(expr2->is_variable());
        REQUIRE(expr3->is_variable());

        REQUIRE(symbols.get_string(expr1->symbol) == "a");
        REQUIRE(symbols.get_string(expr2->symbol) == "foo");
        REQUIRE(symbols.get_string(expr3->symbol) == "var123");
    }
}

TEST_CASE("Parse nullary operators", "[parser]")
{
    SymbolTable symbols;
    Parser parser(symbols);

    SECTION("Parse simple constant")
    {
        auto expr = parser.parse_sexpr("(zero)");
        REQUIRE(expr != nullptr);
        REQUIRE(expr->is_operator());
        REQUIRE(symbols.get_string(expr->symbol) == "zero");
        REQUIRE(expr->nchildren() == 0);
    }

    SECTION("Parse numeric constants")
    {
        auto expr1 = parser.parse_sexpr("(0)");
        auto expr2 = parser.parse_sexpr("(1)");

        REQUIRE(expr1->is_operator());
        REQUIRE(expr2->is_operator());
        REQUIRE(symbols.get_string(expr1->symbol) == "0");
        REQUIRE(symbols.get_string(expr2->symbol) == "1");
        REQUIRE(expr1->nchildren() == 0);
        REQUIRE(expr2->nchildren() == 0);
    }
}

TEST_CASE("Parse binary operators with variables", "[parser]")
{
    SymbolTable symbols;
    Parser parser(symbols);

    SECTION("Parse (add ?a ?b)")
    {
        auto expr = parser.parse_sexpr("(add ?a ?b)");
        REQUIRE(expr != nullptr);
        REQUIRE(expr->is_operator());
        REQUIRE(symbols.get_string(expr->symbol) == "add");
        REQUIRE(expr->nchildren() == 2);

        // Check first child
        REQUIRE(expr->children[0]->is_variable());
        REQUIRE(symbols.get_string(expr->children[0]->symbol) == "a");

        // Check second child
        REQUIRE(expr->children[1]->is_variable());
        REQUIRE(symbols.get_string(expr->children[1]->symbol) == "b");
    }

    SECTION("Parse (mul ?x ?y)")
    {
        auto expr = parser.parse_sexpr("(mul ?x ?y)");
        REQUIRE(expr->is_operator());
        REQUIRE(symbols.get_string(expr->symbol) == "mul");
        REQUIRE(expr->nchildren() == 2);
        REQUIRE(expr->children[0]->is_variable());
        REQUIRE(expr->children[1]->is_variable());
    }
}

TEST_CASE("Parse nested expressions", "[parser]")
{
    SymbolTable symbols;
    Parser parser(symbols);

    SECTION("Parse (add (mul ?x ?y) ?z)")
    {
        auto expr = parser.parse_sexpr("(add (mul ?x ?y) ?z)");
        REQUIRE(expr != nullptr);
        REQUIRE(expr->is_operator());
        REQUIRE(symbols.get_string(expr->symbol) == "add");
        REQUIRE(expr->nchildren() == 2);

        // First child should be (mul ?x ?y)
        auto mul_expr = expr->children[0];
        REQUIRE(mul_expr->is_operator());
        REQUIRE(symbols.get_string(mul_expr->symbol) == "mul");
        REQUIRE(mul_expr->nchildren() == 2);
        REQUIRE(mul_expr->children[0]->is_variable());
        REQUIRE(symbols.get_string(mul_expr->children[0]->symbol) == "x");
        REQUIRE(mul_expr->children[1]->is_variable());
        REQUIRE(symbols.get_string(mul_expr->children[1]->symbol) == "y");

        // Second child should be ?z
        REQUIRE(expr->children[1]->is_variable());
        REQUIRE(symbols.get_string(expr->children[1]->symbol) == "z");
    }

    SECTION("Parse deeply nested: (f (g (h ?x)))")
    {
        auto expr = parser.parse_sexpr("(f (g (h ?x)))");
        REQUIRE(expr->is_operator());
        REQUIRE(symbols.get_string(expr->symbol) == "f");
        REQUIRE(expr->nchildren() == 1);

        auto g_expr = expr->children[0];
        REQUIRE(g_expr->is_operator());
        REQUIRE(symbols.get_string(g_expr->symbol) == "g");
        REQUIRE(g_expr->nchildren() == 1);

        auto h_expr = g_expr->children[0];
        REQUIRE(h_expr->is_operator());
        REQUIRE(symbols.get_string(h_expr->symbol) == "h");
        REQUIRE(h_expr->nchildren() == 1);
        REQUIRE(h_expr->children[0]->is_variable());
    }
}

TEST_CASE("Parse mixed patterns", "[parser]")
{
    SymbolTable symbols;
    Parser parser(symbols);

    SECTION("Parse (add (one) ?x) - constant and variable")
    {
        auto expr = parser.parse_sexpr("(add (one) ?x)");
        REQUIRE(expr->is_operator());
        REQUIRE(expr->nchildren() == 2);

        REQUIRE(expr->children[0]->is_operator());
        REQUIRE(symbols.get_string(expr->children[0]->symbol) == "one");
        REQUIRE(expr->children[0]->nchildren() == 0);

        REQUIRE(expr->children[1]->is_variable());
        REQUIRE(symbols.get_string(expr->children[1]->symbol) == "x");
    }

    SECTION("Parse (mul (add ?a ?b) (add ?c ?d))")
    {
        auto expr = parser.parse_sexpr("(mul (add ?a ?b) (add ?c ?d))");
        REQUIRE(expr->is_operator());
        REQUIRE(symbols.get_string(expr->symbol) == "mul");
        REQUIRE(expr->nchildren() == 2);

        auto add1 = expr->children[0];
        REQUIRE(add1->is_operator());
        REQUIRE(symbols.get_string(add1->symbol) == "add");
        REQUIRE(add1->nchildren() == 2);

        auto add2 = expr->children[1];
        REQUIRE(add2->is_operator());
        REQUIRE(symbols.get_string(add2->symbol) == "add");
        REQUIRE(add2->nchildren() == 2);
    }
}

TEST_CASE("Parse with whitespace variations", "[parser]")
{
    SymbolTable symbols;
    Parser parser(symbols);

    SECTION("Extra whitespace should be ignored")
    {
        auto expr1 = parser.parse_sexpr("(add ?a ?b)");
        auto expr2 = parser.parse_sexpr("(  add   ?a   ?b  )");
        auto expr3 = parser.parse_sexpr("(add\t?a\n?b)");

        REQUIRE(expr1->is_operator());
        REQUIRE(expr2->is_operator());
        REQUIRE(expr3->is_operator());

        REQUIRE(symbols.get_string(expr1->symbol) == "add");
        REQUIRE(symbols.get_string(expr2->symbol) == "add");
        REQUIRE(symbols.get_string(expr3->symbol) == "add");

        REQUIRE(expr1->nchildren() == 2);
        REQUIRE(expr2->nchildren() == 2);
        REQUIRE(expr3->nchildren() == 2);
    }
}

TEST_CASE("Parse error cases", "[parser]")
{
    SymbolTable symbols;
    Parser parser(symbols);

    SECTION("Empty string throws")
    {
        REQUIRE_THROWS(parser.parse_sexpr(""));
    }

    SECTION("Unmatched parentheses throw")
    {
        REQUIRE_THROWS(parser.parse_sexpr("(add ?a ?b"));
        REQUIRE_THROWS(parser.parse_sexpr("add ?a ?b)"));
        REQUIRE_THROWS(parser.parse_sexpr("(add (mul ?x ?y) ?z"));
    }

    SECTION("Invalid variable syntax throws")
    {
        // Variable must start with ?
        REQUIRE_THROWS(parser.parse_sexpr("x"));
        REQUIRE_THROWS(parser.parse_sexpr("(add x ?y)"));
    }

    SECTION("Empty parentheses throw")
    {
        REQUIRE_THROWS(parser.parse_sexpr("()"));
    }
}

TEST_CASE("Round-trip: parse then to_sexpr", "[parser]")
{
    SymbolTable symbols;
    Parser parser(symbols);

    SECTION("Simple patterns round-trip correctly")
    {
        std::string input1 = "(add ?a ?b)";
        auto expr1 = parser.parse_sexpr(input1);
        std::string output1 = expr1->to_sexpr(symbols);
        REQUIRE(output1 == input1);

        std::string input2 = "(mul (add ?x ?y) ?z)";
        auto expr2 = parser.parse_sexpr(input2);
        std::string output2 = expr2->to_sexpr(symbols);
        REQUIRE(output2 == input2);
    }
}
