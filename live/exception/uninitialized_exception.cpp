//
// Created by LENOVO on 2025/4/7.
//

#include "camellia.h"

namespace camellia {
uninitialized_exception::uninitialized_exception(const std::string_view &class_name) : _msg(text_t(class_name) + " is uninitialized.") {}

const char *uninitialized_exception::what() const noexcept { return _msg.c_str(); }
} // namespace camellia