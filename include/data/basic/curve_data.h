//
// Created by LENOVO on 2025/4/4.
//

#ifndef CAMELLIABACKEND_CURVE_DATA_H
#define CAMELLIABACKEND_CURVE_DATA_H

#include <vector>
#include "variant.h"

namespace camellia {
    struct curve_point_data {
        vector2 position{0.0F, 0.0F};
        number_t left_tangent{0.0F};
        number_t right_tangent{0.0F};
    };

    struct curve_data {
        std::vector<curve_point_data> points{};
    };
}
#endif //CAMELLIABACKEND_CURVE_DATA_H
