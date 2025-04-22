//
// Created by LENOVO on 2025/4/4.
//

#ifndef CAMELLIABACKEND_ACTOR_DATA_H
#define CAMELLIABACKEND_ACTOR_DATA_H

#include <map>
#include "variant.h"

namespace camellia {
    struct actor_data {
        integer_t h_actor_type;
        text_t name, nickname;

        // actors can share names, so a unique id is needed
        hash_t h_id;

        std::map<hash_t, variant> default_attributes;
    };
}
#endif //CAMELLIABACKEND_ACTOR_DATA_H
