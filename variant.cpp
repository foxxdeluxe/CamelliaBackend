#include "variant.h"
#include "helper/algorithm_helper.h"
#include <variant>

#define IMPL_VECTOR_COMMON_OPS(X)                                                                                                                              \
    bool vector##X ::operator==(const vector##X &other) const {                                                                                                \
        for (int i = 0; i < X; i++) {                                                                                                                          \
            if (dim[i] != other.dim[i])                                                                                                                        \
                return false;                                                                                                                                  \
        }                                                                                                                                                      \
        return true;                                                                                                                                           \
    }                                                                                                                                                          \
                                                                                                                                                               \
    bool vector##X ::operator!=(const vector##X &other) const {                                                                                                \
        for (int i = 0; i < X; i++) {                                                                                                                          \
            if (dim[i] != other.dim[i])                                                                                                                        \
                return true;                                                                                                                                   \
        }                                                                                                                                                      \
        return false;                                                                                                                                          \
    }                                                                                                                                                          \
                                                                                                                                                               \
    bool vector##X ::approx_equals(const vector##X &other) const {                                                                                             \
        for (int i = 0; i < X; i++) {                                                                                                                          \
            if (!algorithm_helper::approx_equals(dim[i], other.dim[i]))                                                                                        \
                return false;                                                                                                                                  \
        }                                                                                                                                                      \
        return true;                                                                                                                                           \
    }                                                                                                                                                          \
    static_assert(true, "")

