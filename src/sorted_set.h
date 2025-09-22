#pragma once

#include <vector>

#include "id.h"

class SortedSet {
private:
    std::vector<id_t> data;

public:
    SortedSet();

    bool insert(id_t id);
    bool contains(id_t id) const;
    size_t size() const;
    bool empty() const;
    void clear();

    std::vector<id_t>::const_iterator begin() const;
    std::vector<id_t>::const_iterator end() const;

    const std::vector<id_t>& get_data() const;
};
