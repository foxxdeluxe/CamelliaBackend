//
// Created by LENOVO on 2025/4/7.
//

#ifndef CAMELLIABACKEND_UNINITIALIZED_EXCEPTION_H
#define CAMELLIABACKEND_UNINITIALIZED_EXCEPTION_H


#include <exception>
#include "variant.h"

#define THROW_UNINITIALIZED_IF_NULL(P) if ( P == nullptr ) [[unlikely]] throw uninitialized_exception( CLASS_NAME )

namespace camellia {
    class uninitialized_exception : public std::exception {
    public:
        [[nodiscard]] const char *what() const noexcept override;

        explicit uninitialized_exception(const text_t &class_name);

    private:
        text_t _msg;
    };
}

#endif //CAMELLIABACKEND_UNINITIALIZED_EXCEPTION_H
