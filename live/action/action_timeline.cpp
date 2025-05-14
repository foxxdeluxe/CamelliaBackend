//
// Created by LENOVO on 2025/4/4.
//

#include <cmath>
#include <format>

#include "camellia.h"

#define CLASS_NAME "action_timeline_keyframe"

namespace camellia {
number_t action_timeline_keyframe::get_time() const {
    RETURN_ZERO_IF_NULL(_data);
    return _data->time;
}

number_t action_timeline_keyframe::get_preferred_duration() const {
    RETURN_ZERO_IF_NULL(_data);
    return std::fabsf(_data->preferred_duration_signed);
}

boolean_t action_timeline_keyframe::get_linger() const {
    RETURN_FALSE_IF_NULL(_data);
    return _data->preferred_duration_signed <= 0.0F;
}

number_t action_timeline_keyframe::get_actual_duration() const {
    RETURN_ZERO_IF_NULL(_data);
    return _actual_duration;
}

const std::shared_ptr<action_timeline_keyframe_data> action_timeline_keyframe::get_data() const { return _data; }

const std::map<text_t, variant> *action_timeline_keyframe::get_override_params() const {
    RETURN_NULL_IF_NULL(_data);
    return &_data->override_params;
}

action &action_timeline_keyframe::get_action() const { return *_p_action; }

action_timeline &action_timeline_keyframe::get_timeline() const { return *_parent_timeline; }

void action_timeline_keyframe::init(const std::shared_ptr<action_timeline_keyframe_data> data, action_timeline *parent, integer_t ti, integer_t i,
                                    number_t actual_duration) {

    _data = data;
    _parent_timeline = parent;
    _actual_duration = actual_duration;

    const auto action_data = parent->get_stage().get_action_data(data->h_action_name);
    THROW_IF_NULL(action_data, std::format("Action data not found.\n"
                                           "Action = {}",
                                           data->h_action_name));

    _p_action = action::allocate_action(action_data);
    _p_action->init(action_data, this, ti, i);
}

void action_timeline_keyframe::fina() {
    _data = nullptr;
    _parent_timeline = nullptr;

    if (_p_action != nullptr) {
        _p_action->fina();
        action::collect_action(_p_action);
        _p_action = nullptr;
    }
}

variant action_timeline_keyframe::query_param(const text_t &key) const {
    THROW_UNINITIALIZED_IF_NULL(_data);
    auto it = get_override_params()->find(key);
    return it == get_override_params()->end() ? variant() : it->second;
}

#undef CLASS_NAME
#define CLASS_NAME "action_timeline"

stage &action_timeline::get_stage() const {
    THROW_UNINITIALIZED_IF_NULL(_p_stage);
    return *_p_stage;
}

timeline_evaluator *action_timeline::get_timeline_evaluator() const { return _p_timeline_evaluator; }

number_t action_timeline::get_effective_duration() const { return _effective_duration; }

void action_timeline::init(std::vector<std::shared_ptr<action_timeline_data>> data, stage &stage, timeline_evaluator *p_parent) {
    _data = data;
    _p_stage = &stage;
    _p_timeline_evaluator = p_parent;

    for (const auto &d : data) {
        for (int t = 0; t < d->tracks.size(); t++) {
            auto track = d->tracks[t];
            std::vector<action_timeline_keyframe> live_track;
            for (int i = 0; i < track->keyframes.size(); i++) {
                auto keyframe = track->keyframes[i];
                live_track.emplace_back();
                auto &live_keyframe = live_track.back();
                auto preferred_duration = std::fabsf(keyframe->preferred_duration_signed);
                live_keyframe.init(
                    keyframe, this, t, i,
                    (i >= track->keyframes.size() - 1 ? preferred_duration : std::min(track->keyframes[i + 1]->time - keyframe->time, preferred_duration)));
            }

            _tracks.push_back(std::move(live_track));
            _next_keyframe_indices.push_back(0);
        }
        _effective_duration = std::max(_effective_duration, d->effective_duration);
    }
}

void action_timeline::fina() {
    _data.clear();
    _p_stage = nullptr;
    _p_timeline_evaluator = nullptr;
    _effective_duration = 0.0F;

    for (auto &track : _tracks) {
        for (auto &keyframe : track) {
            keyframe.fina();
        }
    }

    _tracks.clear();
    _next_keyframe_indices.clear();
}

std::vector<const action_timeline_keyframe *> action_timeline::sample(number_t timeline_time) const {
    std::vector<const action_timeline_keyframe *> res;
    for (auto &track : _tracks) {
        auto index = algorithm_helper::upper_bound<action_timeline_keyframe>(
            track, [&](const action_timeline_keyframe &k) { return algorithm_helper::compare_to(k.get_time(), timeline_time); });
        if (index <= 0)
            continue;
        res.push_back(&track[index - 1]);
    }
    return res;
}

std::vector<const action_timeline_keyframe *> action_timeline::update(const number_t timeline_time, const boolean_t continuous) {
    std::vector<const action_timeline_keyframe *> res;

    if (!continuous) {
        _next_keyframe_indices.clear();
        for (auto &track : _tracks) {
            auto index = algorithm_helper::upper_bound<action_timeline_keyframe>(
                track, [&](const action_timeline_keyframe &k) { return algorithm_helper::compare_to(k.get_time(), timeline_time); });
            _next_keyframe_indices.push_back(index);
            if (index <= 0)
                continue;
            res.push_back(&track[index - 1]);
        }
    } else {
        for (int i = 0; i < _tracks.size(); i++) {
            auto &track = _tracks[i];

            action_timeline_keyframe *k;
            while (_next_keyframe_indices[i] < track.size() && (k = &track[_next_keyframe_indices[i]])->get_time() <= timeline_time) {
                if (k->get_action().get_type() < 0)
                    res.push_back(k); // take instant actions into account
                _next_keyframe_indices[i]++;
            }

            if (_next_keyframe_indices[i] > 0) {
                auto &keyframe = track[_next_keyframe_indices[i] - 1];
                if (keyframe.get_action().get_type() > 0)
                    res.push_back(&keyframe); // continuous actions only
            }
        }
    }

    return res;
}

variant action_timeline::get_base_value(const number_t timeline_time, const hash_t h_attribute_name, const modifier_action &until) const {
    auto val = _p_timeline_evaluator->get_initial_value(h_attribute_name);

    for (const auto &keyframe : sample(timeline_time)) {
        if (keyframe->get_action().get_type() != action_data::action_types::ACTION_MODIFIER)
            continue;
        const auto mo = dynamic_cast<modifier_action *>(&keyframe->get_action());
        if (mo == &until)
            return val;

        const auto action_time = timeline_time - keyframe->get_time();
        val = mo->apply_modifier(action_time, h_attribute_name, val);
    }

    throw std::runtime_error(std::format("Target modifier action not found at specified time.\n"
                                         "Time = {}, Action = {}",
                                         timeline_time, until.get_name_hash()));
}

variant action_timeline::get_prev_value(const modifier_action &ac) const {
    auto &track = _tracks[ac.get_track_index()];
    auto &current_keyframe = track[ac.get_index()];
    auto base_value = get_base_value(current_keyframe.get_time(), ac.get_attribute_name_hash(), ac);
    if (ac.get_index() <= 0)
        return base_value;

    auto &last_keyframe = track[ac.get_index() - 1];
    auto &lac = last_keyframe.get_action();
    if (lac.get_type() != action_data::action_types::ACTION_MODIFIER)
        return base_value;
    const auto lma = dynamic_cast<modifier_action *>(&lac);
    if (lma->get_attribute_name_hash() != ac.get_attribute_name_hash())
        return base_value;

    if (lma->final_value.get_value_type() == variant::VOID)
        lma->final_value = lma->apply_modifier(current_keyframe.get_time() - last_keyframe.get_time(), ac.get_attribute_name_hash(), base_value);
    return lma->final_value;
}
} // namespace camellia
#undef CLASS_NAME