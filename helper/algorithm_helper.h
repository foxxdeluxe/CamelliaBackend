#ifndef ALGORITHM_HELPER_H
#define ALGORITHM_HELPER_H

#include "../camellia_typedef.h"
#include "constexpr-xxh3.h"
#include "manager.h"
#include "variant.h"
#include <functional>
#include <memory>
#include <string_view>
#include <unordered_map>
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

struct bbcode {
    struct bbcode_node {
        enum node_type : char { TYPE_TEXT, TYPE_TAG };

        virtual ~bbcode_node() = default;
        [[nodiscard]] virtual node_type get_type() const = 0;
        [[nodiscard]] virtual variant to_variant() const = 0;
        [[nodiscard]] virtual text_t to_text() const = 0;
    };

    struct text_node : public bbcode_node {
        text_t text;
        [[nodiscard]] node_type get_type() const override { return TYPE_TEXT; }
        [[nodiscard]] variant to_variant() const override { return text; }
        [[nodiscard]] static text_node from_variant(const variant &v);
        [[nodiscard]] text_t to_text() const override { return text; }
    };

    struct tag_node : public bbcode_node {
        text_t tag_name;
        std::vector<text_t> params;
        std::vector<std::unique_ptr<bbcode_node>> children;
        [[nodiscard]] node_type get_type() const override { return TYPE_TAG; }
        [[nodiscard]] variant to_variant() const override;
        [[nodiscard]] static tag_node from_variant(const variant &v);
        [[nodiscard]] text_t to_text() const override;
    };

    explicit bbcode(const text_t &text);
    [[nodiscard]] variant to_variant() const;
    [[nodiscard]] static bbcode from_variant(const variant &v);
    [[nodiscard]] text_t to_text() const;

    std::vector<std::unique_ptr<bbcode_node>> root_nodes;

private:
    bbcode() = default;
};

number_t calc_bbcode_node_duration(const bbcode::bbcode_node &node, number_t duration_per_char);
number_t calc_bbcode_duration(const bbcode &bbcode, number_t duration_per_char);

} // namespace camellia::algorithm_helper

#endif // ALGORITHM_HELPER_H
