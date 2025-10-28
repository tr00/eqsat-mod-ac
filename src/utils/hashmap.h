#pragma once

#include <functional>

#include "ankerl/unordered_dense.h"

/// CREDIT: https://github.com/martinus/unordered_dense

template <typename K, typename V, typename Hash = ankerl::unordered_dense::hash<K>, typename Eq = std::equal_to<K>>
using HashMap = ankerl::unordered_dense::map<K, V, Hash, Eq>;

template <typename K>
using HashSet = ankerl::unordered_dense::set<K>;
