#include "variant.h"
#include "helper/algorithm_helper.h"
#include <cstdio>
#include <cstring>
#include <sstream>
#include <string>
#include <variant>
#include "helper/serialization_helper.h"

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

variant::operator hash_t() const { return std::get<hash_t>(_data); }

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

variant variant::from_desc(const text_t &descriptor) {
    if (descriptor.empty()) {
        return {}; // VOID
    }

    char type_char = descriptor[0];

    switch (type_char) {
    case VOID_PREFIX: // VOID
        return {};

    case INTEGER_PREFIX: { // INTEGER
        if (descriptor.size() <= 1) {
            return {0};
        }

        // Find base suffix (D=decimal, H=hex, O=octal, B=binary)
        char base_suffix = descriptor.back();

        int base = 10;
        size_t len{descriptor.size() - 2};
        switch (base_suffix) {
        case 'H':
            base = 16;
            break;
        case 'O':
            base = 8;
            break;
        case 'B':
            base = 2;
            break;
        case 'D':
            break;
        default:
            // No suffix, assume entire content is decimal
            len = descriptor.size() - 1;
            break;
        }

        try {
            return {static_cast<integer_t>(std::stol(descriptor.substr(1, len), nullptr, base))};
        } catch (...) {
            return {0};
        }
    }

    case NUMBER_PREFIX: { // NUMBER (floating point)
        try {
            return {std::stof(descriptor.substr(1))};
        } catch (...) {
            return {0.0F};
        }
    }

    case BOOLEAN_PREFIX: { // BOOLEAN
        if (descriptor.size() <= 1 || descriptor[1] == '0' || descriptor[1] == 'F') {
            return {false};
        } else {
            return {true};
        }
    }

    case TEXT_PREFIX: // TEXT
        return {descriptor.substr(1)};

    case ERROR_PREFIX: // ERROR
        return {descriptor.substr(1), true};

    case VECTOR2_PREFIX: { // VECTOR2
        // Format: "2x,y"
        std::istringstream iss(descriptor);
        iss.seekg(1);
        vector2 v{0.0, 0.0};
        char comma{'Z'};
        iss >> v.dim[0] >> comma >> v.dim[1];
        return {v};
    }

    case VECTOR3_PREFIX: { // VECTOR3
        // Format: "3x,y,z"
        std::istringstream iss(descriptor);
        iss.seekg(1);
        vector3 v{0.0, 0.0, 0.0};
        char comma{'Z'};
        iss >> v.dim[0] >> comma >> v.dim[1] >> comma >> v.dim[2];
        return {v};
    }

    case VECTOR4_PREFIX: { // VECTOR4
        // Format: "4x,y,z,w"
        std::istringstream iss(descriptor);
        iss.seekg(1);
        vector4 v{0.0, 0.0, 0.0, 0.0};
        char comma{'Z'};
        iss >> v.dim[0] >> comma >> v.dim[1] >> comma >> v.dim[2] >> comma >> v.dim[3];
        return {v};
    }

    case BYTES_PREFIX: { // BYTES
        // Hex-encoded bytes
        bytes_t bytes;
        for (size_t i = 1; i < descriptor.size(); i += 2) {
            if (i + 1 < descriptor.size()) {
                text_t hex_byte = descriptor.substr(i, 2);
                try {
                    auto byte = static_cast<unsigned char>(std::stoul(hex_byte, nullptr, 16));
                    bytes.push_back(byte);
                } catch (...) {
                    // Skip invalid hex
                    bytes.push_back(0);
                }
            }
        }
        return {bytes};
    }

    case ARRAY_PREFIX: { // ARRAY - special case where '[' is the type indicator
        // Parse array: [elem1,elem2,elem3]
        if (descriptor.back() != ARRAY_SUFFIX) {
            return {std::vector<variant>()};
        }

        std::vector<variant> elements;
        if (descriptor.size() <= 2) {
            return {elements};
        }

        // Parse comma-separated elements with escape handling
        size_t start = 1;
        int bracket_level = 0;
        bool escaped = false;

        for (size_t i = start; i <= descriptor.length() - 1; ++i) {
            char c = (i < descriptor.length() - 1) ? descriptor[i] : ARRAY_SEPARATOR; // Treat end as comma

            if (escaped) {
                escaped = false;
                continue;
            }

            if (c == ESCAPE_CHAR) {
                escaped = true;
                continue;
            }

            if (c == ARRAY_PREFIX) {
                bracket_level++;
            } else if (c == ARRAY_SUFFIX) {
                bracket_level--;
            } else if (c == ARRAY_SEPARATOR && bracket_level == 0) {
                // Found a separator at top level
                text_t element_desc = descriptor.substr(start, i - start);

                text_t unescaped;
                if (element_desc.size() >= 1 && element_desc[0] == TEXT_PREFIX) {
                    // Unescape the element descriptor
                    bool esc = false;
                    for (char ch : element_desc) {
                        if (esc) {
                            unescaped += ch;
                            esc = false;
                        } else if (ch == ESCAPE_CHAR) {
                            esc = true;
                        } else {
                            unescaped += ch;
                        }
                    }
                } else {
                    unescaped = element_desc;
                }

                elements.push_back(from_desc(unescaped)); // TODO: optimize this
                start = i + 1;
            }
        }

        return {elements};
    }

    case ATTRIBUTE_PREFIX: { // ATTRIBUTE (hash)
        try {
            if (descriptor.size() <= 1) {
                return {hash_t(0)};
            }

            if (descriptor[1] == ':') {
                hash_t hash = algorithm_helper::calc_hash(descriptor.substr(2));
                return {hash};
            }

            hash_t hash = std::stoull(descriptor.substr(1), nullptr, 16);
            return {hash};
        } catch (...) {
            return {hash_t(0)};
        }
    }

    default:
        return {}; // Unknown type, return VOID
    }
}

