//
// Created by LENOVO on 2025/4/2.
//

#include "camellia.h"
#include "xxHash/xxhash.h"
#include <algorithm>
#include <cmath>


namespace camellia::algorithm_helper {
boolean_t approx_equals(number_t a, number_t b) {
    auto tolerance = std::max({1.0F, std::fabsf(a), std::fabsf(b)});
    return std::fabsf(a - b) <=
           std::numeric_limits<float>::epsilon() * tolerance;
}

integer_t get_bbcode_string_length(const text_t &bbcode) {
    integer_t res = 0;
    for (int i = 0; i < bbcode.length(); i++) {
        res++;
        if (bbcode[i] != '[')
            continue;

        auto j = i + 1;
        while (j < bbcode.length() && bbcode[j] != ']')
            j++;
        if (j >= bbcode.length()) {
            res += j - i - 1;
            break;
        }

        i = j;
    }

    return res;
}

hash_t calc_hash(const std::string &str) {
    return XXH64(str.data(), str.length(), 11451);
}
} // namespace camellia::algorithm_helper