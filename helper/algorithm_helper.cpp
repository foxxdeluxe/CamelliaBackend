//
// Created by LENOVO on 2025/4/2.
//

#include "algorithm_helper.h"
#include "camellia.h"
#include "xxHash/xxhash.h"
#include <algorithm>
#include <cmath>
#include <cstring>

namespace camellia::algorithm_helper {
constexpr hash_t XXHASH_SEED = 431134; // AELLEA

boolean_t approx_equals(number_t a, number_t b) {
    auto tolerance = std::max({1.0F, std::fabsf(a), std::fabsf(b)});
    return std::fabsf(a - b) <= std::numeric_limits<float>::epsilon() * tolerance;
}

integer_t get_bbcode_string_length(const text_t &bbcode) {
    integer_t res = 0;
    for (int i = 0; i < bbcode.length(); i++) {
        res++;
        if (bbcode[i] != '[') {
            continue;
        }

        auto j = i + 1;
        while (j < bbcode.length() && bbcode[j] != ']') {
            j++;
        }
        if (j >= bbcode.length()) {
            res += j - i - 1;
            break;
        }

        i = j;
    }

    return res;
}

hash_t calc_hash(const std::string &str) noexcept {
    auto hash = XXH3_64bits_withSeed(str.data(), str.size(), XXHASH_SEED);
    if (hash >= RESERVE_SIZE) [[likely]] {
        return hash;
    }

    // Jackpot!!!
    for (auto seed = XXHASH_SEED + 1; hash < RESERVE_SIZE; seed++) {
        hash = XXH3_64bits_withSeed(str.data(), str.size(), seed);
    }
    return hash;
}

hash_t calc_hash(const char *str) noexcept {
    auto len = std::strlen(str);
    auto hash = XXH3_64bits_withSeed(str, len, XXHASH_SEED);
    if (hash >= RESERVE_SIZE) [[likely]] {
        return hash;
    }

    // Jackpot!!!
    for (auto seed = XXHASH_SEED + 1; hash < RESERVE_SIZE; seed++) {
        hash = XXH3_64bits_withSeed(str, len, seed);
    }
    return hash;
}
} // namespace camellia::algorithm_helper