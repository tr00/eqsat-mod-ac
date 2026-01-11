#pragma once

#include <functional>

#include "../id.h"
#include "../utils/hashmap.h"

namespace eqsat
{

class WrappedHashMapSet
{
  private:
    const void *map;
    bool (*contains_fn)(const void *, id_t);
    size_t (*size_fn)(const void *);
    void (*for_each_fn)(const void *, std::function<void(id_t)>);

  public:
    template <typename V>
    explicit WrappedHashMapSet(const HashMap<id_t, V>& map)
        : map(&map)
        , contains_fn(
              [](const void *ptr, id_t id) -> bool { return static_cast<const HashMap<id_t, V> *>(ptr)->contains(id); })
        , size_fn([](const void *ptr) -> size_t { return static_cast<const HashMap<id_t, V> *>(ptr)->size(); })
        , for_each_fn([](const void *ptr, std::function<void(id_t)> f) {
            for (const auto& [key, _] : *static_cast<const HashMap<id_t, V> *>(ptr))
                f(key);
        })
    {
    }

    bool contains(id_t id) const
    {
        return contains_fn(map, id);
    }

    size_t size() const
    {
        return size_fn(map);
    }

    void for_each(std::function<void(id_t)> f) const
    {
        for_each_fn(map, f);
    }
};

} // namespace eqsat
