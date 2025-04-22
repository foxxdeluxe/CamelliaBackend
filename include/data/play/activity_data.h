//
// Created by LENOVO on 2025/4/4.
//

#ifndef CAMELLIABACKEND_ACTIVITY_DATA_H
#define CAMELLIABACKEND_ACTIVITY_DATA_H

#include "variant.h"
#include "data/action/action_timeline_data.h"

namespace camellia {
    struct activity_data {
        hash_t h_actor_id;
        action_timeline_data timeline;
        std::map<hash_t, variant> initial_attributes;
    };
}
#endif //CAMELLIABACKEND_ACTIVITY_DATA_H
