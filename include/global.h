//
// Created by LENOVO on 2025/4/4.
//

#ifndef CAMELLIABACKEND_GLOBAL_H
#define CAMELLIABACKEND_GLOBAL_H

namespace camellia {
    typedef float number_t;
    typedef int integer_t;
    typedef bool boolean_t;
    typedef std::string text_t;
    typedef std::vector<unsigned char> bytes_t;
    typedef std::vector<unsigned char> bytes_t;
    typedef unsigned long long hash_t;
}

#define RETURN_IF_NULL(P, X) if ( P == nullptr ) [[unlikely]] return X
#define RETURN_NULL_IF_NULL(P) RETURN_IF_NULL(P, nullptr)
#define RETURN_ZERO_IF_NULL(P) RETURN_IF_NULL(P, 0)
#define RETURN_FALSE_IF_NULL(P) RETURN_IF_NULL(P, false)

#define THROW_IF_NULL(P, M) if ( P == nullptr ) [[unlikely]] throw std::runtime_error( M )

#endif //CAMELLIABACKEND_GLOBAL_H
