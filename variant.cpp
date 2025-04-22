//
// Created by LENOVO on 2025/4/1.
//
#include "variant.h"
#include "helper/algorithm_helper.h"

#define IMPL_VECTOR_COMMON_OPS(X) \
bool vector ## X ::operator==(const vector ## X &other) const {  \
    for (int i = 0; i < X ; i++) {   \
        if (dim[i] != other.dim[i]) return false;   \
    }   \
    return true;    \
}   \
\
bool vector ## X ::operator!=(const vector ## X &other) const {  \
    for (int i = 0; i < X ; i++) {   \
        if (dim[i] != other.dim[i]) return true;    \
    }   \
    return false;   \
}   \
\
bool vector ## X ::approx_equals(const vector ## X &other) const {   \
    for (int i = 0; i < X ; i++) {   \
        if (!algorithm_helper::approx_equals(dim[i], other.dim[i])) return false;   \
    }   \
    return true;    \
} static_assert(true, "")

namespace camellia {
    IMPL_VECTOR_COMMON_OPS(2);

    vector2::vector2(number_t x, number_t y) : dim{x, y} {}

    number_t vector2::get_x() const {
        return dim[0];
    }

    number_t vector2::get_y() const {
        return dim[1];
    }

    void vector2::set_x(number_t x) {
        dim[0] = x;
    }

    void vector2::set_y(number_t y) {
        dim[1] = y;
    }

    IMPL_VECTOR_COMMON_OPS(3);

    vector3::vector3(number_t x, number_t y, number_t z) : dim{x, y, z} {}

    number_t vector3::get_x() const {
        return dim[0];
    }

    number_t vector3::get_y() const {
        return dim[1];
    }

    number_t vector3::get_z() const {
        return dim[2];
    }

    void vector3::set_x(number_t x) {
        dim[0] = x;
    }

    void vector3::set_y(number_t y) {
        dim[1] = y;
    }

    void vector3::set_z(number_t z) {
        dim[2] = z;
    }

    IMPL_VECTOR_COMMON_OPS(4);

    vector4::vector4(number_t x, number_t y, number_t z, number_t w) : dim{x, y, z, w} {}

    number_t vector4::get_x() const {
        return dim[0];
    }

    number_t vector4::get_y() const {
        return dim[1];
    }

    number_t vector4::get_z() const {
        return dim[2];
    }

    number_t vector4::get_w() const {
        return dim[3];
    }

    void vector4::set_x(number_t x) {
        dim[0] = x;
    }

    void vector4::set_y(number_t y) {
        dim[1] = y;
    }

    void vector4::set_z(number_t z) {
        dim[2] = z;
    }

    void vector4::set_w(number_t w) {
        dim[3] = w;
    }


    variant &variant::get_default(types type) {
        static variant def_void{};
        static variant def_number(0.0F);
        static variant def_integer(0);
        static variant def_boolean(false);
        static variant def_text(text_t{});
        static variant def_vector2(vector2(0.0F, 0.0F));
        static variant def_vector3(vector3(0.0F, 0.0F, 0.0F));
        static variant def_vector4(vector4(0.0F, 0.0F, 0.0F, 0.0F));
        static variant def_bytes(bytes_t{});
        static variant def_error(text_t{}, true);

        switch (type) {
            case NUMBER:
                return def_number;
            case INTEGER:
                return def_integer;
            case BOOLEAN:
                return def_boolean;
            case TEXT:
                return def_text;
            case VECTOR3:
                return def_vector3;
            case VECTOR4:
                return def_vector4;
            case BYTES:
                return def_bytes;
            case ERROR:
                return def_error;
            default:        // VOID and others
                return def_void;
        }
    }

    variant::types variant::get_value_type() const {
        return _type;
    }

    bool variant::operator==(const variant &other) const {
        if (_type != other._type) return false;

        switch (_type) {
            case VOID:
                return true;
            case BOOLEAN:
                return _obj.boolean == other._obj.boolean;
            case INTEGER:
                return _obj.integer == other._obj.integer;
            case NUMBER:
                return _obj.number == other._obj.number;
            case TEXT:
            case ERROR:
                return *_obj.p_text == *other._obj.p_text;
            case VECTOR3:
                return *_obj.p_vector3 == *other._obj.p_vector3;
            case VECTOR4:
                return *_obj.p_vector4 == *other._obj.p_vector4;
            case BYTES:
                return *_obj.p_bytes == *other._obj.p_bytes;
            default:        // others
                return false;
        }
    }

    bool variant::operator!=(const variant &other) const {
        return !(*this == other);
    }

