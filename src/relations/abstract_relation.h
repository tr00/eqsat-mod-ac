#pragma once

#include <cstddef>
#include <fstream>

#include "handle.h"
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
    explicit AbstractRelation(RowStore rel)
        : kind(ROW_STORE)
        , row_store(std::move(rel))
    {
    }

    explicit AbstractRelation(RelationAC rel)
        : kind(RELATION_AC)
        , ac_rel(std::move(rel))
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

    AbstractRelation(AbstractRelation&& other)
        : kind(other.kind)
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

    RelationKind get_kind() const
    {
        return kind;
    }

    Symbol get_symbol() const
    {
        switch (kind)
        {
        case ROW_STORE:
            return row_store.get_symbol();
        case RELATION_AC:
            return ac_rel.get_symbol();
        }

        __builtin_unreachable();
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

        __builtin_unreachable();
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

        __builtin_unreachable();
    }

    AbstractIndex populate_index(uint32_t vo)
    {
        switch (kind)
        {
        case ROW_STORE:
            return row_store.populate_index(vo);
        case RELATION_AC:
            return ac_rel.populate_index();
        }

        __builtin_unreachable();
    }

    AbstractIndex create_index()
    {
        switch (kind)
        {
        case ROW_STORE:
            return row_store.create_index();
        case RELATION_AC:
            return ac_rel.create_index();
        }

        __builtin_unreachable();
    }

    bool rebuild(Handle handle)
    {
        switch (kind)
        {
        case ROW_STORE:
            return row_store.rebuild(handle);
        case RELATION_AC:
            return ac_rel.rebuild(handle);
        }

        __builtin_unreachable();
    }

    void dump(std::ofstream& out, const SymbolTable& symbols) const
    {
        switch (kind)
        {
        case ROW_STORE:
            return row_store.dump(out, symbols);
        case RELATION_AC:
            return ac_rel.dump(out, symbols);
        }

        __builtin_unreachable();
    }
};
