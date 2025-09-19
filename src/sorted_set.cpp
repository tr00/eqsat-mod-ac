#include "sorted_set.h"

SortedSet::SortedSet() {}

bool SortedSet::insert(id_t id) {
    auto it = std::lower_bound(data.begin(), data.end(), id);

    if (it != data.end() && *it == id) {
        return false;
    }

    data.insert(it, id);
    return true;
}

bool SortedSet::contains(id_t id) const {
    auto it = std::lower_bound(data.begin(), data.end(), id);
    return it != data.end() && *it == id;
}

size_t SortedSet::size() const {
    return data.size();
}

bool SortedSet::empty() const {
    return data.empty();
}

void SortedSet::clear() {
    data.clear();
}

std::vector<id_t>::const_iterator SortedSet::begin() const {
    return data.begin();
}

std::vector<id_t>::const_iterator SortedSet::end() const {
    return data.end();
}

const std::vector<id_t>& SortedSet::get_data() const {
    return data;
}
