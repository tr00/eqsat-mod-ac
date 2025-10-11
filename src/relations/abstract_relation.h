#pragma once

#include <cstddef>

#include "indices/abstract_index.h"
#include "relations/relation_ac.h"
#include "row_store.h"
#include "symbol_table.h"

enum RelationKind
{
    ROW_STORE,
    RELATION_AC,
};

class AbstractRelation
{
  private:
    RelationKind kind;
    union {
        RowStore row_store;
        RelationAC ac_rel;
    };

  public:
    explicit AbstractRelation(RowStore rel) : kind(ROW_STORE), row_store(std::move(rel))
    {
    }

    explicit AbstractRelation(RelationAC rel) : kind(RELATION_AC), ac_rel(std::move(rel))
    {
    }

    ~AbstractRelation()
    {
        switch (kind)
        {
        case ROW_STORE:
            row_store.~RowStore();
            break;
        case RELATION_AC:
            ac_rel.~RelationAC();
            break;
        }
    }

    AbstractRelation(const AbstractRelation&) = delete;
    AbstractRelation& operator=(const AbstractRelation&) = delete;

    AbstractRelation(AbstractRelation&& other) : kind(other.kind)
    {
        switch (kind)
        {
        case ROW_STORE:
            new (&row_store) RowStore(std::move(other.row_store));
            break;
        case RELATION_AC:
            new (&ac_rel) RelationAC(std::move(other.ac_rel));
            break;
        }
    }

    AbstractRelation& operator=(AbstractRelation&& other)
    {
        if (this != &other)
        {
            // Destroy current object
            switch (kind)
            {
            case ROW_STORE:
                row_store.~RowStore();
                break;
            case RELATION_AC:
                ac_rel.~RelationAC();
                break;
            }
            // Move construct new object
            kind = other.kind;
            switch (kind)
            {
            case ROW_STORE:
                new (&row_store) RowStore(std::move(other.row_store));
                break;
            case RELATION_AC:
                new (&ac_rel) RelationAC(std::move(other.ac_rel));
                break;
            }
        }
        return *this;
    }

    Symbol get_operator_symbol() const
    {
        switch (kind)
        {
        case ROW_STORE:
            return row_store.get_operator_symbol();
        case RELATION_AC:
            return ac_rel.get_operator_symbol();
        }
        assert(0);
    }

    size_t size() const
    {
        switch (kind)
        {
        case ROW_STORE:
            return row_store.size();
        case RELATION_AC:
            return ac_rel.size();
        }
        assert(0);
    }

    void add_tuple(const Vec<id_t>& tuple)
    {
        switch (kind)
        {
        case ROW_STORE:
            return row_store.add_tuple(tuple);
        case RELATION_AC:
            return ac_rel.add_tuple(tuple);
        }
        assert(0);
    }

    AbstractIndex build_index(uint32_t vo)
    {
        switch (kind)
        {
        case ROW_STORE:
            return row_store.build_index(vo);
        case RELATION_AC:
            assert(0 && "RelationAC does not support build_index with variable ordering");
        }
        assert(0);
    }

    AbstractIndex build_index()
    {
        switch (kind)
        {
        case ROW_STORE:
            assert(0 && "RowStore requires build_index with variable ordering parameter");
        case RELATION_AC:
            return ac_rel.build_index();
        }
        assert(0);
    }

    // repair?
};