namespace camellia {
IMPL_VECTOR_COMMON_OPS(2);

vector2::vector2(number_t x, number_t y) : dim{x, y} {}

number_t vector2::get_x() const { return dim[0]; }

number_t vector2::get_y() const { return dim[1]; }

void vector2::set_x(number_t x) { dim[0] = x; }

void vector2::set_y(number_t y) { dim[1] = y; }

IMPL_VECTOR_COMMON_OPS(3);

vector3::vector3(number_t x, number_t y, number_t z) : dim{x, y, z} {}

number_t vector3::get_x() const { return dim[0]; }

number_t vector3::get_y() const { return dim[1]; }

number_t vector3::get_z() const { return dim[2]; }

void vector3::set_x(number_t x) { dim[0] = x; }

void vector3::set_y(number_t y) { dim[1] = y; }

void vector3::set_z(number_t z) { dim[2] = z; }

IMPL_VECTOR_COMMON_OPS(4);

vector4::vector4(number_t x, number_t y, number_t z, number_t w) : dim{x, y, z, w} {}

number_t vector4::get_x() const { return dim[0]; }

number_t vector4::get_y() const { return dim[1]; }

number_t vector4::get_z() const { return dim[2]; }

number_t vector4::get_w() const { return dim[3]; }

void vector4::set_x(number_t x) { dim[0] = x; }

void vector4::set_y(number_t y) { dim[1] = y; }

void vector4::set_z(number_t z) { dim[2] = z; }

void vector4::set_w(number_t w) { dim[3] = w; }

variant::types variant::get_value_type() const { return _type; }

bool variant::operator==(const variant &other) const {
    if (_type != other._type) {
        return false;
    }

    switch (_type) {
    case VOID:
        return true;
    case BOOLEAN:
        return std::get<boolean_t>(_data) == std::get<boolean_t>(other._data);
    case INTEGER:
        return std::get<integer_t>(_data) == std::get<integer_t>(other._data);
    case NUMBER:
        return std::get<number_t>(_data) == std::get<number_t>(other._data);
    case TEXT:
    case ERROR:
        return std::get<text_t>(_data) == std::get<text_t>(other._data);
    case VECTOR2:
        return std::get<vector2>(_data) == std::get<vector2>(other._data);
    case VECTOR3:
        return std::get<vector3>(_data) == std::get<vector3>(other._data);
    case VECTOR4:
        return std::get<vector4>(_data) == std::get<vector4>(other._data);
    case BYTES:
        return std::get<bytes_t>(_data) == std::get<bytes_t>(other._data);
    case ARRAY:
        return std::get<std::vector<variant>>(_data) == std::get<std::vector<variant>>(other._data);
    case ATTRIBUTE:
        return std::get<hash_t>(_data) == std::get<hash_t>(other._data);
    default: // others
        return false;
    }
}

bool variant::operator!=(const variant &other) const { return !(*this == other); }

variant &variant::operator=(const variant &v) {
    if (this == &v) {
        return *this;
    }

    _type = v._type;

    switch (_type) {
    case VOID:
        _data = std::monostate{};
        break;
    case INTEGER:
        _data = std::get<integer_t>(v._data);
        break;
    case NUMBER:
        _data = std::get<number_t>(v._data);
        break;
    case BOOLEAN:
        _data = std::get<boolean_t>(v._data);
        break;
    case TEXT:
    case ERROR:
        _data = std::get<text_t>(v._data);
        break;
    case VECTOR2:
        _data = std::get<vector2>(v._data);
        break;
    case VECTOR3:
        _data = std::get<vector3>(v._data);
        break;
    case VECTOR4:
        _data = std::get<vector4>(v._data);
        break;
    case BYTES:
        _data = std::get<bytes_t>(v._data);
        break;
    case ARRAY:
        _data = std::get<std::vector<variant>>(v._data);
        break;
    case ATTRIBUTE:
        _data = std::get<hash_t>(v._data);
        break;
    }
    return *this;
}

variant &variant::operator=(variant &&v) noexcept {
    _type = v._type;
    _data = std::move(v._data);
    v._type = VOID;
    v._data = std::monostate{};
    return *this;
}

variant::variant() : variant(VOID) {}

variant::variant(integer_t i) : variant(INTEGER) { _data = i; }

variant::variant(number_t n) : variant(NUMBER) { _data = n; }

variant::variant(boolean_t b) : variant(BOOLEAN) { _data = b; }

variant::variant(const char *c, boolean_t is_error) : variant(text_t(c), is_error) {}

variant::variant(const text_t &t, boolean_t is_error) : variant(is_error ? ERROR : TEXT) { _data = t; }

variant::variant(text_t &&t, boolean_t is_error) : variant(is_error ? ERROR : TEXT) { _data = std::move(t); }

variant::variant(const vector2 &v) : variant(VECTOR2) { _data = v; }

variant::variant(const vector3 &v) : variant(VECTOR3) { _data = v; }

variant::variant(const vector4 &v) : variant(VECTOR4) { _data = v; }

variant::variant(const bytes_t &b) : variant(BYTES) { _data = b; }

variant::variant(bytes_t &&b) : variant(BYTES) { _data = std::move(b); }

variant::variant(const std::vector<variant> &a) : variant(ARRAY) { _data = a; }

variant::variant(std::vector<variant> &&a) : variant(ARRAY) { _data = std::move(a); }

variant::variant(hash_t h) : variant(ATTRIBUTE) { _data = h; }

variant::variant(types t) : _type(t) {
    switch (_type) {
    case VOID:
        _data = std::monostate{};
        break;
    case INTEGER:
        _data = integer_t{};
        break;
    case NUMBER:
        _data = number_t{};
        break;
    case BOOLEAN:
        _data = boolean_t{};
        break;
    case TEXT:
    case ERROR:
        _data = text_t{};
        break;
    case VECTOR2:
        _data = vector2(0.0F, 0.0F);
        break;
    case VECTOR3:
        _data = vector3(0.0F, 0.0F, 0.0F);
        break;
    case VECTOR4:
        _data = vector4(0.0F, 0.0F, 0.0F, 0.0F);
        break;
    case BYTES:
        _data = bytes_t{};
        break;
    case ARRAY:
        _data = std::vector<variant>{};
        break;
    case ATTRIBUTE:
        _data = hash_t{};
        break;
    }
}

variant::variant(variant &&v) noexcept : _type(v._type), _data(std::move(v._data)) {
    v._type = VOID;
    v._data = std::monostate{};
}

variant::operator integer_t() const { return std::get<integer_t>(_data); }

variant::operator number_t() const { return std::get<number_t>(_data); }

variant::operator boolean_t() const { return std::get<boolean_t>(_data); }

const text_t &variant::get_text() const { return std::get<text_t>(_data); }

bool variant::approx_equals(const variant &other) const {
    switch (_type) {
    case NUMBER:
    case INTEGER: {
        if (other._type != NUMBER && other._type != INTEGER) {
            return *this == other;
        }
        auto a = _type == NUMBER ? std::get<number_t>(_data) : (number_t)std::get<integer_t>(_data);
        auto b = other._type == NUMBER ? std::get<number_t>(other._data) : (number_t)std::get<integer_t>(other._data);
        return algorithm_helper::approx_equals(a, b);
    }
    case VECTOR2: {
        if (other._type != VECTOR2) {
            return *this == other;
        }
        return std::get<vector2>(_data).approx_equals(std::get<vector2>(other._data));
    }
    case VECTOR3: {
        if (other._type != VECTOR3) {
            return *this == other;
        }
        return std::get<vector3>(_data).approx_equals(std::get<vector3>(other._data));
    }
    case VECTOR4: {
        if (other._type != VECTOR4) {
            return *this == other;
        }
        return std::get<vector4>(_data).approx_equals(std::get<vector4>(other._data));
    }
    case ARRAY: {
        if (other._type != ARRAY) {
            return *this == other;
        }

        if (std::get<std::vector<variant>>(_data).size() != std::get<std::vector<variant>>(other._data).size()) {
            return false;
        }
        for (size_t i = 0; i < std::get<std::vector<variant>>(_data).size(); i++) {
            if (!std::get<std::vector<variant>>(_data)[i].approx_equals(std::get<std::vector<variant>>(other._data)[i])) {
                return false;
            }
        }
        return true;
    }
    default:
        return *this == other;
    }
}

const vector2 &variant::get_vector2() const { return std::get<vector2>(_data); }

const vector3 &variant::get_vector3() const { return std::get<vector3>(_data); }

const vector4 &variant::get_vector4() const { return std::get<vector4>(_data); }

const bytes_t &variant::get_bytes() const { return std::get<bytes_t>(_data); }

const std::vector<variant> &variant::get_array() const { return std::get<std::vector<variant>>(_data); }
} // namespace camellia
