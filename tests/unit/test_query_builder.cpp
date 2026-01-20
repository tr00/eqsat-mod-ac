#include <catch2/catch_test_macros.hpp>

#include "query.h"
#include "query_builder.h"
#include "theory.h"

using namespace eqsat;
using namespace eqsat::test;

TEST_CASE("QueryBuilder - basic construction", "[query_builder]")
{
    Theory theory;
    Symbol add = theory.add_operator("add", 2);
    Symbol mul = theory.add_operator("mul", 2);

    SECTION("Empty query with name")
    {
        auto query = QueryBuilder(theory, "test_query").build();

        REQUIRE(query.name == theory.intern("test_query"));
        REQUIRE(query.constraints.size() == 0);
        REQUIRE(query.head.size() == 0);
    }

    SECTION("Query with single constraint")
    {
        auto query =
            QueryBuilder(theory, "single_constraint").with_constraint(add, {0, 1, 2}).with_head_vars({0, 1, 2}).build();

        REQUIRE(query.constraints.size() == 1);
        REQUIRE(query.constraints[0].symbol == add);
        REQUIRE(query.constraints[0].variables.size() == 3);
        REQUIRE(query.constraints[0].variables[0] == 0);
        REQUIRE(query.constraints[0].variables[1] == 1);
        REQUIRE(query.constraints[0].variables[2] == 2);

        REQUIRE(query.head.size() == 3);
        REQUIRE(query.head[0] == 0);
        REQUIRE(query.head[1] == 1);
        REQUIRE(query.head[2] == 2);
    }

    SECTION("Query with multiple constraints")
    {
        auto query = QueryBuilder(theory, "multi_constraint")
                         .with_constraint(add, {0, 1, 2})
                         .with_constraint(mul, {2, 3, 4})
                         .with_head_vars({0, 1, 2, 3, 4})
                         .build();

        REQUIRE(query.constraints.size() == 2);
        REQUIRE(query.constraints[0].symbol == add);
        REQUIRE(query.constraints[1].symbol == mul);

        REQUIRE(query.head.size() == 5);
    }

    SECTION("Query with Vec parameter")
    {
        Vec<var_t> vars = {0, 1, 2};
        auto query = QueryBuilder(theory, "vec_test").with_constraint(add, vars).with_head_vars(vars).build();

        REQUIRE(query.constraints.size() == 1);
        REQUIRE(query.head.size() == 3);
    }

    SECTION("Query with individual head vars")
    {
        auto query = QueryBuilder(theory, "individual_heads")
                         .with_constraint(add, {0, 1, 2})
                         .with_head_var(0)
                         .with_head_var(1)
                         .with_head_var(2)
                         .build();

        REQUIRE(query.head.size() == 3);
        REQUIRE(query.head[0] == 0);
        REQUIRE(query.head[1] == 1);
        REQUIRE(query.head[2] == 2);
    }

    SECTION("Method chaining works correctly")
    {
        // This should compile and work without issues
        auto query = QueryBuilder(theory, "chaining")
                         .with_constraint(add, {0, 1, 2})
                         .with_constraint(mul, {2, 3, 4})
                         .with_head_var(0)
                         .with_head_var(4)
                         .build();

        REQUIRE(query.constraints.size() == 2);
        REQUIRE(query.head.size() == 2);
    }
}

TEST_CASE("QueryBuilder - with Constraint object", "[query_builder]")
{
    Theory theory;
    Symbol add = theory.add_operator("add", 2);

    SECTION("Add pre-constructed Constraint")
    {
        Constraint constraint(add, {0, 1, 2});

        auto query =
            QueryBuilder(theory, "constraint_obj").with_constraint(constraint).with_head_vars({0, 1, 2}).build();

        REQUIRE(query.constraints.size() == 1);
        REQUIRE(query.constraints[0].symbol == add);
    }
}

TEST_CASE("QueryBuilder - direct symbol construction", "[query_builder]")
{
    Theory theory;
    Symbol name = theory.intern("direct_symbol");

    SECTION("Construct with Symbol directly")
    {
        auto query = QueryBuilder(name).build();

        REQUIRE(query.name == name);
        REQUIRE(query.constraints.size() == 0);
        REQUIRE(query.head.size() == 0);
    }
}

TEST_CASE("QueryBuilder - get reference to query", "[query_builder]")
{
    Theory theory;
    Symbol add = theory.add_operator("add", 2);

    SECTION("Get non-const reference")
    {
        QueryBuilder builder(theory, "get_test");
        builder.with_constraint(add, {0, 1, 2});

        Query& query_ref = builder.get();
        REQUIRE(query_ref.constraints.size() == 1);

        // Can modify through reference
        query_ref.add_head_var(5);

        auto query = builder.build();
        REQUIRE(query.head.size() == 1);
        REQUIRE(query.head[0] == 5);
    }

    SECTION("Get const reference")
    {
        QueryBuilder builder(theory, "const_get_test");
        builder.with_constraint(add, {0, 1, 2});

        const QueryBuilder& const_builder = builder;
        const Query& query_ref = const_builder.get();

        REQUIRE(query_ref.constraints.size() == 1);
    }
}
