//
// Created by LENOVO on 2025/4/4.
//

#include "camellia.h"
#include <format>
#include <set>

namespace camellia {
action *action::allocate_action(const std::shared_ptr<action_data> p_data) {
    switch (p_data->get_action_type()) {
    case action_data::ACTION_MODIFIER: {
        return new modifier_action();
    }
    default: {
        throw std::runtime_error(std::format("Unknown action type.\n"
                                             "Type = {}",
                                             p_data->get_action_type()));
    }
    }
}

void action::collect_action(const action *action) { delete action; }

integer_t action::get_track_index() const { return track_index; }

integer_t action::get_index() const { return index; }

void action::init(const std::shared_ptr<action_data> data, action_timeline_keyframe *parent, const integer_t ti, const integer_t i) {

    track_index = ti;
    index = i;
    p_base_data = data;
}

void action::fina() {
    track_index = -1;
    index = -1;
    p_base_data = nullptr;
}

action_data::action_types modifier_action::get_type() const { return action_data::action_types::ACTION_MODIFIER; }

const std::shared_ptr<modifier_action_data> modifier_action::get_data() const { return std::dynamic_pointer_cast<modifier_action_data>(p_base_data); }

hash_t modifier_action::get_name_hash() const { return get_data()->h_action_name; }

number_t modifier_action::get_actual_duration() const { return _p_parent_keyframe->get_actual_duration(); }

number_t modifier_action::get_preferred_duration() const { return _p_parent_keyframe->get_preferred_duration(); }

hash_t modifier_action::get_attribute_name_hash() const { return get_data()->h_attribute_name; }

variant::types modifier_action::get_value_type() const { return get_data()->value_type; }

const std::map<text_t, variant> &modifier_action::get_default_params() const { return get_data()->default_params; }

void modifier_action::init(const std::shared_ptr<action_data> data, action_timeline_keyframe *p_parent, const integer_t ti, const integer_t i) {

    continuous_action::init(data, p_parent, ti, i);

    // guaranteed not to be nullptr by allocate_action
    auto mad = std::dynamic_pointer_cast<modifier_action_data>(data);

    _p_parent_keyframe = p_parent;
    _p_timeline = &_p_parent_keyframe->get_timeline();

    _p_script = new scripting_helper::engine();

    std::set<text_t> seen;
    for (auto &p : *p_parent->get_override_params()) {
        if (seen.contains(p.first))
            continue;
        _p_script->set_property(p.first, p.second);
        seen.insert(p.first);
    }

    for (auto &p : get_default_params()) {
        if (seen.contains(p.first))
            continue;
        _p_script->set_property(p.first, p.second);
        seen.insert(p.first);
    }

    auto code = p_parent->get_timeline().get_stage().get_script_code(mad->h_script_name);
    THROW_IF_NULL(code, std::format("Failed to find modifier action script.\n"
                                    "Script = {}, Action = {}",
                                    mad->h_script_name, mad->h_action_name));

    try {
        _p_script->guarded_evaluate(*code, variant::VOID);
    } catch (const scripting_helper::engine::scripting_engine_error &err) {
        throw std::runtime_error(std::format("Error while evaluating modifier action script.\n"
                                             "Script = {}, Action = {}\n"
                                             "{}",
                                             mad->h_script_name, mad->h_action_name, err.what()));
    }

    _is_valid = true;
}

void modifier_action::fina() {
    _is_valid = false;

    _p_timeline = nullptr;
    final_value = variant();

    if (_p_script != nullptr) {
        delete _p_script;
        _p_script = nullptr;
    }

    continuous_action::fina();
}

variant modifier_action::apply_modifier(const number_t action_time, const hash_t h_attribute_name, const variant &val) const {
    if (get_attribute_name_hash() != h_attribute_name)
        return val;
    return modify(action_time, val);
}

void modifier_action::apply_modifier(const number_t action_time, std::map<hash_t, variant> &attributes) const {
    const auto it = attributes.find(get_attribute_name_hash());
    if (it == attributes.end()) {
        // TODO: Report warning
        return;
    }

    attributes[get_attribute_name_hash()] = modify(action_time, it->second);
}

const char *modifier_action::TIME_NAME = "time";
const char *modifier_action::DURATION_NAME = "duration";
const char *modifier_action::PREV_NAME = "prev";
const char *modifier_action::ORIG_NAME = "orig";
const char *modifier_action::RUN_NAME = "run";

variant modifier_action::modify(const number_t action_time, const variant &base_value) const {
    if (!_is_valid)
        return base_value;

    try {
        // set built-in constants
        _p_script->set_property(TIME_NAME, action_time);
        _p_script->set_property(DURATION_NAME, std::min(get_actual_duration(), get_preferred_duration()));
        _p_script->set_property(PREV_NAME, _p_timeline->get_prev_value(*this));
        _p_script->set_property(ORIG_NAME, base_value);

        return _p_script->guarded_invoke(RUN_NAME, 0, nullptr, get_value_type());

    } catch (scripting_helper::engine::scripting_engine_error &err) {
        const auto data = get_data();
        throw std::runtime_error(std::format("Error while invoking function 'run()' of the modifier action script.\n"
                                             "Script = {}, Action = {}\n"
                                             "{}",
                                             data->h_script_name, data->h_action_name, err.what()));
    }
}
} // namespace camellia