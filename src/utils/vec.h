#pragma once

#include <memory>

#include "gch/small_vector.hpp"

/// CREDIT: https://github.com/gharveymn/small_vector

template <typename T, unsigned int InlineCapacity = gch::default_buffer_size<std::allocator<T>>::value>
using Vec = gch::small_vector<T, InlineCapacity>;
