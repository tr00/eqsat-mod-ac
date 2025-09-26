#pragma once

#include "indices/abstract_index.h"
#include "row_store.h"
#include "symbol_table.h"
#include <cstddef>
#include <vector>

enum RelationKind
{
    ROW_STORE,
};

class AbstractRelation
{
  private:
    RelationKind kind;
    union {
        RowStore row_store;
    };

  public:
    explicit AbstractRelation(RowStore rel) : kind(ROW_STORE), row_store(std::move(rel))
    {
    }

    ~AbstractRelation()
    {
        switch (kind)
        {
        case ROW_STORE:
            row_store.~RowStore();
            break;
        }
    }

    AbstractRelation(const AbstractRelation &) = delete;
    AbstractRelation &operator=(const AbstractRelation &) = delete;

    AbstractRelation(AbstractRelation &&other) : kind(other.kind)
    {
        switch (kind)
        {
        case ROW_STORE:
            new (&row_store) RowStore(std::move(other.row_store));
            break;
        }
    }

    AbstractRelation &operator=(AbstractRelation &&other)
    {
        if (this != &other)
        {
            // Destroy current object
            switch (kind)
            {
            case ROW_STORE:
                row_store.~RowStore();
                break;
            }
            // Move construct new object
            kind = other.kind;
            switch (kind)
            {
            case ROW_STORE:
                new (&row_store) RowStore(std::move(other.row_store));
                break;
            }
        }
        return *this;
    }

    symbol_t get_operator_symbol() const
    {
        switch (kind)
        {
        case ROW_STORE:
            return row_store.get_operator_symbol();
        }
        assert(0);
    }

    size_t size() const
    {
        switch (kind)
        {
        case ROW_STORE:
            return row_store.size();
        }
        assert(0);
    }

    void add_tuple(const std::vector<id_t> &tuple)
    {
        switch (kind)
        {
        case ROW_STORE:
            return row_store.add_tuple(tuple);
        }
        assert(0);
    }

    AbstractIndex build_index(uint32_t vo)
    {
        switch (kind)
        {
        case ROW_STORE:
            return row_store.build_index(vo);
        }
        assert(0);
    }
};