text_t variant::to_desc() const {
    switch (_type) {
    case VOID:
        return text_t{VOID_PREFIX};

    case INTEGER: {
        integer_t value = std::get<integer_t>(_data);
        return std::format("{}{}{}", INTEGER_PREFIX, value, INTEGER_DECIMAL_SUFFIX);
    }

    case NUMBER: {
        number_t value = std::get<number_t>(_data);
        return std::format("{}{}", NUMBER_PREFIX, value);
    }

    case BOOLEAN: {
        boolean_t value = std::get<boolean_t>(_data);
        return std::format("{}{}", BOOLEAN_PREFIX, value ? "1" : "0");
    }

    case TEXT: {
        const auto &value = std::get<text_t>(_data);
        return std::format("{}{}", TEXT_PREFIX, value);
    }

    case ERROR: {
        const auto &value = std::get<text_t>(_data);
        return std::format("{}{}", ERROR_PREFIX, value);
    }

    case VECTOR2: {
        const auto &v = std::get<vector2>(_data);
        return std::format("{}{}{}{}", VECTOR2_PREFIX, v.get_x(), VECTOR_SEPARATOR, v.get_y());
    }

    case VECTOR3: {
        const auto &v = std::get<vector3>(_data);
        return std::format("{}{}{}{}{}{}", VECTOR3_PREFIX, v.get_x(), VECTOR_SEPARATOR, v.get_y(), VECTOR_SEPARATOR, v.get_z());
    }

    case VECTOR4: {
        const auto &v = std::get<vector4>(_data);
        return std::format("{}{}{}{}{}{}{}{}", VECTOR4_PREFIX, v.get_x(), VECTOR_SEPARATOR, v.get_y(), VECTOR_SEPARATOR, v.get_z(), VECTOR_SEPARATOR,
                           v.get_w());
    }

    case BYTES: {
        const auto &bytes = std::get<bytes_t>(_data);
        text_t result;
        result.reserve((bytes.size() * 2) + 1);
        result += BYTES_PREFIX;
        for (unsigned char byte : bytes) {
            result += std::format("{:02X}", byte);
        }
        return result;
    }

    case ARRAY: {
        const auto &elements = std::get<std::vector<variant>>(_data);
        text_t result{ARRAY_PREFIX};

        for (size_t i = 0; i < elements.size(); ++i) {
            if (i > 0) {
                result += ARRAY_SEPARATOR;
            }

            auto element_desc = elements[i].to_desc();

            if (element_desc.size() >= 1 && element_desc[0] == TEXT_PREFIX) {
                // Escape special characters in the element string descriptor
                auto escaped = text_t();
                escaped.reserve(element_desc.size());
                for (char c : element_desc) {
                    if (c == ESCAPE_CHAR || c == ARRAY_SEPARATOR || c == ARRAY_PREFIX || c == ARRAY_SUFFIX) {
                        escaped += ESCAPE_CHAR;
                    }
                    escaped += c;
                }
                result += escaped;
            } else {
                result += element_desc;
            }
        }

        result += ARRAY_SUFFIX;
        return result;
    }

    case ATTRIBUTE: {
        auto hash = std::get<hash_t>(_data);
        return std::format("{}{:016X}", ATTRIBUTE_PREFIX, hash);
    }

    default:
        return text_t{VOID_PREFIX}; // Fallback to VOID
    }
}

