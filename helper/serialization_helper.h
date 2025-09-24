#ifndef SERIALIZATION_HELPER_H
#define SERIALIZATION_HELPER_H

#include "camellia_typedef.h"
#include <cstdint>

namespace camellia::serialization_helper {

// Write little-endian 32-bit integer
void write_le32(bytes_t &data, uint32_t value);

// Write little-endian 64-bit integer
void write_le64(bytes_t &data, uint64_t value);

// Write little-endian float
void write_le_float(bytes_t &data, float value);

// Read little-endian 32-bit integer
uint32_t read_le32(const bytes_t &data, size_t &offset);

// Read little-endian 64-bit integer
uint64_t read_le64(const bytes_t &data, size_t &offset);

// Read little-endian float
float read_le_float(const bytes_t &data, size_t &offset);

} // namespace camellia::serialization_helper

#endif // SERIALIZATION_HELPER_H