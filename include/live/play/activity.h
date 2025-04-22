//
// Created by LENOVO on 2025/4/4.
//

#ifndef CAMELLIABACKEND_ACTIVITY_H
#define CAMELLIABACKEND_ACTIVITY_H


#include "timeline_evaluator.h"
#include "data/play/activity_data.h"
#include "live/action/action_timeline.h"
#include "actor.h"

namespace camellia {
    class stage;

    class activity : public timeline_evaluator {
    public:
        [[nodiscard]] stage *get_stage() const;
        void init(const activity_data &data, boolean_t keep_actor, stage &parent);
        void fina();
        number_t update(number_t beat_time) override;
        variant get_initial_value(hash_t h_attribute_name) override;
        boolean_t handle_dirty_attribute(hash_t key, const variant &val) override;

    private:
        const activity_data *_p_data{};

        stage *_p_parent_stage{};
        boolean_t _is_valid{false};

        action_timeline _timeline;
        std::map<hash_t, variant> _initial_attributes;

        static integer_t next_aid;
        integer_t _aid{-1};
    };
}

#endif //CAMELLIABACKEND_ACTIVITY_H
