//
// Created by LENOVO on 2025/4/4.
//

#ifndef CAMELLIABACKEND_STAGE_H
#define CAMELLIABACKEND_STAGE_H


#include "data/play/stage_data.h"
#include "dialog.h"
#include "actor.h"
#include "activity.h"
#include "manager.h"

namespace camellia {
    class manager;

    class stage {
    public:
        [[nodiscard]] virtual dialog &get_main_dialog() const = 0;
        virtual actor &allocate_actor(integer_t aid, hash_t h_actor_type) = 0;
        [[nodiscard]] virtual actor *get_actor(integer_t aid) = 0;
        virtual void collect_actor(integer_t aid) = 0;
        void advance();
        virtual number_t update(number_t stage_time);
        virtual ~stage() = default;

#ifndef SWIG
        void init(const stage_data &data, manager &parent);
        void fina();
        [[nodiscard]] const actor_data *get_actor_data(hash_t h_id) const;
        [[nodiscard]] const action_data *get_action_data(hash_t h_id) const;
        const std::string * get_script_code(hash_t h_script_name) const;
        void set_beat(const beat_data *beat);

    private:
        const stage_data * _scenario{};

        const beat_data *_current_beat{nullptr};
        integer_t _next_beat_index{0};
        boolean_t _is_initialized{false};

        number_t _stage_time{0.0F}, _beat_begin_time{0.0F}, _time_to_end{0.0F};

        manager *_p_parent_backend{nullptr};
        dialog *_p_main_dialog{nullptr};

        std::map<integer_t, activity> _activities;
#endif
    };
}

#endif //CAMELLIABACKEND_STAGE_H
