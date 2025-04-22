//
// Created by LENOVO on 2025/4/4.
//

#ifndef CAMELLIABACKEND_ACTION_H
#define CAMELLIABACKEND_ACTION_H


#include "data/action/action_data.h"
#include "data/action/continuous/modifier_action_data.h"
#include "helper/scripting_helper.h"

namespace camellia {
    class action_timeline_keyframe;

    class action_timeline;

    class action {
    public:
        static action *allocate_action(const action_data *p_data);

        static void collect_action(const action *action);

        [[nodiscard]] integer_t get_track_index() const;

        [[nodiscard]] integer_t get_index() const;

        virtual void init(const action_data *data,
                          action_timeline_keyframe *parent,
                          integer_t ti,
                          integer_t i);

        virtual void fina();

        [[nodiscard]] virtual action_data::action_types get_type() const = 0;

        virtual ~action() = default;

    protected:
        const action_data *p_base_data{};

    private:
        integer_t track_index{-1}, index{-1};
    };


    class continuous_action : public action {
    public:

    };

    class instant_action : public action {
    public:
    };

    class modifier_action : public continuous_action {
    public:
        [[nodiscard]] action_data::action_types get_type() const override;

        [[nodiscard]] const modifier_action_data *get_data() const;

        [[nodiscard]] hash_t get_name_hash() const;

        [[nodiscard]] number_t get_actual_duration() const;

        [[nodiscard]] number_t get_preferred_duration() const;

        [[nodiscard]] hash_t get_attribute_name_hash() const;

        [[nodiscard]] variant::types get_value_type() const;

        [[nodiscard]] const std::map<text_t, variant> & get_default_params() const;

        void init(const action_data *data, action_timeline_keyframe *p_parent, integer_t ti, integer_t i) override;

        void fina() override;

        variant apply_modifier(number_t action_time, hash_t h_attribute_name, const variant &val) const;

        void apply_modifier(number_t action_time, std::map<hash_t, variant> &attributes) const;

        variant final_value;

    private:
#ifndef SWIG
        const text_t RUN_NAME = "run";
        const text_t TIME_NAME = "time";
        const text_t DURATION_NAME = "duration";
        const text_t PREV_NAME = "prev";
        const text_t ORIG_NAME = "orig";

        boolean_t _is_valid{false};

        action_timeline_keyframe * _p_parent_keyframe{nullptr};
        action_timeline *_p_timeline{nullptr};
        scripting_helper::engine *_p_script{nullptr};

        variant modify(number_t action_time, const variant &base_value) const;

#endif
    };
}

#endif //CAMELLIABACKEND_ACTION_H
