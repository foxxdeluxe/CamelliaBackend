//
// Created by LENOVO on 2025/4/4.
//

#include "live/play/activity.h"

#include <format>

#include "live/play/actor.h"
#include "global.h"
#include "live/exception/uninitialized_exception.h"
#include "live/play/stage.h"

#define CLASS_NAME "activity"

namespace camellia {

    integer_t activity::next_aid = 0;

    stage *activity::get_stage() const {
        return _p_parent_stage;
    }

    void activity::init(const activity_data &data, boolean_t keep_actor, stage &parent) {
        _p_data = &data;
        _p_parent_stage = &parent;

        if (!keep_actor) {
            _aid = next_aid++;
            const auto actor_data = parent.get_actor_data(_p_data->h_actor_id);
            THROW_IF_NULL(actor_data, std::format("Actor data not found.\n"
                                                  "Actor = {}, Activity = {}",
                                                  actor_data->h_id, _aid));

            const auto p_actor = &parent.allocate_actor(_aid, actor_data->h_actor_type);
            p_actor->init(*actor_data);
        } else if (parent.get_actor(_aid) == nullptr) {
            throw std::runtime_error(std::format("Could not keep actor if it doesn't exist.\n"
                                                 "Activity = {}",
                                                 _aid));
        }

        _timeline.init(data.timeline, *_p_parent_stage, this);

        for (auto &attribute: data.initial_attributes) {
            _initial_attributes[attribute.first] = attribute.second;
        }

        _is_valid = true;
    }

    void activity::fina() {
        _is_valid = false;

        auto p_actor = _p_parent_stage->get_actor(_aid);
        if (p_actor != nullptr) {
            p_actor->fina();
            _p_parent_stage->collect_actor(_aid);
        }

        _timeline.fina();
        _initial_attributes.clear();

        _p_parent_stage = nullptr;
        _p_data = nullptr;
    }

    number_t activity::update(number_t beat_time) {
        if (!_is_valid) return 0.0F;
        RETURN_ZERO_IF_NULL(_p_data);

        auto p_actor = _p_parent_stage->get_actor(_aid);
        RETURN_ZERO_IF_NULL(p_actor);

        auto candidates = _timeline.update(beat_time);
        auto temp_attributes = std::map<hash_t, variant>(_initial_attributes);

        for (auto keyframe: candidates) {
            if (keyframe->get_action().get_type() != action_data::action_types::ACTION_MODIFIER) continue;
            auto ma = dynamic_cast<modifier_action *>(&keyframe->get_action());
            auto action_time = beat_time - keyframe->get_time();

            if (action_time <= keyframe->get_preferred_duration()) {
                ma->apply_modifier(action_time, temp_attributes);
            } else if (keyframe->get_linger()) {
                ma->apply_modifier(keyframe->get_preferred_duration(), temp_attributes);
            }
        }

        p_actor->attributes.update(temp_attributes);
        p_actor->attributes.handle_dirty_attributes(*p_actor);

        return _timeline.get_effective_duration() - beat_time;
    }

    variant activity::get_initial_value(hash_t h_attribute_name) {
        if (!_is_valid) return {};

        const auto p_actor = _p_parent_stage->get_actor(_aid);
        THROW_IF_NULL(p_actor, std::format("Actor instance not found.\n"
                                           "Activity = {}",
                                           _aid));

        if (const auto it = _p_data->initial_attributes.find(h_attribute_name);
            it != _p_data->initial_attributes.end()) return it->second;

        if (const auto it = p_actor->get_default_attributes().find(h_attribute_name);
            it != p_actor->get_default_attributes().end()) return it->second;

        // TODO: Report warning
        return {};
    }

    boolean_t activity::handle_dirty_attribute(hash_t key, const variant &val) {
        if (!_is_valid) return false;

        auto p_actor = _p_parent_stage->get_actor(_aid);
        THROW_IF_NULL(p_actor, std::format("Actor instance not found.\n"
                                           "Activity = {}",
                                           _aid));

        return p_actor->handle_dirty_attribute(key, val);
    }
}

#undef CLASS_NAME