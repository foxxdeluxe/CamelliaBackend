#ifndef ALGORITHM_HELPER_H
#define ALGORITHM_HELPER_H

#include "../camellia_typedef.h"
#include "constexpr-xxh3.h"
#include <functional>
#include <vector>

namespace camellia::algorithm_helper {
constexpr hash_t RESERVE_SIZE = 0x8000ULL;
constexpr hash_t XXHASH_SEED = 431134ULL; // AELLEA

boolean_t approx_equals(number_t a, number_t b);
integer_t get_bbcode_string_length(const text_t &bbcode);
hash_t calc_hash(const std::string &str) noexcept;
consteval hash_t calc_hash_const(std::string_view str) noexcept {
    auto hash = constexpr_xxh3::XXH3_64bits_withSeed_const(str.data(), str.size(), XXHASH_SEED);
    if (hash >= RESERVE_SIZE) [[likely]] {
        return hash;
    }

    // Jackpot!!!
    for (auto seed = XXHASH_SEED + 1; hash < RESERVE_SIZE; seed++) {
        hash = constexpr_xxh3::XXH3_64bits_withSeed_const(str.data(), str.size(), seed);
    }
    return hash;
}

template <typename T> integer_t compare_to(T a, T b) {
    if (a > b) {
        return 1;
    }
    if (a < b) {
        return -1;
    }
    return 0;
}

template <class T> integer_t upper_bound(const std::vector<T> &list, std::function<integer_t(const T &)> cmp) {
    integer_t l = 0;
    integer_t r = list.size();

    while (l < r) {
        auto m = (l + r) / 2;
        if (cmp(list[m]) <= 0) {
            l = m + 1;
        } else {
            r = m;
        }
    }

    return l;
}

template <class T> integer_t lower_bound(const std::vector<T> &list, std::function<integer_t(const T &)> cmp) {
    integer_t l = 0;
    integer_t r = list.size();

    while (l < r) {
        auto m = (l + r) / 2;
        if (cmp(list[m]) < 0) {
            l = m + 1;
        } else {
            r = m;
        }
    }

    return l;
}

hash_t calc_hash(const char *str) noexcept;

} // namespace camellia::algorithm_helper

#endif // ALGORITHM_HELPER_H
