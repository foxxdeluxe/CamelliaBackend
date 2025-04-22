//
// Created by LENOVO on 2025/4/1.
//

#ifndef CAMELLIABACKEND_VARIANT_H
#define CAMELLIABACKEND_VARIANT_H
#include <format>
#include <string>
#include <vector>
#include "global.h"

#define TEXT(X) (text_t(X))
#define DEF_VECTOR_COMMON_OPS(X) \
bool operator==(const vector ## X &other) const;   \
bool operator!=(const vector ## X &other) const;  \
\
[[nodiscard]] bool approx_equals(const vector ## X &other) const

namespace camellia {
    struct vector2 {
        DEF_VECTOR_COMMON_OPS(2);

        vector2(number_t x, number_t y);

        [[nodiscard]] number_t get_x() const;

        [[nodiscard]] number_t get_y() const;

        void set_x(number_t x);

        void set_y(number_t y);

#ifndef SWIG
        number_t dim[2];
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
        number_t dim[3];
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

        void set_w(number_t z);

        void set_z(number_t w);

#ifndef SWIG
        number_t dim[4];
#endif
    };

    class variant {
    public:
        enum types {
            ERROR = -1, VOID, INTEGER, NUMBER, BOOLEAN, TEXT, VECTOR2, VECTOR3, VECTOR4, BYTES
        };

        static variant &get_default(types type);

        [[nodiscard]] types get_value_type() const;

        bool operator==(const variant &other) const;

        bool operator!=(const variant &other) const;

        variant &operator=(const variant &v);


        variant();

        variant(const variant &v);

        ~variant();

        explicit operator integer_t() const;

        explicit operator number_t() const;

        explicit operator boolean_t() const;

        [[nodiscard]] const text_t &get_text() const;

        [[nodiscard]] const vector2 &get_vector2() const;

        [[nodiscard]] const vector3 &get_vector3() const;

        [[nodiscard]] const vector4 &get_vector4() const;

        [[nodiscard]] const bytes_t &get_bytes() const;

        [[nodiscard]] bool approx_equals(const variant &other) const;

#ifdef SWIG
       variant(integer_t i);
       variant(number_t n);
       variant(boolean_t b);
       variant(const text_t &t, boolean_t is_error = false);
       variant(const vector2 &v);
       variant(const vector3 &v);
       variant(const vector4 &v);
       variant(const bytes_t &b);
#else
        variant &operator=(variant &&v) noexcept;
        variant(variant &&v) noexcept;

        union obj_union {
            integer_t integer;
            number_t number;
            boolean_t boolean;

            text_t *p_text;
            vector2 *p_vector2;
            vector3 *p_vector3;
            vector4 *p_vector4;
            bytes_t *p_bytes;
        };

        explicit(false) variant(integer_t i);
        explicit(false) variant(number_t n);
        explicit(false) variant(boolean_t b);
        explicit(false) variant(const text_t &t, boolean_t is_error = false);
        explicit(false) variant(text_t &&t, boolean_t is_error = false);
        explicit(false) variant(const vector2 &v);
        explicit(false) variant(const vector3 &v);
        explicit(false) variant(const vector4 &v);
        explicit(false) variant(const bytes_t &b);
        explicit(false) variant(bytes_t &&b);

    private:
        types _type;
        obj_union _obj{};

        explicit variant(types t);

#endif
    };
}

#ifndef SWIG
template<>
struct std::formatter<camellia::variant::types>
{
    static constexpr auto parse(const std::format_parse_context& ctx)
    {
        return ctx.begin();
    }

    auto format(const camellia::variant::types t, std::format_context& ctx) const
    {
        std::string s;
        switch (t)
        {
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
        default:
            s = t;
        }
        return fmt.format(s, ctx);
    }

private:
    std::formatter<std::string> fmt;
};
#endif

#endif //CAMELLIABACKEND_VARIANT_H
