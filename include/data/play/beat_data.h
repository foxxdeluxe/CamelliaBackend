//
// Created by LENOVO on 2025/4/4.
//

#ifndef CAMELLIABACKEND_BEAT_DATA_H
#define CAMELLIABACKEND_BEAT_DATA_H

#include <map>
#include "dialog_data.h"
#include "variant.h"
#include "activity_data.h"

namespace camellia {
    struct beat_data {
        dialog_data dialog;

        // <actor_instance_id, data>
        std::map<integer_t, activity_data> activities;
    };
}
#endif //CAMELLIABACKEND_BEAT_DATA_H
