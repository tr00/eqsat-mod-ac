#include <algorithm>
#include <cassert>
#include <cstddef>
#include <iostream>
#include <stdexcept>

#include "compiler.h"
#include "egraph.h"
#include "engine.h"
#include "id.h"
#include "parser.h"
#include "utils/hashmap.h"

namespace eqsat
{

EGraph::EGraph(const Theory& theory)
    : theory(theory)
{

    // initialize database with one relation per operator
    for (const auto& [symbol, arity] : theory.operators)
    {
        if (arity == AC)
            db.create_relation_ac(symbol, handle());
        else
            db.create_relation(symbol, arity + 1); // +1 for the id
    }

    // compile rewrite rule patterns to queries
    if (!theory.rewrite_rules.empty())
    {
        Compiler compiler(theory);

        auto kernels = compiler.compile_many(theory.rewrite_rules);

        for (auto [query, subst] : kernels)
        {
            queries.push_back(query);
            substs.push_back(subst);
        }

        // collect required indices for compiled queries
        HashSet<std::pair<Symbol, uint32_t>> index_set;
        for (const auto& query : queries)
        {
            auto required = query.get_required_indices();
            for (auto& [op, perm] : required)
            {
                if (theory.get_arity(op) == AC)
                    perm = 0;
                index_set.insert({op, perm});
            }
        }

        required_indices.reserve(index_set.size());
        for (auto tuple : index_set)
            required_indices.push_back(tuple);
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

id_t EGraph::add_expr(const std::string& expr_str)
{
    Parser parser(theory.symbols);
    auto expr = parser.parse_sexpr(expr_str);
    return add_expr(expr);
}

id_t EGraph::add_enode(Symbol symbol, Vec<id_t> children)
{
    ENode enode(symbol, std::move(children));
    return add_enode(std::move(enode));
}

id_t EGraph::add_enode(ENode enode)
{
    for (auto& child : enode.children)
        child = canonicalize(child);

    if (theory.get_arity(enode.op) == AC)
    {
        std::sort(enode.children.begin(), enode.children.end());
    }

    // lookup if enode already exists
    auto it = memo.find(enode);
    if (it != memo.end())
        return it->second;

    // create new eclass-id
    // and insert into db and memo
    ++enodes;
    id_t id = uf.make_set();

    Vec<id_t> tuple = enode.children; // copy
    tuple.push_back(id);

    assert(db.has_relation(enode.op));
    db.add_tuple(enode.op, tuple);

    memo[std::move(enode)] = id;
    return id;
}

std::optional<id_t> EGraph::lookup(ENode enode) const
{
    for (auto& id : enode.children)
        id = canonicalize(id);

    if (theory.get_arity(enode.op) == AC)
    {
        std::sort(enode.children.begin(), enode.children.end());
    }

    auto it = memo.find(enode);
    return it == memo.end() ? std::nullopt : std::optional<id_t>(it->second);
}

id_t EGraph::unify(id_t a, id_t b)
{
    id_t id = uf.unify(a, b);

    // std::cout << "unifying(" << a << ", " << b << ")\n";

    return id;
}

void EGraph::apply_matches(const Vec<id_t>& matches, Subst& subst, const HashMap<id_t, ENode>& ephemeral_map)
{
    size_t head_size = subst.head_size;

    assert(!matches.empty());
    assert(head_size != 0);
    assert(matches.size() % head_size == 0);

    size_t num_matches = matches.size() / head_size;

    Vec<id_t> match(head_size);
    for (size_t j = 0; j < num_matches; ++j)
    {
        for (size_t k = 0; k < head_size; ++k)
            match[k] = matches[j * head_size + k];

        apply_match(match, subst, ephemeral_map);
    }
}

void EGraph::apply_match(const Vec<id_t>& match, Subst& subst, const HashMap<id_t, ENode>& ephemeral_map)
{
    // Materialize ephemeral IDs in the match vector
    Vec<id_t> materialized_match = match;
    for (id_t& id : materialized_match)
    {
        // Check if ID is ephemeral (MSB set)
        if (id & 0x80000000)
        {
            auto it = ephemeral_map.find(id);
            assert(it != ephemeral_map.end());
            // Materialize: add the enode to get real ID
            id = add_enode(it->second);
        }
    }

    auto callback = [this](Symbol sym, Vec<id_t> children) -> id_t {
        return this->add_enode(sym, std::move(children));
    };

    id_t lhs_id = materialized_match.back(); // root
    id_t rhs_id = subst.instantiate(callback, materialized_match);

    unify(lhs_id, rhs_id);
}

bool EGraph::rebuild()
{
    Vec<std::pair<ENode, id_t>> worklist;
    for (auto& [enode, id] : memo)
    {
        id = canonicalize(id);

        for (auto child : enode.children)
            if (child != canonicalize(child))
                worklist.push_back({enode, id});
    }

    for (auto& [enode, id] : worklist)
    {
        memo.erase(enode);

        for (auto& child : enode.children)
            child = canonicalize(child);

        if (theory.get_arity(enode.op) == AC)
            std::sort(enode.children.begin(), enode.children.end());

        memo.emplace(enode, id);
    }

    return db.rebuild(handle());
}

void EGraph::saturate(std::size_t max_iters)
{
    Engine engine(db, handle());
    HashMap<Symbol, Vec<id_t>> matches;

    for (const auto& query : queries)
        matches[query.name] = Vec<id_t>();

    for (std::size_t iter = 0; iter < max_iters; ++iter)
    {
        for (const auto& [op_symbol, perm] : required_indices)
            db.populate_index(op_symbol, perm);

        // ematching
        for (const auto& query : queries)
            engine.execute(matches[query.name], query);

        for (const auto& [name, match_vec] : matches)
        {
            if (match_vec.empty())
                continue;

            // find substitution with the same name
            const auto name2 = name;
            auto cmp = [name2](const auto& subst) { return subst.name == name2; };
            auto it = std::find_if(substs.begin(), substs.end(), cmp);
            assert(it != substs.end());

            apply_matches(match_vec, *it, engine.get_ephemeral_map());
        }

        db.clear_indices();
        rebuild();
        rebuild();

        std::cout << "iteration: " << iter + 1 << "  eclasses: " << uf.eclasses() << "  enodes: " << db.total_size()
                  << "  memo: " << memo.size() << std::endl;
    }
}

void EGraph::dump_to_file(const std::string& filename) const
{
    std::ofstream out(filename);
    if (!out.is_open())
        throw std::runtime_error("Failed to open file for writing: " + filename);

    out << "====<< E-Graph >>====\n\n";
    out << "   enodes: " << enodes << "\n";
    out << " eclasses: " << uf.eclasses() << "\n\n";

    db.dump_to_file(out, theory.symbols);

    out << "====<< Hash Cons >>====\n\n";

    out << "size: " << memo.size() << "\n\n";

    for (const auto& [enode, eclass] : memo)
    {
        out << "  " << theory.symbols.get_string(enode.op) << "(";

        size_t n = enode.children.size();
        for (size_t i = 0; i < n; ++i)
        {
            out << enode.children[i];
            if (i < n - 1)
                out << ", ";
        }

        out << ") ~~> " << eclass << "\n";
    }

    out << "\n";

    uf.dump_to_file(out);

    out.flush();
    out.close();

    std::cout << "E-graph dumped to: " << filename << std::endl;
}

} // namespace eqsat
