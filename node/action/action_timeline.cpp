#include <cmath>
#include <cstddef>
#include <format>
#include <memory>

#include "action.h"
#include "action_timeline.h"
#include "camellia_constant.h"
#include "camellia_macro.h"
#include "helper/algorithm_helper.h"
#include "helper/resource_helper.h"
#include "node/stage.h"

namespace camellia {
number_t action_timeline_keyframe::get_time() const {
    // Assume _data is valid - this is a precondition
    return _data->time;
}

number_t action_timeline_keyframe::get_preferred_duration() const {
    // Assume _data is valid - this is a precondition
    return std::fabsf(_data->preferred_duration_signed);
}

boolean_t action_timeline_keyframe::get_linger() const {
    // Assume _data is valid - this is a precondition
    return _data->preferred_duration_signed <= 0.0F;
}

number_t action_timeline_keyframe::get_actual_duration() const { return std::min(_effective_duration, get_preferred_duration()); }

number_t action_timeline_keyframe::get_effective_duration() const { return _effective_duration; }

integer_t action_timeline_keyframe::get_track_index() const { return _track_index; }

integer_t action_timeline_keyframe::get_index() const { return _index; }

std::shared_ptr<action_timeline_keyframe_data> action_timeline_keyframe::get_data() const {
    // Assume _data is valid - this is a precondition
    return _data;
}

const std::map<text_t, variant> *action_timeline_keyframe::get_override_params() const {
    // Assume _data is valid - this is a precondition
    return &_data->override_params;
}

action *action_timeline_keyframe::get_action() const {
    return _p_action.get();
}

action_timeline *action_timeline_keyframe::get_parent_timeline() const {
    return static_cast<action_timeline *>(_p_parent);
}

void action_timeline_keyframe::init(const std::shared_ptr<action_timeline_keyframe_data> &data, action_timeline *parent, integer_t ti, integer_t i,
                                    number_t effective_duration) {
    REQUIRES_VALID(*data);

    _data = data;
    _p_parent = parent;
    _effective_duration = effective_duration;
    _track_index = ti;
    _index = i;

    auto *stage_ptr = parent->get_stage();
    const auto action_data = stage_ptr ? stage_ptr->get_action_data(data->h_action_name) : nullptr;
    FAIL_LOG_IF(action_data == nullptr, std::format("Action data ({}) not found.", data->h_action_name));

    auto type = action_data->get_action_type();
    switch (type) {
    case action_data::ACTION_MODIFIER: {
        _p_action = get_manager().new_live_object<modifier_action>();
        break;
    }
    case action_data::ACTION_COMPOSITE: {
        _p_action = get_manager().new_live_object<composite_action>();
        break;
    }
    default: {
        FAIL_LOG_RETURN(std::format("Unknown action type ({}).", type), );
    }
    }
    _p_action->init(action_data, this);

    _state = state::READY;
}

void action_timeline_keyframe::fina() {
    _state = state::UNINITIALIZED;
    _error_message.clear();
    _data = nullptr;
    _p_parent = nullptr;
    _track_index = -1;
    _index = -1;

    if (_p_action != nullptr) {
        _p_action->fina();
        _p_action = nullptr;
    }
}

variant action_timeline_keyframe::query_param(const text_t &key) const {
    auto it = get_override_params()->find(key);
    return it == get_override_params()->end() ? variant() : it->second;
}

stage *action_timeline::get_stage() const {
    return _p_stage;
}

number_t action_timeline::get_effective_duration() const {
    return _effective_duration;
}

void action_timeline::init(const std::vector<std::shared_ptr<action_timeline_data>> &data, stage &stage, node *p_parent) {
    for (size_t i = 0; i < data.size(); i++) {
        REQUIRES_VALID_MSG(*data[i], std::format("Action timeline data #{} is invalid", i));
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
                auto live_keyframe = get_manager().new_live_object<action_timeline_keyframe>();
                auto preferred_duration = std::fabsf(keyframe->preferred_duration_signed);

                auto effective_duration = keyframe->preferred_duration_signed < 0.0F ? NUMBER_POSITIVE_INFINITY : preferred_duration;
                if (i < track->keyframes.size() - 1) {
                    effective_duration = std::min(track->keyframes[i + 1]->time - keyframe->time, effective_duration);
                }

                live_keyframe->init(keyframe, this, track_index, i, effective_duration);
                live_track[i] = std::move(live_keyframe);
            }

            _last_completed_keyframe_indices.push_back(-1);
            track_index++;
        }
        _effective_duration = std::max(_effective_duration, d->effective_duration);
    }

    _state = state::READY;
}

