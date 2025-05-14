#include <string>
#include <vector>

// Swig doesn't get on well with stdint.h, so we use old-school types instead
namespace camellia {
typedef float number_t;
typedef int integer_t;
typedef bool boolean_t;
typedef std::string text_t;
typedef std::vector<unsigned char> bytes_t;
typedef unsigned long long hash_t;
} // namespace camellia