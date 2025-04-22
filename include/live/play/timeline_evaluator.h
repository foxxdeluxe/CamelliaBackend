//
// Created by LENOVO on 2025/4/4.
//

#ifndef CAMELLIABACKEND_TIMELINE_EVALUATOR_H
#define CAMELLIABACKEND_TIMELINE_EVALUATOR_H


#include "variant.h"
#include "attribute_registry.h"

namespace camellia {
    class timeline_evaluator : public dirty_attribute_handler {
    public:
        virtual number_t update(number_t timeline_time) = 0;
        virtual variant get_initial_value(hash_t h_attribute_name) = 0;
    };
}


#endif //CAMELLIABACKEND_TIMELINE_EVALUATOR_H