bytes_t variant::to_binary() const {
    using namespace serialization_helper;

    bytes_t result;

    // Write type as first byte
    result.push_back(static_cast<unsigned char>(_type));

    switch (_type) {
    case VOID:
        // No additional data needed
        break;

    case INTEGER: {
        int32_t value = std::get<integer_t>(_data);
        write_le32(result, static_cast<uint32_t>(value));
        break;
    }

    case NUMBER: {
        float value = std::get<number_t>(_data);
        write_le_float(result, value);
        break;
    }

    case BOOLEAN: {
        bool value = std::get<boolean_t>(_data);
        result.push_back(value ? 1 : 0);
        break;
    }

    case TEXT:
    case ERROR: {
        const auto &text = std::get<text_t>(_data);
        write_le32(result, static_cast<uint32_t>(text.size()));
        result.insert(result.end(), text.begin(), text.end());
        break;
    }

    case VECTOR2: {
        const auto &v = std::get<vector2>(_data);
        write_le_float(result, v.get_x());
        write_le_float(result, v.get_y());
        break;
    }

    case VECTOR3: {
        const auto &v = std::get<vector3>(_data);
        write_le_float(result, v.get_x());
        write_le_float(result, v.get_y());
        write_le_float(result, v.get_z());
        break;
    }

    case VECTOR4: {
        const auto &v = std::get<vector4>(_data);
        write_le_float(result, v.get_x());
        write_le_float(result, v.get_y());
        write_le_float(result, v.get_z());
        write_le_float(result, v.get_w());
        break;
    }

    case BYTES: {
        const auto &bytes = std::get<bytes_t>(_data);
        write_le32(result, static_cast<uint32_t>(bytes.size()));
        result.insert(result.end(), bytes.begin(), bytes.end());
        break;
    }

    case ARRAY: {
        const auto &elements = std::get<std::vector<variant>>(_data);
        write_le32(result, static_cast<uint32_t>(elements.size()));
        for (const auto &element : elements) {
            auto element_binary = element.to_binary();
            result.insert(result.end(), element_binary.begin(), element_binary.end());
        }
        break;
    }

    case ATTRIBUTE: {
        uint64_t hash = std::get<hash_t>(_data);
        write_le64(result, hash);
        break;
    }

    default:
        // Unknown type, just write the type byte
        break;
    }

    return result;
}

