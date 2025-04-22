//
// Created by LENOVO on 2025/4/16.
//

#include "rc_obj.h"

namespace camellia {
    void rc_obj::ref() const {
        _ref_count++;
    }

    void rc_obj::unref() const {
        _ref_count--;
        if (_ref_count <= 0) delete this;
    }

    rc_obj::~rc_obj() = default;
}