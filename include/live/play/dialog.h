//
// Created by LENOVO on 2025/4/4.
//

#ifndef CAMELLIABACKEND_DIALOG_H
#define CAMELLIABACKEND_DIALOG_H


#include "timeline_evaluator.h"
#include "variant.h"
#include "live/action/action_timeline.h"
#include "data/play/dialog_data.h"
#include "helper/scripting_helper.h"
#include "attribute_registry.h"
#include "helper/algorithm_helper.h"

namespace camellia {
    class stage;

    class dialog;

    class text_region : public timeline_evaluator {
    public:
        [[nodiscard]] text_t get_current_text() const;
        [[nodiscard]] text_t get_full_text() const;
        [[nodiscard]] boolean_t get_is_visible() const;
        [[nodiscard]] integer_t get_id() const;
        [[nodiscard]] number_t get_transition_duration() const;
        [[nodiscard]] number_t get_transition_speed() const;
        number_t update(number_t region_time) override;
        variant get_initial_value(hash_t h_attribute_name) override;
        virtual boolean_t handle_visibility_update(boolean_t is_visible) = 0;
        virtual ~text_region() = default;

#ifndef SWIG
        [[nodiscard]] dialog *get_parent_dialog() const;
        virtual void init(const text_region_data &data, dialog &parent);
        virtual void fina();

    private:
        const hash_t H_TEXT_NAME = algorithm_helper::calc_hash("text");

        const text_t FULL_TEXT_LENGTH_NAME = "full_text_length";
        const text_t TRANSITION_SPEED_NAME = "transition_speed";
        const text_t ORIG_NAME = "orig";
        const text_t TIME_NAME = "time";
        const text_t RUN_NAME = "run";

        action_timeline _timeline;
        const text_region_data *_data;
        dialog *_parent_dialog;
        scripting_helper::engine *_p_transition_script;

        std::map<hash_t, variant> _initial_attributes;
        attribute_registry _attributes;

        boolean_t _is_visible, _last_is_visible;
#endif
    };

    class dialog {
    public:
        virtual text_region &append_text_region() = 0;
        [[nodiscard]] virtual text_region *get_text_region(size_t index) = 0;
        [[nodiscard]] virtual size_t get_text_region_count() = 0;
        virtual void trim_text_regions(size_t from_index) = 0;
        number_t update(number_t beat_time);
        virtual ~dialog() = default;

#ifndef SWIG
        [[nodiscard]] stage &get_stage() const;
        void init(stage &st);
        void fina();
        void advance(const dialog_data &data);

    private:
        const dialog_data *_current{};
        stage *_parent_stage{};

        action_timeline _region_life_timeline{};
        boolean_t _use_life_timeline{false};
        boolean_t _hide_inactive_regions{true};
#endif
    };
}

#endif //CAMELLIABACKEND_DIALOG_H
