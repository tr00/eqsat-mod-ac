#pragma once

#include "database.h"
#include "query.h"

class QueryCompiler
{
  private:
    Database& db;

  public:
    QueryCompiler(Database& db) : db(db)
    {
    }

    void build_indices(Query& query);
};
