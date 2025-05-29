//
// Created by LENOVO on 2025/4/4.
//

#include <cmath>
#include <cstddef>
#include <format>

#include "action.h"
#include "action_timeline.h"
#include "camellia_constant.h"
#include "camellia_macro.h"
#include "helper/algorithm_helper.h"
#include "helper/resource_helper.h"
#include "live/play/stage.h"

namespace camellia {
number_t action_timeline_keyframe::get_time() const {
    REQUIRES_NOT_NULL(_data);
    return _data->time;
}

number_t action_timeline_keyframe::get_preferred_duration() const {
    REQUIRES_NOT_NULL(_data);
    return std::fabsf(_data->preferred_duration_signed);
}

boolean_t action_timeline_keyframe::get_linger() const {
    REQUIRES_NOT_NULL(_data);
    return _data->preferred_duration_signed <= 0.0F;
}

number_t action_timeline_keyframe::get_actual_duration() const { return std::min(_effective_duration, get_preferred_duration()); }

number_t action_timeline_keyframe::get_effective_duration() const { return _effective_duration; }

integer_t action_timeline_keyframe::get_track_index() const { return _track_index; }

integer_t action_timeline_keyframe::get_index() const { return _index; }

std::shared_ptr<action_timeline_keyframe_data> action_timeline_keyframe::get_data() const {
    REQUIRES_NOT_NULL(_data);
    return _data;
}

const std::map<text_t, variant> *action_timeline_keyframe::get_override_params() const {
    REQUIRES_NOT_NULL(_data);
    return &_data->override_params;
}

action &action_timeline_keyframe::get_action() const {
    REQUIRES_NOT_NULL(_p_action);
    return *_p_action;
}

action_timeline &action_timeline_keyframe::get_timeline() const {
    REQUIRES_NOT_NULL(_parent_timeline);
    return *_parent_timeline;
}

void action_timeline_keyframe::init(const std::shared_ptr<action_timeline_keyframe_data> &data, action_timeline *parent, integer_t ti, integer_t i,
                                    number_t effective_duration) {
    REQUIRES_VALID(*data);

    _data = data;
    _parent_timeline = parent;
    _effective_duration = effective_duration;
    _track_index = ti;
    _index = i;

    const auto action_data = parent->get_stage().get_action_data(data->h_action_name);
    THROW_IF(action_data == nullptr, std::format("Action data ({}) not found.", data->h_action_name));

    _p_action = &action::allocate_action(action_data->get_action_type());
    _p_action->init(action_data, this);

    _is_initialized = true;
}

void action_timeline_keyframe::fina() {
    _is_initialized = false;
    _data = nullptr;
    _parent_timeline = nullptr;
    _track_index = -1;
    _index = -1;

    if (_p_action != nullptr) {
        _p_action->fina();
        action::collect_action(*_p_action);
        _p_action = nullptr;
    }
}

variant action_timeline_keyframe::query_param(const text_t &key) const {
    auto it = get_override_params()->find(key);
    return it == get_override_params()->end() ? variant() : it->second;
}

action_timeline_keyframe::action_timeline_keyframe(const action_timeline_keyframe & /*other*/) { THROW_NO_LOC("Copying not allowed"); }

action_timeline_keyframe &action_timeline_keyframe::operator=(const action_timeline_keyframe &other) {
    if (this == &other) {
        return *this;
    }

    THROW_NO_LOC("Copying not allowed");
}

stage &action_timeline::get_stage() const {
    REQUIRES_NOT_NULL(_p_stage);
    return *_p_stage;
}

live_object *action_timeline::get_parent() const {
    REQUIRES_NOT_NULL(_p_parent);
    return _p_parent;
}

number_t action_timeline::get_effective_duration() const { return _effective_duration; }

void action_timeline::init(const std::vector<std::shared_ptr<action_timeline_data>> &data, stage &stage, live_object *p_parent) {
    for (size_t i = 0; i < data.size(); i++) {
        REQUIRES_VALID_MSG(*data[i], std::format("data #{} is invalid", i));
    }

    _data = data;
    _p_stage = &stage;
    _p_parent = p_parent;

    int track_index = 0;
    for (const auto &d : data) {
        _tracks.resize(_tracks.size() + d->tracks.size());
        for (const auto &track : d->tracks) {
            auto &live_track = _tracks[track_index];
            live_track.resize(track->keyframes.size());
            for (int i = 0; i < track->keyframes.size(); i++) {
                auto keyframe = track->keyframes[i];
                auto *live_keyframe = new action_timeline_keyframe();
                live_track[i] = live_keyframe;
                auto preferred_duration = std::fabsf(keyframe->preferred_duration_signed);

                auto effective_duration = keyframe->preferred_duration_signed < 0.0F ? NUMBER_POSITIVE_INFINITY : preferred_duration;
                if (i < track->keyframes.size() - 1) {
                    effective_duration = std::min(track->keyframes[i + 1]->time - keyframe->time, effective_duration);
                }

                live_keyframe->init(keyframe, this, track_index, i, effective_duration);
            }

            _last_completed_keyframe_indices.push_back(-1);
            track_index++;
        }
        _effective_duration = std::max(_effective_duration, d->effective_duration);
    }

    _current_composite_keyframes.resize(_tracks.size());

    _is_initialized = true;
}

void action_timeline::fina() {
    _is_initialized = false;
    _data.clear();
    _p_stage = nullptr;
    _p_parent = nullptr;
    _effective_duration = 0.0F;

    for (auto &track : _tracks) {
        for (auto &keyframe : track) {
            keyframe->fina();
            delete keyframe;
        }
    }

    _tracks.clear();
    _last_completed_keyframe_indices.clear();
    _current_composite_keyframes.clear();
}

std::vector<const action_timeline_keyframe *> action_timeline::sample(number_t timeline_time) const {
    REQUIRES_INITIALIZED(*this);

    std::vector<const action_timeline_keyframe *> res;
    for (const auto &track : _tracks) {
        auto index = algorithm_helper::upper_bound<action_timeline_keyframe *>(
            track, [&](const action_timeline_keyframe *k) { return algorithm_helper::compare_to(k->get_time(), timeline_time); });
        if (index <= 0) {
            continue;
        }
        res.push_back(track[index - 1]);
    }
    return res;
}

std::map<hash_t, variant> action_timeline::update(const number_t timeline_time, const std::map<hash_t, variant> &attributes, const boolean_t continuous,
                                                  const boolean_t exclude_ongoing) {
    REQUIRES_INITIALIZED(*this);
    resource_helper::finally fin([this]() { _current_initial_attributes = nullptr; });

    _current_initial_attributes = &attributes;
    std::vector<const action_timeline_keyframe *> ongoing_keyframes;
    std::vector<const action_timeline_keyframe *> finishing_keyframes;

    if (!continuous) {
        _last_completed_keyframe_indices.clear();
        for (auto &track : _tracks) {
            auto index = algorithm_helper::upper_bound<action_timeline_keyframe *>(
                track, [&](const action_timeline_keyframe *k) { return algorithm_helper::compare_to(k->get_time(), timeline_time); });
            if (index <= 0) {
                _last_completed_keyframe_indices.push_back(-1);
                continue;
            }
            auto &keyframe = track[index - 1];
            if (timeline_time <= keyframe->get_time() + keyframe->get_effective_duration()) {
                if (!exclude_ongoing) {
                    ongoing_keyframes.push_back(keyframe);
                }
                _last_completed_keyframe_indices.push_back(index - 1);
            } else {
                _last_completed_keyframe_indices.push_back(index);
            }
        }
    } else {
        for (int i = 0; i < _tracks.size(); i++) {
            auto &track = _tracks[i];

            while (_last_completed_keyframe_indices[i] < static_cast<integer_t>(track.size()) - 1) {
                auto &k = track[_last_completed_keyframe_indices[i] + 1];
                if (timeline_time <= k->get_time() + k->get_effective_duration()) {
                    if (!exclude_ongoing) {
                        ongoing_keyframes.push_back(k);
                    }
                    break;
                }

                finishing_keyframes.push_back(k);
                _last_completed_keyframe_indices[i]++;
            }
        }
    }

    std::map<hash_t, variant> temp_attributes{attributes};

    for (const auto *keyframe : finishing_keyframes) {
        auto *action = &keyframe->get_action();
        switch (action->get_type()) {
        case action_data::action_types::ACTION_COMPOSITE: {
            auto *ca = dynamic_cast<composite_action *>(action);
            temp_attributes = ca->get_timeline().update(keyframe->get_preferred_duration(), temp_attributes, continuous, true);
            break;
        }
        default: {
            break;
        }
        }
    }

    for (const auto *keyframe : ongoing_keyframes) {
        auto *action = &keyframe->get_action();
        auto action_time = std::min(timeline_time - keyframe->get_time(), keyframe->get_preferred_duration());
        switch (action->get_type()) {
        case action_data::action_types::ACTION_MODIFIER: {
            auto *ma = dynamic_cast<modifier_action *>(action);
            ma->apply_modifier(action_time, temp_attributes);
            break;
        }
        case action_data::action_types::ACTION_COMPOSITE: {
            auto *ca = dynamic_cast<composite_action *>(action);
            temp_attributes = ca->get_timeline().update(action_time, temp_attributes, continuous, false);
            break;
        }
        default: {
            break;
        }
        }
    }

    return temp_attributes;
}

variant action_timeline::get_base_value(const number_t timeline_time, const hash_t h_attribute_name, const modifier_action &until) const {
    REQUIRES_NOT_NULL_MSG(_current_initial_attributes, "Initial attributes not set. Is this called during update?");
    auto it = _current_initial_attributes->find(h_attribute_name);
    if (it == _current_initial_attributes->end()) {
        THROW(std::format("Initial attribute ({}) not found.", h_attribute_name));
    }
    auto val = it->second;

    for (const auto &keyframe : sample(timeline_time)) {
        if (keyframe->get_action().get_type() != action_data::action_types::ACTION_MODIFIER) {
            continue;
        }
        auto *const mo = dynamic_cast<modifier_action *>(&keyframe->get_action());
        if (mo == &until) {
            return val;
        }

        const auto action_time = timeline_time - keyframe->get_time();
        val = mo->apply_modifier(action_time, h_attribute_name, val);
    }

    THROW(std::format("Target modifier action ({}) not found at specified time ({}).", until.get_name_hash(), timeline_time));
}

variant action_timeline::get_prev_value(const modifier_action &ac) const {
    const auto &track = _tracks[ac.get_parent_keyframe().get_track_index()];
    const auto &current_keyframe = track[ac.get_parent_keyframe().get_index()];
    auto base_value = get_base_value(current_keyframe->get_time(), ac.get_attribute_name_hash(), ac);
    if (ac.get_parent_keyframe().get_index() <= 0) {
        return base_value;
    }

    const auto &last_keyframe = track[ac.get_parent_keyframe().get_index() - 1];
    auto &lac = last_keyframe->get_action();
    if (lac.get_type() != action_data::action_types::ACTION_MODIFIER) {
        return base_value;
    }
    auto *const lma = dynamic_cast<modifier_action *>(&lac);
    if (lma->get_attribute_name_hash() != ac.get_attribute_name_hash()) {
        return base_value;
    }

    if (lma->final_value.get_value_type() == variant::VOID) {
        lma->final_value = lma->apply_modifier(current_keyframe->get_time() - last_keyframe->get_time(), ac.get_attribute_name_hash(), base_value);
    }
    return lma->final_value;
}

action_timeline::action_timeline(const action_timeline & /*other*/) { THROW_NO_LOC("Copying not allowed"); }

action_timeline &action_timeline::operator=(const action_timeline &other) {
    if (this == &other) {
        return *this;
    }

    THROW_NO_LOC("Copying not allowed");
}

std::string action_timeline::get_locator() const noexcept {
    std::string parent_locator{"???"};
    if (_p_parent != nullptr) {
        parent_locator = _p_parent->get_locator();
    } else if (_p_stage != nullptr) {
        parent_locator = _p_stage->get_locator();
    }
    return std::format("{} > ActionTimeline", parent_locator);
}

std::string action_timeline_keyframe::get_locator() const noexcept {
    return std::format("{} > ActionTimelineKeyframe(T{}, #{}, @{})", _parent_timeline != nullptr ? _parent_timeline->get_locator() : "???", _track_index,
                       _index, _data->time);
}
} // namespace camellia
