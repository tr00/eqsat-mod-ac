#pragma once

#include "gch/small_vector.hpp"
#include <ankerl/unordered_dense.h>
#include <cstdint>

using id_t = uint32_t;

template <typename T>
using Vec = gch::small_vector<T>;

template <typename K, typename V>
using HashMap = ankerl::unordered_dense::map<K, V>;
