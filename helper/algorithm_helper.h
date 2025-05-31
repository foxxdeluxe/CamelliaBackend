#ifndef ALGORITHM_HELPER_H
#define ALGORITHM_HELPER_H

#include "../camellia_typedef.h"
#include <functional>
#include <vector>

namespace camellia::algorithm_helper {
constexpr hash_t RESERVE_SIZE = 0x8000;

boolean_t approx_equals(number_t a, number_t b);
integer_t get_bbcode_string_length(const text_t &bbcode);
hash_t calc_hash(const std::string &str) noexcept;

#ifndef SWIG
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
#endif

} // namespace camellia::algorithm_helper

#endif // ALGORITHM_HELPER_H
