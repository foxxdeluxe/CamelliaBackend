//
// Created by LENOVO on 2025/4/4.
//

#include <cmath>
#include <cstddef>
#include <format>

#include "camellia.h"
#include "helper/resource_helper.h"

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

    const auto action_data = parent->get_stage().get_action_data(data->h_action_name);
    THROW_IF(action_data == nullptr, std::format("Action data ({}) not found.\n"
                                                 "{}",
                                                 data->h_action_name, get_locator()));

    _p_action = &action::allocate_action(action_data->get_action_type());
    _p_action->init(action_data, this, ti, i);
}

void action_timeline_keyframe::fina() {
    _data = nullptr;
    _parent_timeline = nullptr;

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

    for (const auto &d : data) {
        for (int t = 0; t < d->tracks.size(); t++) {
            auto track = d->tracks[t];
            std::vector<action_timeline_keyframe> live_track;
            for (int i = 0; i < track->keyframes.size(); i++) {
                auto keyframe = track->keyframes[i];
                live_track.emplace_back();
                auto &live_keyframe = live_track.back();
                auto preferred_duration = std::fabsf(keyframe->preferred_duration_signed);

                auto effective_duration = keyframe->preferred_duration_signed < 0.0F ? NUMBER_POSITIVE_INFINITY : preferred_duration;
                if (i < track->keyframes.size() - 1) {
                    effective_duration = std::min(track->keyframes[i + 1]->time - keyframe->time, effective_duration);
                }

                live_keyframe.init(keyframe, this, t, i, effective_duration);
            }

            _tracks.push_back(std::move(live_track));
            _next_keyframe_indices.push_back(0);
        }
        _effective_duration = std::max(_effective_duration, d->effective_duration);
    }

    _current_composite_keyframes.resize(_tracks.size());
}

void action_timeline::fina() {
    _data.clear();
    _p_stage = nullptr;
    _p_parent = nullptr;
    _effective_duration = 0.0F;

    for (auto &track : _tracks) {
        for (auto &keyframe : track) {
            keyframe.fina();
        }
    }

    _tracks.clear();
    _next_keyframe_indices.clear();
    _current_composite_keyframes.clear();
}

std::vector<const action_timeline_keyframe *> action_timeline::sample(number_t timeline_time) const {
    std::vector<const action_timeline_keyframe *> res;
    for (const auto &track : _tracks) {
        auto index = algorithm_helper::upper_bound<action_timeline_keyframe>(
            track, [&](const action_timeline_keyframe &k) { return algorithm_helper::compare_to(k.get_time(), timeline_time); });
        if (index <= 0) {
            continue;
        }
        res.push_back(&track[index - 1]);
    }
    return res;
}

