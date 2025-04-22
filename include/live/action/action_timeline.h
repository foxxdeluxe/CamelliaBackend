//
// Created by LENOVO on 2025/4/4.
//

#ifndef CAMELLIABACKEND_ACTION_TIMELINE_H
#define CAMELLIABACKEND_ACTION_TIMELINE_H

#include "action.h"
#include "data/action/action_timeline_data.h"

namespace camellia {

    class stage;

    class timeline_evaluator;

    class action_timeline_keyframe {
    public:
        [[nodiscard]] number_t get_time() const;

        [[nodiscard]] number_t get_preferred_duration() const;

        [[nodiscard]] boolean_t get_linger() const;

        [[nodiscard]] number_t get_actual_duration() const;

        [[nodiscard]] const action_timeline_keyframe_data *get_data() const;

        [[nodiscard]] const std::map<text_t, variant> * get_override_params() const;

        [[nodiscard]] action &get_action() const;

        [[nodiscard]] action_timeline &get_timeline() const;

        void init(const action_timeline_keyframe_data *data,
                  action_timeline *parent,
                  integer_t ti,
                  integer_t i,
                  number_t actual_duration);

        void fina();

        [[nodiscard]] variant query_param(const text_t& key) const;

    private:
        const action_timeline_keyframe_data *_data{};
        action_timeline *_parent_timeline{};
        number_t _actual_duration{};
        action *_p_action{};
    };

    class action_timeline {
    public:
        [[nodiscard]] stage &get_stage() const;

        [[nodiscard]] timeline_evaluator *get_timeline_evaluator() const;

        [[nodiscard]] number_t get_effective_duration() const;

        void init(const action_timeline_data &data, stage &stage, timeline_evaluator *p_parent);

        void fina();

        [[nodiscard]] std::vector<const action_timeline_keyframe *> sample(number_t timeline_time) const;

        std::vector<const action_timeline_keyframe *> update(number_t timeline_time, boolean_t continuous = true);

        variant get_base_value(number_t timeline_time, hash_t h_attribute_name, const modifier_action &until) const;

        variant get_prev_value(const modifier_action &ac) const;

    private:
        const action_timeline_data *_p_data;
        std::vector<integer_t> _next_keyframe_indices;
        std::vector<std::vector<action_timeline_keyframe>> _tracks;

        stage *_p_stage;
        timeline_evaluator *_p_timeline_evaluator;
    };
}

#endif //CAMELLIABACKEND_ACTION_TIMELINE_H
