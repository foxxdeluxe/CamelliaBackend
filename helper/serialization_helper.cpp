#include "serialization_helper.h"
#include <cstring>

namespace camellia::serialization_helper {

// Write little-endian 32-bit integer
void write_le32(bytes_t &data, uint32_t value) {
    data.push_back(static_cast<unsigned char>(value & 0xFFU));
    data.push_back(static_cast<unsigned char>((value >> 8U) & 0xFFU));
    data.push_back(static_cast<unsigned char>((value >> 16U) & 0xFFU));
    data.push_back(static_cast<unsigned char>((value >> 24U) & 0xFFU));
}

// Write little-endian 64-bit integer
void write_le64(bytes_t &data, uint64_t value) {
    data.push_back(static_cast<unsigned char>(value & 0xFFU));
    data.push_back(static_cast<unsigned char>((value >> 8U) & 0xFFU));
    data.push_back(static_cast<unsigned char>((value >> 16U) & 0xFFU));
    data.push_back(static_cast<unsigned char>((value >> 24U) & 0xFFU));
    data.push_back(static_cast<unsigned char>((value >> 32U) & 0xFFU));
    data.push_back(static_cast<unsigned char>((value >> 40U) & 0xFFU));
    data.push_back(static_cast<unsigned char>((value >> 48U) & 0xFFU));
    data.push_back(static_cast<unsigned char>((value >> 56U) & 0xFFU));
}

// Write little-endian float
void write_le_float(bytes_t &data, float value) {
    uint32_t bits = 0;
    std::memcpy(&bits, &value, sizeof(bits));
    write_le32(data, bits);
}

// Read little-endian 32-bit integer
uint32_t read_le32(const bytes_t &data, size_t &offset) {
    if (offset + 4 > data.size()) {
        return 0;
    }
    uint32_t value = static_cast<uint32_t>(data[offset]) | (static_cast<uint32_t>(data[offset + 1]) << 8U) | (static_cast<uint32_t>(data[offset + 2]) << 16U) |
                     (static_cast<uint32_t>(data[offset + 3]) << 24U);
    offset += 4;
    return value;
}

// Read little-endian 64-bit integer
uint64_t read_le64(const bytes_t &data, size_t &offset) {
    if (offset + 8U > data.size()) {
        return 0;
    }
    uint64_t value = static_cast<uint64_t>(data[offset]) | (static_cast<uint64_t>(data[offset + 1]) << 8U) | (static_cast<uint64_t>(data[offset + 2]) << 16U) |
                     (static_cast<uint64_t>(data[offset + 3]) << 24U) | (static_cast<uint64_t>(data[offset + 4]) << 32U) |
                     (static_cast<uint64_t>(data[offset + 5]) << 40U) | (static_cast<uint64_t>(data[offset + 6]) << 48U) |
                     (static_cast<uint64_t>(data[offset + 7]) << 56U);
    offset += 8;
    return value;
}

// Read little-endian float
float read_le_float(const bytes_t &data, size_t &offset) {
    uint32_t bits = read_le32(data, offset);
    float value = 0.0F;
    std::memcpy(&value, &bits, sizeof(value));
    return value;
}

} // namespace camellia::serialization_helper
