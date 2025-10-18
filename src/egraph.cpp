#include <cassert>
#include <cstddef>
#include <iostream>
#include <stdexcept>

#include "compiler.h"
#include "egraph.h"
#include "engine.h"
#include "id.h"

EGraph::EGraph(const Theory& theory) : theory(theory)
{

    // initialize database with one relation per operator
    for (const auto& [symbol, arity] : theory.operators)
    {
        if (arity == AC)
            db.create_relation_ac(symbol);
        else
            db.create_relation(symbol, arity + 1); // +1 for the id
    }

    // compile rewrite rule patterns to queries
    if (!theory.rewrite_rules.empty())
    {
        Compiler compiler;

        auto kernels = compiler.compile_many(theory.rewrite_rules);

        for (auto [query, subst] : kernels)
        {
            queries.push_back(query);
            substs.push_back(subst);
        }

        // create required indices for compiled queries
        for (const auto& query : queries)
        {
            auto required = query.get_required_indices();
            for (const auto& [op_symbol, perm] : required)
            {
                if (!db.has_index(op_symbol, perm))
                {
                    db.create_index(op_symbol, perm);
                    required_indices.push_back({op_symbol, perm});
                }
            }
        }
    }
}

id_t EGraph::add_expr(std::shared_ptr<Expr> expr)
{
    if (expr->is_variable())
        throw std::runtime_error("Cannot insert pattern variables into e-graph");

    // Recursively insert children and collect their ids
    Vec<id_t> child_ids;
    child_ids.reserve(expr->children.size());

    for (const auto& child : expr->children)
    {
        id_t child_id = add_expr(child);
        child_ids.push_back(child_id);
    }

    return add_enode(expr->symbol, std::move(child_ids));
}

id_t EGraph::add_enode(Symbol symbol, Vec<id_t> children)
{
    ENode enode(symbol, std::move(children));
    return add_enode(std::move(enode));
}

id_t EGraph::add_enode(ENode enode)
{
    auto it = memo.find(enode);
    if (it != memo.end())
        return it->second;

    ++enodes;
    id_t id = uf.make_set();

    Vec<id_t> tuple = enode.children; // copy
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

void EGraph::apply_matches(const Vec<id_t>& match_vec, Subst& subst)
{
    size_t head_size = subst.head_size;

    assert(!match_vec.empty());
    assert(head_size != 0);

    size_t num_matches = match_vec.size() / head_size;

    for (size_t j = 0; j < num_matches; ++j)
    {
        // Extract match tuple
        Vec<id_t> match(head_size);
        for (size_t k = 0; k < head_size; ++k)
        {
            match[k] = match_vec[j * head_size + k];
        }

        // Create callback for instantiation
        auto callback = [this](Symbol sym, Vec<id_t> children) -> id_t {
            return this->add_enode(sym, std::move(children));
        };

        // Instantiate RHS
        id_t rhs_id = subst.instantiate(callback, match);

        // LHS root is LAST element in match (matches database tuple format)
        id_t lhs_id = match[head_size - 1];

        // Unify
        unify(lhs_id, rhs_id);
    }
}

void EGraph::saturate(std::size_t max_iters)
{
    Engine engine(db);
    HashMap<Symbol, Vec<id_t>> matches;

    for (auto query : queries)
        matches[query.name] = Vec<id_t>();

    for (std::size_t iter = 0; iter < max_iters; ++iter)
    {
        for (const auto& [op_symbol, perm] : required_indices)
            db.create_index(op_symbol, perm);

        db.populate_indices();

        for (const auto& query : queries)
        {
            engine.prepare(query);
            matches[query.name] = std::move(engine.execute());
        }

        for (const auto& [name, match_vec] : matches)
        {
            // Skip empty match vectors (no matches found)
            if (match_vec.empty())
                continue;

            // find substitution with the same name
            for (size_t i = 0; i < substs.size(); ++i)
            {
                if (substs[i].name == name)
                {
                    apply_matches(match_vec, substs[i]);
                    break;
                }
            }
        }

        db.clear_indices();

        std::cout << "iteration: " << iter << "  eclasses: " << uf.size() << "  enodes: " << enodes << std::endl;
    }
}
