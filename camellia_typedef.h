#ifndef CAMELLIA_TYPEDEF_H
#define CAMELLIA_TYPEDEF_H

#include <cstdint>
#include <string>
#include <vector>

namespace camellia {
using number_t = float;
using integer_t = std::int32_t;
using boolean_t = bool;
using text_t = std::string;
using bytes_t = std::vector<std::uint8_t>;
using hash_t = std::uint64_t;

enum log_level : char { LOG_DEBUG, LOG_INFO, LOG_WARN, LOG_ERROR, LOG_FATAL };
} // namespace camellia

#endif // CAMELLIA_TYPEDEF_H