void action_timeline::fina() {
    _state = state::UNINITIALIZED;
    _error_message.clear();
    _data.clear();
    _p_stage = nullptr;
    _p_parent = nullptr;
    _effective_duration = 0.0F;

    for (auto &track : _tracks) {
        for (auto &keyframe : track) {
            keyframe->fina();
        }
    }

    _tracks.clear();
    _last_completed_keyframe_indices.clear();
}

std::vector<const action_timeline_keyframe *> action_timeline::sample(number_t timeline_time) const {
    REQUIRES_READY_RETURN(*this, std::vector<const action_timeline_keyframe *>());

    std::vector<const action_timeline_keyframe *> res;
    for (const auto &track : _tracks) {
        auto index = algorithm_helper::upper_bound<std::unique_ptr<action_timeline_keyframe>>(
            track, [&](const std::unique_ptr<action_timeline_keyframe> &k) { return algorithm_helper::compare_to(k->get_time(), timeline_time); });
        if (index <= 0) {
            continue;
        }
        res.push_back(track[index - 1].get());
    }
    return res;
}

std::map<hash_t, variant> action_timeline::update(const number_t timeline_time, const std::map<hash_t, variant> &attributes,
                                                  std::vector<std::map<hash_t, variant>> &ref_attributes, const boolean_t continuous,
                                                  const boolean_t exclude_ongoing) {
    REQUIRES_READY_RETURN(*this, (std::map<hash_t, variant>()));
    resource_helper::finally fin([this]() { _current_initial_attributes = nullptr; });

    _current_initial_attributes = &attributes;
    std::vector<const action_timeline_keyframe *> ongoing_keyframes;
    std::vector<const action_timeline_keyframe *> finishing_keyframes;

    if (!continuous) {
        _last_completed_keyframe_indices.clear();
        for (auto &track : _tracks) {
            auto index = algorithm_helper::upper_bound<std::unique_ptr<action_timeline_keyframe>>(
                track, [&](const std::unique_ptr<action_timeline_keyframe> &k) { return algorithm_helper::compare_to(k->get_time(), timeline_time); });
            if (index <= 0) {
                _last_completed_keyframe_indices.push_back(-1);
                continue;
            }
            auto *keyframe = track[index - 1].get();
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
                auto *k = track[_last_completed_keyframe_indices[i] + 1].get();
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
        // Skip if keyframe is in failed state
        if (keyframe->has_error()) {
            continue;
        }
        auto *action = keyframe->get_action();
        if (action == nullptr) {
            continue;
        }
        switch (action->get_action_type()) {
        case action_data::action_types::ACTION_COMPOSITE: {
            auto *ca = dynamic_cast<composite_action *>(action);
            auto *timeline = ca->get_timeline();
            if (timeline != nullptr) {
                temp_attributes = timeline->update(keyframe->get_preferred_duration(), temp_attributes, ref_attributes, continuous, true);
            }
            break;
        }
        default: {
            break;
        }
        }
    }

    for (const auto *keyframe : ongoing_keyframes) {
        // Skip if keyframe is in failed state
        if (keyframe->has_error()) {
            continue;
        }
        auto *action = keyframe->get_action();
        if (action == nullptr) {
            continue;
        }
        auto action_time = std::min(timeline_time - keyframe->get_time(), keyframe->get_preferred_duration());
        switch (action->get_action_type()) {
        case action_data::action_types::ACTION_MODIFIER: {
            auto *ma = dynamic_cast<modifier_action *>(action);
            ma->apply_modifier(action_time, temp_attributes, ref_attributes);
            break;
        }
        case action_data::action_types::ACTION_COMPOSITE: {
            auto *ca = dynamic_cast<composite_action *>(action);
            auto *timeline = ca->get_timeline();
            if (timeline != nullptr) {
                temp_attributes = timeline->update(action_time, temp_attributes, ref_attributes, continuous, false);
            }
            break;
        }
        default: {
            break;
        }
        }
    }

    return temp_attributes;
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
    return std::format("{} > ActionTimelineKeyframe(T{}, #{}, @{})", _p_parent != nullptr ? _p_parent->get_locator() : "???", _track_index, _index,
                       _data->time);
}
} // namespace camellia
