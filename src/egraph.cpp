#include "egraph.h"
#include "compiler.h"
#include "engine.h"
#include <cassert>
#include <cstddef>
#include <stdexcept>
#include <vector>

EGraph::EGraph(const Theory& theory) : theory(theory)
{

    // initialize database with one relation per operator
    for (const auto& [symbol, arity] : theory.operators)
        db.add_relation(symbol, arity + 1);

    // compile rewrite rule patterns to queries
    if (!theory.rewrite_rules.empty())
    {
        Compiler compiler;

        std::vector<Query> queries = compiler.compile_many(theory.rewrite_rules);

        // queries now contains the compiled patterns
        // (future work: store these for use in saturation)
    }
}

id_t EGraph::add_expr(std::shared_ptr<Expr> expr)
{
    if (expr->is_variable())
        throw std::runtime_error("Cannot insert pattern variables into e-graph");

    // Recursively insert children and collect their ids
    std::vector<id_t> child_ids;
    child_ids.reserve(expr->children.size());

    for (const auto& child : expr->children)
    {
        id_t child_id = add_expr(child);
        child_ids.push_back(child_id);
    }

    return add_enode(expr->symbol, std::move(child_ids));
}

id_t EGraph::add_enode(Symbol symbol, std::vector<id_t> children)
{
    ENode enode(symbol, std::move(children));
    return add_enode(std::move(enode));
}

id_t EGraph::add_enode(ENode enode)
{
    auto it = memo.find(enode);
    if (it != memo.end())
        return it->second;

    id_t id = uf.make_set();

    std::vector<id_t> tuple = enode.children; // copy
    tuple.push_back(id);

    assert(db.has_relation(enode.op));
    db.add_tuple(enode.op, tuple);

    memo[std::move(enode)] = id;
    return id;
}

id_t EGraph::unify(id_t a, id_t b)
{
    id_t id = uf.unify(a, b);

    worklist.push_back(id);

    return id;
}

void EGraph::saturate(std::size_t max_iters)
{
    Engine engine(db);

    for (std::size_t iter = 0; iter < max_iters; ++iter)
    {
        db.build_indices();

        for (const auto& query : queries)
        {
            engine.prepare(query);
            engine.execute();
        }

        // for query in queries
        // res = engine.execute(query, db)
        // push(matches, res)

        // for match in matches
        // match.instantiate()

        // rebuilding
        db.clear_indices();
    }
}
