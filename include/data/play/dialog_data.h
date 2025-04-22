//
// Created by LENOVO on 2025/4/4.
//

#ifndef CAMELLIABACKEND_DIALOG_DATA_H
#define CAMELLIABACKEND_DIALOG_DATA_H

#include <optional>
#include "variant.h"
#include "data/action/action_timeline_data.h"

namespace camellia {
    struct text_region_attachment {
        struct attachment_layout {
            enum modes {
                SEPARATE_LINES, ENVELOPE_LINES
            };

            modes mode{SEPARATE_LINES};
            vector2 offset = {0.0F, 0.0F}, anchor_pos = {0.0F, 0.0F}, pivot_pos = {0.0F, 0.0F};
            number_t rotation{0.0F};
        };

        attachment_layout layout{};
    };

    struct text_region_attachment_text : public text_region_attachment {
        text_t text{};
    };

    struct text_region_data {
        integer_t id{};
        text_t text{};
        std::vector<text_region_attachment> attachments{};
        action_timeline_data timeline{};

        number_t transition_duration{0.0F};
        hash_t h_transition_script_name{};
    };

    struct dialog_data {
        hash_t h_character_id{};
        std::vector<text_region_data> regions{};
        action_timeline_data region_life_timeline{.effective_duration = -1.0F};
    };
}
#endif //CAMELLIABACKEND_DIALOG_DATA_H
