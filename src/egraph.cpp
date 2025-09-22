#include <cassert>
#include <stdexcept>
#include "egraph.h"

EGraph::EGraph(const Theory& theory) : theory(theory) {

    // initialize database with one relation per operator
    for (const auto& [symbol, arity] : theory.signature.operators)
        db.add_relation(symbol, arity + 1);

}

id_t EGraph::insert_term(std::shared_ptr<Expression> expression) {
    if (expression->is_variable()) {
        // Variables are not supported in e-graphs, only concrete terms
        throw std::runtime_error("Cannot insert pattern variables into e-graph");
    }

    // Recursively insert children and collect their ids
    std::vector<id_t> child_ids;
    child_ids.reserve(expression->children.size());

    for (const auto& child : expression->children) {
        id_t child_id = insert_term(child);
        child_ids.push_back(child_id);
    }

    ENode node(expression->symbol, child_ids);

    // check if this e-node already exists in the memo table
    auto it = memo.find(node);
    if (it != memo.end())
        return it->second;

    // Create a new id for this e-node
    id_t new_id = uf.make_set();
    memo[node] = new_id;

    // Add the e-node to the database as a tuple
    // The tuple consists of child_ids followed by the new_id
    std::vector<id_t> tuple = child_ids;
    tuple.push_back(new_id);

    assert(db.has_relation(expression->symbol));

    db.add_tuple(expression->symbol, tuple);

    return new_id;
}

id_t EGraph::unify(id_t a, id_t b) {
    id_t id = uf.unify(a, b);

    worklist.push_back(id);

    return id;
}