variant variant::from_binary(const bytes_t &binary_data) {
    using namespace serialization_helper;

    if (binary_data.empty()) {
        return {}; // VOID
    }

    size_t offset = 0;
    auto type = static_cast<types>(binary_data[offset++]);

    switch (type) {
    case VOID:
        return {};

    case INTEGER: {
        uint32_t bits = read_le32(binary_data, offset);
        auto value = static_cast<int32_t>(bits);
        return {static_cast<integer_t>(value)};
    }

    case NUMBER: {
        float value = read_le_float(binary_data, offset);
        return {static_cast<number_t>(value)};
    }

    case BOOLEAN: {
        if (offset >= binary_data.size()) {
            return {false};
        }
        bool value = binary_data[offset] != 0;
        return {value};
    }

    case TEXT: {
        uint32_t length = read_le32(binary_data, offset);
        if (offset + length > binary_data.size()) {
            return {text_t()};
        }
        text_t text(binary_data.begin() + static_cast<ptrdiff_t>(offset), binary_data.begin() + static_cast<ptrdiff_t>(offset + length));
        return {text};
    }

    case ERROR: {
        uint32_t length = read_le32(binary_data, offset);
        if (offset + length > binary_data.size()) {
            return {text_t(), true};
        }
        text_t text(binary_data.begin() + static_cast<ptrdiff_t>(offset), binary_data.begin() + static_cast<ptrdiff_t>(offset + length));
        return {text, true};
    }

    case VECTOR2: {
        float x = read_le_float(binary_data, offset);
        float y = read_le_float(binary_data, offset);
        return {vector2(x, y)};
    }

    case VECTOR3: {
        float x = read_le_float(binary_data, offset);
        float y = read_le_float(binary_data, offset);
        float z = read_le_float(binary_data, offset);
        return {vector3(x, y, z)};
    }

    case VECTOR4: {
        float x = read_le_float(binary_data, offset);
        float y = read_le_float(binary_data, offset);
        float z = read_le_float(binary_data, offset);
        float w = read_le_float(binary_data, offset);
        return {vector4(x, y, z, w)};
    }

    case BYTES: {
        uint32_t length = read_le32(binary_data, offset);
        if (offset + length > binary_data.size()) {
            return {bytes_t()};
        }
        bytes_t bytes(binary_data.begin() + static_cast<ptrdiff_t>(offset), binary_data.begin() + static_cast<ptrdiff_t>(offset + length));
        return {bytes};
    }

    case ARRAY: {
        uint32_t count = read_le32(binary_data, offset);
        std::vector<variant> elements;
        elements.reserve(count);

        for (uint32_t i = 0; i < count && offset < binary_data.size(); ++i) {
            // Extract the current element by finding where it ends
            size_t element_start = offset;
            if (element_start >= binary_data.size()) {
                break;
            }

            auto element_type = static_cast<types>(binary_data[element_start]);
            offset = element_start + 1;

            // Calculate element size based on type
            size_t element_size = 1; // Type byte
            switch (element_type) {
            case VOID:
                break;
            case INTEGER:
            case NUMBER:
                element_size += 4U;
                break;
            case BOOLEAN:
                element_size += 1U;
                break;
            case TEXT:
            case ERROR:
            case BYTES: {
                if (offset + 4 > binary_data.size()) {
                    return {elements};
                }
                uint32_t length = read_le32(binary_data, offset);
                element_size += 4U + length;
                offset = element_start + element_size;
                break;
            }
            case VECTOR2:
                element_size += 8U;
                break;
            case VECTOR3:
                element_size += 12U;
                break;
            case VECTOR4:
                element_size += 16U;
                break;
            case ATTRIBUTE:
                element_size += 8U;
                break;
            case ARRAY: {
                // Recursive case - need to parse the sub-array to find its end
                offset = element_start + 1;
                if (offset + 4 > binary_data.size()) {
                    return {elements};
                }
                uint32_t sub_count = read_le32(binary_data, offset);
                // For simplicity, use recursive call to parse sub-array
                bytes_t sub_array(binary_data.begin() + static_cast<ptrdiff_t>(element_start), binary_data.end());
                auto sub_variant = from_binary(sub_array);
                elements.push_back(sub_variant);
                // Skip the parsed sub-array - this is complex, so we'll handle it differently
                continue;
            }
            default:
                return {elements};
            }

            if (element_type != ARRAY) {
                offset = element_start + element_size;
                if (offset > binary_data.size()) {
                    break;
                }

                bytes_t element_data(binary_data.begin() + static_cast<ptrdiff_t>(element_start), binary_data.begin() + static_cast<ptrdiff_t>(offset));
                elements.push_back(from_binary(element_data));
            }
        }

        return {elements};
    }

    case ATTRIBUTE: {
        uint64_t hash = read_le64(binary_data, offset);
        return {static_cast<hash_t>(hash)};
    }

    default:
        return {}; // Unknown type, return VOID
    }
}

} // namespace camellia