std::map<hash_t, variant> action_timeline::update(const number_t timeline_time, const std::map<hash_t, variant> &attributes, const boolean_t continuous,
                                                  const boolean_t exclude_continuous) {
    resource_helper::finally fin([this]() { _current_initial_attributes = nullptr; });

    _current_initial_attributes = &attributes;
    std::vector<const action_timeline_keyframe *> candidates;

    if (!continuous) {
        _next_keyframe_indices.clear();
        _current_composite_keyframes.clear();
        for (int i = 0; i < _tracks.size(); i++) {
            auto &track = _tracks[i];
            auto index = algorithm_helper::upper_bound<action_timeline_keyframe>(
                track, [&](const action_timeline_keyframe &k) { return algorithm_helper::compare_to(k.get_time(), timeline_time); });
            _next_keyframe_indices.push_back(index);
            if (index <= 0) {
                continue;
            }
            auto &keyframe = track[index - 1];
            candidates.push_back(&keyframe);
            if (keyframe.get_action().get_type() == action_data::action_types::ACTION_COMPOSITE) {
                _current_composite_keyframes[i] = &keyframe;
            }
        }
    } else {
        for (int i = 0; i < _tracks.size(); i++) {
            auto &track = _tracks[i];

            action_timeline_keyframe *k{nullptr};
            while (_next_keyframe_indices[i] < track.size() && timeline_time <= (k = &track[_next_keyframe_indices[i]])->get_time()) {
                auto *prev_composite_keyframe = _current_composite_keyframes[i];
                if (prev_composite_keyframe != nullptr) {
                    candidates.push_back(prev_composite_keyframe);
                    _current_composite_keyframes[i] = nullptr;
                }

                auto action_type = k->get_action().get_type();
                if (action_type < action_data::action_types::ACTION_COMPOSITE) {
                    candidates.push_back(k); // take instant keyframes into account
                } else if (action_type == action_data::action_types::ACTION_COMPOSITE && timeline_time > k->get_time() + k->get_effective_duration()) {
                    candidates.push_back(k); // take ended composite keyframes into account
                }
                _next_keyframe_indices[i]++;
            }

            if (_next_keyframe_indices[i] > 0) {
                auto &keyframe = track[_next_keyframe_indices[i] - 1];
                if (keyframe.get_linger() || keyframe.get_time() + keyframe.get_preferred_duration() <= timeline_time) {
                    auto type = keyframe.get_action().get_type();
                    if (type > action_data::action_types::ACTION_COMPOSITE) {
                        if (!exclude_continuous) {
                            candidates.push_back(&keyframe); // continuous keyframes only
                        }
                    } else if (type == action_data::action_types::ACTION_COMPOSITE) {
                        // _current_composite_keyframes[i] is either nullptr or &keyframe here
                        _current_composite_keyframes[i] = &keyframe;
                        candidates.push_back(&keyframe); // ongoing composite keyframes only
                    }
                } else {
                    // when current timeline time is not inside any keyframe, and not before all keyframes
                    auto *prev_composite_keyframe = _current_composite_keyframes[i];
                    if (prev_composite_keyframe != nullptr) {
                        // only when current timeline time is after a composite keyframe and before the next keyframe
                        candidates.push_back(prev_composite_keyframe);
                        _current_composite_keyframes[i] = nullptr;
                    }
                }
            }
        }
    }

    std::map<hash_t, variant> temp_attributes{attributes};

    for (const auto *keyframe : candidates) {
        auto *action = &keyframe->get_action();
        switch (action->get_type()) {
        case action_data::action_types::ACTION_MODIFIER: {
            auto *ma = dynamic_cast<modifier_action *>(action);
            auto action_time = timeline_time - keyframe->get_time();
            ma->apply_modifier(std::min(action_time, keyframe->get_preferred_duration()), temp_attributes);
            break;
        }
        case action_data::action_types::ACTION_COMPOSITE: {
            auto *ca = dynamic_cast<composite_action *>(action);
            if (timeline_time > keyframe->get_time() + keyframe->get_effective_duration()) {
                // already ended,
                temp_attributes = ca->get_timeline().update(timeline_time - keyframe->get_time(), temp_attributes, continuous, true);
            } else {
                // ongoing
                temp_attributes = ca->get_timeline().update(timeline_time - keyframe->get_time(), temp_attributes, continuous, false);
            }
            break;
        }
        default: {
            THROW(std::format("Unknown type ({}) of action ({}).", action->get_type(), action->get_locator()));
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
    const auto &track = _tracks[ac.get_track_index()];
    const auto &current_keyframe = track[ac.get_index()];
    auto base_value = get_base_value(current_keyframe.get_time(), ac.get_attribute_name_hash(), ac);
    if (ac.get_index() <= 0) {
        return base_value;
    }

    const auto &last_keyframe = track[ac.get_index() - 1];
    auto &lac = last_keyframe.get_action();
    if (lac.get_type() != action_data::action_types::ACTION_MODIFIER) {
        return base_value;
    }
    auto *const lma = dynamic_cast<modifier_action *>(&lac);
    if (lma->get_attribute_name_hash() != ac.get_attribute_name_hash()) {
        return base_value;
    }

    if (lma->final_value.get_value_type() == variant::VOID) {
        lma->final_value = lma->apply_modifier(current_keyframe.get_time() - last_keyframe.get_time(), ac.get_attribute_name_hash(), base_value);
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
    return std::format("{} > ActionTimelineKeyframe(@{})", _parent_timeline != nullptr ? _parent_timeline->get_locator() : "???", _data->time);
}
} // namespace camellia
