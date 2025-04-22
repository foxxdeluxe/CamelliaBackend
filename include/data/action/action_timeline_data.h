//
// Created by LENOVO on 2025/4/4.
//

#ifndef CAMELLIABACKEND_ACTION_TIMELINE_DATA_H
#define CAMELLIABACKEND_ACTION_TIMELINE_DATA_H

#include <map>
#include <vector>
#include "variant.h"

namespace camellia {
    struct action_timeline_keyframe_data {
        number_t time{0.0F};
        number_t preferred_duration_signed{0.0F};

        hash_t h_action_name{0ULL};
        std::map<text_t , variant> override_params{};
    };

    struct action_timeline_track_data {
        std::vector<action_timeline_keyframe_data> keyframes{};
    };

    struct action_timeline_data {
        std::vector<action_timeline_track_data> tracks{};
        number_t effective_duration{0.0F};
    };
}
#endif //CAMELLIABACKEND_ACTION_TIMELINE_DATA_H
