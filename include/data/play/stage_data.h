//
// Created by LENOVO on 2025/4/4.
//

#ifndef CAMELLIABACKEND_STAGE_DATA_H
#define CAMELLIABACKEND_STAGE_DATA_H

#include <vector>
#include <map>
#include <memory>
#include "beat_data.h"
#include "actor_data.h"
#include "data/action/action_data.h"
#include "rc_obj.h"

namespace camellia {
    struct stage_data {
        std::vector<beat_data> beats{};
        std::map<hash_t, text_t> scripts{};
        std::map<hash_t, actor_data> actors{};

        // pointers in data structs must point to new-ed objects
        std::map<hash_t, std::shared_ptr<action_data>> actions{};

        ~stage_data() = default;
    };
}
#endif //CAMELLIABACKEND_STAGE_DATA_H
