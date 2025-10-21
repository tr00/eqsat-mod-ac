/**
 *
 * WyHash -- https://github.com/wangyi-fudan/wyhash
 *
 *
 * This is free and unencumbered software released into the public domain.
 *
 * Anyone is free to copy, modify, publish, use, compile, sell, or
 * distribute this software, either in source code form or as a compiled
 * binary, for any purpose, commercial or non-commercial, and by any
 * means.
 *
 * In jurisdictions that recognize copyright laws, the author or authors
 * of this software dedicate any and all copyright interest in the
 * software to the public domain. We make this dedication for the benefit
 * of the public at large and to the detriment of our heirs and
 * successors. We intend this dedication to be an overt act of
 * relinquishment in perpetuity of all present and future rights to this
 * software under copyright law.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
 * IN NO EVENT SHALL THE AUTHORS BE LIABLE FOR ANY CLAIM, DAMAGES OR
 * OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE,
 * ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
 * OTHER DEALINGS IN THE SOFTWARE.
 *
 * For more information, please refer to <http://unlicense.org/>
 */

#pragma once

#include "ankerl/unordered_dense.h"

namespace eqsat
{

#define SEED 0x9E3779B97F4A7C15UL

/// CREDIT: https://github.com/wangyi-fudan/wyhash
[[nodiscard]] inline uint64_t hash64(uint64_t x) noexcept
{
    return ankerl::unordered_dense::detail::wyhash::hash(x);
}

[[nodiscard]] inline uint64_t mix64(uint64_t a, uint64_t b) noexcept
{
    return ankerl::unordered_dense::detail::wyhash::mix(a, b);
}

[[nodiscard]] inline uint32_t mix32(uint32_t a, uint32_t b) noexcept
{
    uint64_t c = 1;
    c *= a ^ 0x53c5ca59u;
    c *= b ^ 0x74743c1bu;
    a = c;
    b = c >> 32;
    return a ^ b;
}

} // namespace eqsat
