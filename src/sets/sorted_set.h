#pragma once

#include <vector>

#include "../id.h"

class SortedVecSet {
private:
    std::vector<id_t> data;

public:
    SortedVecSet() {};


    bool insert(id_t id);

    bool contains(id_t id) const {
        auto it = std::lower_bound(data.begin(), data.end(), id);
        return it != data.end() && *it == id;
    }

    size_t size() const {
        return data.size();
    }

    bool empty() const {
        return data.empty();
    }

    void clear() {
        data.clear();
    }

    std::vector<id_t>::const_iterator begin() const {
        return data.begin();
    }

    std::vector<id_t>::const_iterator end() const {
        return data.end();
    }

    template<typename Func>
    void for_each(Func f) const {
        for (auto it = begin(); it != end(); ++it)
            f(*it);
    }
};
