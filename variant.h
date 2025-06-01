#ifndef CAMELLIA_VARIANT_H
#define CAMELLIA_VARIANT_H

#include "camellia_typedef.h"
#include <array>
#include <format>
#include <variant>
#include <vector>

namespace camellia {

#define DEF_VECTOR_COMMON_OPS(X)                                                                                                                               \
    bool operator==(const vector##X &other) const;                                                                                                             \
    bool operator!=(const vector##X &other) const;                                                                                                             \
                                                                                                                                                               \
    [[nodiscard]] bool approx_equals(const vector##X &other) const

struct vector2 {
    DEF_VECTOR_COMMON_OPS(2);

    vector2(number_t x, number_t y);

    [[nodiscard]] number_t get_x() const;

    [[nodiscard]] number_t get_y() const;

    void set_x(number_t x);

    void set_y(number_t y);

#ifndef SWIG
    std::array<number_t, 2> dim;
#endif
};

struct vector3 {
    DEF_VECTOR_COMMON_OPS(3);

    vector3(number_t x, number_t y, number_t z);

    [[nodiscard]] number_t get_x() const;

    [[nodiscard]] number_t get_y() const;

    [[nodiscard]] number_t get_z() const;

    void set_x(number_t x);

    void set_y(number_t y);

    void set_z(number_t z);

#ifndef SWIG
    std::array<number_t, 3> dim;
#endif
};

struct vector4 {
    DEF_VECTOR_COMMON_OPS(4);

    vector4(number_t x, number_t y, number_t z, number_t w);

    [[nodiscard]] number_t get_x() const;

    [[nodiscard]] number_t get_y() const;

    [[nodiscard]] number_t get_w() const;

    [[nodiscard]] number_t get_z() const;

    void set_x(number_t x);

    void set_y(number_t y);

    void set_z(number_t z);

    void set_w(number_t w);

#ifndef SWIG
    std::array<number_t, 4> dim;
#endif
};

class variant {
public:
    enum types : char { ERROR = -1, VOID, INTEGER, NUMBER, BOOLEAN, TEXT, VECTOR2, VECTOR3, VECTOR4, BYTES, ARRAY, ATTRIBUTE };

    [[nodiscard]] types get_value_type() const;
    bool operator==(const variant &other) const;
    bool operator!=(const variant &other) const;
    variant &operator=(const variant &v);
    variant();
    ~variant() = default;
    explicit operator integer_t() const;
    explicit operator number_t() const;
    explicit operator boolean_t() const;
    explicit operator hash_t() const;
    [[nodiscard]] const text_t &get_text() const;
    [[nodiscard]] const vector2 &get_vector2() const;
    [[nodiscard]] const vector3 &get_vector3() const;
    [[nodiscard]] const vector4 &get_vector4() const;
    [[nodiscard]] const bytes_t &get_bytes() const;
    [[nodiscard]] const std::vector<variant> &get_array() const;
    [[nodiscard]] bool approx_equals(const variant &other) const;

    // Descriptor conversion functions
    static variant from_desc(const text_t &descriptor);
    [[nodiscard]] text_t to_desc() const;

    constexpr static char VOID_PREFIX = 'V';
    constexpr static char INTEGER_PREFIX = 'I';
    constexpr static char NUMBER_PREFIX = 'N';
    constexpr static char BOOLEAN_PREFIX = 'Z';
    constexpr static char TEXT_PREFIX = 'T';
    constexpr static char ERROR_PREFIX = 'E';
    constexpr static char VECTOR2_PREFIX = '2';
    constexpr static char VECTOR3_PREFIX = '3';
    constexpr static char VECTOR4_PREFIX = '4';
    constexpr static char BYTES_PREFIX = 'B';
    constexpr static char ARRAY_PREFIX = '[';
    constexpr static char ATTRIBUTE_PREFIX = 'A';

    constexpr static char INTEGER_BINARY_SUFFIX = 'B';
    constexpr static char INTEGER_OCTAL_SUFFIX = 'O';
    constexpr static char INTEGER_DECIMAL_SUFFIX = 'D';
    constexpr static char INTEGER_HEXADECIMAL_SUFFIX = 'H';

    constexpr static char ARRAY_SEPARATOR = ',';
    constexpr static char ARRAY_SUFFIX = ']';

    constexpr static char VECTOR_SEPARATOR = ',';

    constexpr static char ESCAPE_CHAR = '\\';

#ifdef SWIG
   variant(integer_t i);
   variant(number_t n);
   variant(boolean_t b);
   variant(const text_t &t, boolean_t is_error = false);
   variant(const vector2 &v);
   variant(const vector3 &v);
   variant(const vector4 &v);
   variant(const bytes_t &b);
   variant(const std::vector<variant> &a);
   variant(hash_t h);
#else
    variant(const variant &v) = default;
    variant &operator=(variant &&v) noexcept;
    variant(variant &&v) noexcept;

    // Using std::variant instead of union
    using variant_storage = std::variant<std::monostate,       // VOID
                                         integer_t,            // INTEGER
                                         number_t,             // NUMBER
                                         boolean_t,            // BOOLEAN
                                         text_t,               // TEXT or ERROR
                                         vector2,              // VECTOR2
                                         vector3,              // VECTOR3
                                         vector4,              // VECTOR4
                                         bytes_t,              // BYTES
                                         std::vector<variant>, // ARRAY
                                         hash_t                // ATTRIBUTE
                                         >;

    explicit(false) variant(integer_t i);
    explicit(false) variant(number_t n);
    explicit(false) variant(boolean_t b);
    explicit(false) variant(const char *c, boolean_t is_error = false);
    explicit(false) variant(const text_t &t, boolean_t is_error = false);
    explicit(false) variant(text_t &&t, boolean_t is_error = false);
    explicit(false) variant(const vector2 &v);
    explicit(false) variant(const vector3 &v);
    explicit(false) variant(const vector4 &v);
    explicit(false) variant(const bytes_t &b);
    explicit(false) variant(bytes_t &&b);
    explicit(false) variant(const std::vector<variant> &a);
    explicit(false) variant(std::vector<variant> &&a);
    explicit(false) variant(hash_t h);

private:
    types _type;
    variant_storage _data;

    explicit variant(types t);
#endif
};

} // namespace camellia

#ifndef SWIG
template <> struct std::formatter<camellia::variant::types> {
    static constexpr auto parse(const std::format_parse_context &ctx) { return ctx.begin(); }

    auto format(const camellia::variant::types t, std::format_context &ctx) const {
        std::string s;
        switch (t) {
        case camellia::variant::ERROR:
            s = "ERROR";
            break;
        case camellia::variant::VOID:
            s = "VOID";
            break;
        case camellia::variant::INTEGER:
            s = "INTEGER";
            break;
        case camellia::variant::NUMBER:
            s = "NUMBER";
            break;
        case camellia::variant::BOOLEAN:
            s = "BOOLEAN";
            break;
        case camellia::variant::TEXT:
            s = "TEXT";
            break;
        case camellia::variant::VECTOR2:
            s = "VECTOR2";
            break;
        case camellia::variant::VECTOR3:
            s = "VECTOR3";
            break;
        case camellia::variant::VECTOR4:
            s = "VECTOR4";
            break;
        case camellia::variant::BYTES:
            s = "BYTES";
            break;
        case camellia::variant::ARRAY:
            s = "ARRAY";
            break;
        case camellia::variant::ATTRIBUTE:
            s = "ATTRIBUTE";
            break;
        default:
            s = "UNKNOWN";
        }
        return fmt.format(s, ctx);
    }

private:
    std::formatter<std::string> fmt;
};
#endif

#endif // CAMELLIA_VARIANT_H