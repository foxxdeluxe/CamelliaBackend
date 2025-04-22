//
// Created by LENOVO on 2025/4/2.
//

#ifndef CAMELLIABACKEND_ALGORITHM_HELPER_H
#define CAMELLIABACKEND_ALGORITHM_HELPER_H

#include <string>
#include <locale>
#include <codecvt>
#include <functional>
#include "variant.h"

namespace camellia::algorithm_helper {
    boolean_t approx_equals(number_t a, number_t b);
    integer_t get_bbcode_string_length(const text_t &bbcode);

    template<typename T>
    integer_t compare_to(T a, T b) {
        if (a > b) return 1;
        if (a < b) return -1;
        return 0;
    }

    template<class T>
    integer_t upper_bound(const std::vector<T> &list, std::function<integer_t(const T &)> cmp) {
        integer_t l = 0, r = list.size();

        while (l < r) {
            auto m = (l + r) / 2;
            if (cmp(list[m]) <= 0) l = m + 1;
            else r = m;
        }

        return l;
    }

    template<class T>
    integer_t lower_bound(const std::vector<T> &list, std::function<integer_t(const T &)> cmp) {
        integer_t l = 0, r = list.size();

        while (l < r) {
            auto m = (l + r) / 2;
            if (cmp(list[m]) < 0) l = m + 1;
            else r = m;
        }

        return l;
    }

    hash_t calc_hash(const std::string &str);

#ifndef SWIG
    std::u16string u8_to_u16(const std::string& str);
    std::string u16_to_u8(const std::u16string& str);
#endif
}


#endif //CAMELLIABACKEND_ALGORITHM_HELPER_H
