#pragma once

#include "query.h"
#include "database.h"

class QueryCompiler {
private:

    Database& db;

public:

    QueryCompiler(Database& db) : db(db) {}

    void build_indices(Query& query);
};
