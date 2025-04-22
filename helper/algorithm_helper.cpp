//
// Created by LENOVO on 2025/4/2.
//

#include <algorithm>
#include <cmath>
#include "helper/algorithm_helper.h"
#include "variant.h"
#include "xxhash/xxhash.h"

namespace camellia::algorithm_helper {
    boolean_t approx_equals(number_t a, number_t b) {
        auto tolerance = std::max({1.0F, std::fabsf(a), std::fabsf(b)});
        return std::fabsf(a - b) <= std::numeric_limits<float>::epsilon() * tolerance;
    }

    integer_t get_bbcode_string_length(const text_t& bbcode) {
        integer_t res = 0;
        for (int i = 0; i < bbcode.length(); i++) {
            res++;
            if (bbcode[i] != '[') continue;

            auto j = i + 1;
            while (j < bbcode.length() && bbcode[j] != ']') j++;
            if (j >= bbcode.length()) {
                res += j - i - 1;
                break;
            }

            i = j;
        }

        return res;
    }

    std::u16string u8_to_u16(const std::string &str) {
        static std::wstring_convert<std::codecvt_utf8_utf16<char16_t>, char16_t> convert;
        return convert.from_bytes(str.c_str());
    }

    std::string u16_to_u8(const std::u16string &str) {
        static std::wstring_convert<std::codecvt_utf8_utf16<char16_t>, char16_t> convert;
        return convert.to_bytes(str.c_str());
    }

    hash_t calc_hash(const std::string &str) {
        return XXH64(str.data(), str.length(), 11451);
    }
}