//
// Created by LENOVO on 2025/4/7.
//

#include <format>
#include "live/exception/uninitialized_exception.h"

namespace camellia {
    uninitialized_exception::uninitialized_exception(const text_t &class_name) {
        _msg = class_name + "is uninitialized.";
    }

    const char *uninitialized_exception::what() const noexcept {
        return _msg.c_str();
    }
}