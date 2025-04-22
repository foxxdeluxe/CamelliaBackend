//
// Created by LENOVO on 2025/4/4.
//

#ifndef CAMELLIABACKEND_MODIFIER_ACTION_DATA_H
#define CAMELLIABACKEND_MODIFIER_ACTION_DATA_H

#include "data/action/continuous/continuous_action_data.h"

namespace camellia {
    struct modifier_action_data : public continuous_action_data {
        hash_t h_attribute_name{0ULL};
        variant::types value_type{variant::VOID};
        hash_t h_script_name{0ULL};

        [[nodiscard]] action_types get_action_type() const override
        {
            return action_data::ACTION_MODIFIER;
        }

        ~modifier_action_data() override = default;
    };
}
#endif //CAMELLIABACKEND_MODIFIER_ACTION_DATA_H