    variant &variant::operator=(const variant &v) {
        _type = v._type;
        switch (_type) {
            case VOID:
                break;
            case INTEGER:
            case NUMBER:
            case BOOLEAN:
                _obj = v._obj;
                break;
            case TEXT:
            case ERROR:
                _obj.p_text = new text_t(*v._obj.p_text);
                break;
            case VECTOR2:
                _obj.p_vector2 = new vector2(*v._obj.p_vector2);
                break;
            case VECTOR3:
                _obj.p_vector3 = new vector3(*v._obj.p_vector3);
                break;
            case VECTOR4:
                _obj.p_vector4 = new vector4(*v._obj.p_vector4);
                break;
            case BYTES:
                _obj.p_bytes = new bytes_t(*v._obj.p_bytes);
                break;
        }
        return *this;
    }

    variant &variant::operator=(variant &&v) noexcept {
        this->~variant();

        _obj = v._obj;
        _type = v._type;

        v._type = VOID;

        return *this;
    }

    variant::variant() : variant(VOID) {
        _obj.integer = 0;
    }

    variant::variant(integer_t i) : variant(INTEGER) {
        _obj.integer = i;
    }

    variant::variant(number_t n) : variant(NUMBER) {
        _obj.number = n;
    }

    variant::variant(boolean_t b) : variant(BOOLEAN) {
        _obj.boolean = b;
    }

    variant::variant(const text_t &t, boolean_t is_error) : variant(is_error ? ERROR : TEXT) {
        _obj.p_text = new text_t(t);
    }

    variant::variant(text_t &&t, boolean_t is_error) : variant(is_error ? ERROR : TEXT) {
        _obj.p_text = new text_t(std::move(t));
    }

    variant::variant(const vector2 &v) : variant(VECTOR2) {
        _obj.p_vector2 = new vector2(v);
    }

    variant::variant(const vector3 &v) : variant(VECTOR3) {
        _obj.p_vector3 = new vector3(v);
    }

    variant::variant(const vector4 &v) : variant(VECTOR4) {
        _obj.p_vector4 = new vector4(v);
    }

    variant::variant(const bytes_t &b) : variant(BYTES) {
        _obj.p_bytes = new bytes_t(b);
    }

    variant::variant(bytes_t &&b) : variant(BYTES) {
        _obj.p_bytes = new bytes_t(std::move(b));
    }

    variant::variant(types t) {
        _type = t;
    }

    variant::variant(const variant &v) : variant(v._type) {
        switch (_type) {
            case VOID:
                break;
            case INTEGER:
            case NUMBER:
            case BOOLEAN:
                _obj = v._obj;
                break;
            case TEXT:
            case ERROR:
                _obj.p_text = new text_t(*v._obj.p_text);
                break;
            case VECTOR2:
                _obj.p_vector2 = new vector2(*v._obj.p_vector2);
                break;
            case VECTOR3:
                _obj.p_vector3 = new vector3(*v._obj.p_vector3);
                break;
            case VECTOR4:
                _obj.p_vector4 = new vector4(*v._obj.p_vector4);
                break;
            case BYTES:
                _obj.p_bytes = new bytes_t(*v._obj.p_bytes);
                break;
        }
    }

    variant::variant(variant &&v) noexcept: _type(v._type), _obj(v._obj) {
        v._type = VOID;
    }

    variant::~variant() {
        switch (_type) {
            case VOID:
            case INTEGER:
            case NUMBER:
            case BOOLEAN:
                break;
            case TEXT:
            case ERROR:
                delete _obj.p_text;
                break;
            case VECTOR2:
                delete _obj.p_vector2;
                break;
            case VECTOR3:
                delete _obj.p_vector3;
                break;
            case VECTOR4:
                delete _obj.p_vector4;
                break;
            case BYTES:
                delete _obj.p_bytes;
                break;
        }
    }

    variant::operator integer_t() const {
        return _obj.integer;
    }

    variant::operator number_t() const {
        return _obj.number;
    }

    variant::operator boolean_t() const {
        return _obj.boolean;
    }

    const text_t &variant::get_text() const {
        return *_obj.p_text;
    }

    bool variant::approx_equals(const variant &other) const {
        switch (_type) {
            case NUMBER:
            case INTEGER: {
                if (other._type != NUMBER && other._type != INTEGER) return *this == other;
                auto a = _type == NUMBER ? _obj.number : (number_t) _obj.integer;
                auto b = other._type == NUMBER ? other._obj.number : (number_t) other._obj.integer;
                return algorithm_helper::approx_equals(a, b);
            }
            case VECTOR3:
                return _obj.p_vector3->approx_equals(*other._obj.p_vector3);
            case VECTOR4:
                return _obj.p_vector4->approx_equals(*other._obj.p_vector4);
            default:
                return *this == other;
        }
    }

    const vector2 &variant::get_vector2() const {
        return *_obj.p_vector2;
    }

    const vector3 &variant::get_vector3() const {
        return *_obj.p_vector3;
    }

    const vector4 &variant::get_vector4() const {
        return *_obj.p_vector4;
    }

    const bytes_t &variant::get_bytes() const {
        return *_obj.p_bytes;
    }
}
