#include "egraph.h"

id_t EGraph::insert_term(std::shared_ptr<Expression> expression) {
    // TODO: Implement term insertion
    return 0;
}

id_t EGraph::unify(id_t a, id_t b) {
    id_t id = uf.unify(a, b);

    worklist.push_back(id);

    return id;
}
