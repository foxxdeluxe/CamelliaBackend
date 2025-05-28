//
// Created by LENOVO on 2025/4/4.
//

#include "camellia.h"
#include <format>
#include <set>

namespace camellia {
action &action::allocate_action(const action_data::action_types type) {
    switch (type) {
    case action_data::ACTION_MODIFIER: {
        return *new modifier_action();
    }
    case action_data::ACTION_COMPOSITE: {
        return *new composite_action();
    }
    default: {
        THROW_NO_LOC(std::format("Unknown action type ({}).", type));
    }
    }
}

void action::collect_action(const action &action) { delete &action; }

action_timeline_keyframe &action::get_parent_keyframe() const {
    REQUIRES_NOT_NULL(_p_parent_keyframe);
    return *_p_parent_keyframe;
}

void action::init(const std::shared_ptr<action_data> &data, action_timeline_keyframe *parent) {
    REQUIRES_VALID(*data);

    _p_base_data = data;
    _p_parent_keyframe = parent;
}

void action::fina() {
    _p_base_data = nullptr;
    _p_parent_keyframe = nullptr;
}

action::action(const action & /*other*/) { THROW_NO_LOC("Copying not allowed"); }

action &action::operator=(const action &other) {
    if (this == &other) {
        return *this;
    }

    THROW_NO_LOC("Copying not allowed");
}

void continuous_action::init(const std::shared_ptr<action_data> &data, action_timeline_keyframe *p_parent) {
    const auto cad = std::dynamic_pointer_cast<continuous_action_data>(data);
    REQUIRES_NOT_NULL_MSG(cad, std::format("Failed to cast action data ({}) to continuous action data.", data->h_action_name));
    REQUIRES_VALID(*cad);

    action::init(data, p_parent);
}

void instant_action::init(const std::shared_ptr<action_data> &data, action_timeline_keyframe *p_parent) {
    const auto iad = std::dynamic_pointer_cast<instant_action_data>(data);
    REQUIRES_NOT_NULL_MSG(iad, std::format("Failed to cast action data ({}) to instant action data.", data->h_action_name));
    REQUIRES_VALID(*iad);

    action::init(data, p_parent);
}

action_data::action_types modifier_action::get_type() const { return action_data::action_types::ACTION_MODIFIER; }

std::shared_ptr<modifier_action_data> modifier_action::get_data() const {
    REQUIRES_NOT_NULL(_p_base_data);
    return std::dynamic_pointer_cast<modifier_action_data>(_p_base_data);
}

hash_t modifier_action::get_name_hash() const {
    auto p_data = get_data();
    REQUIRES_NOT_NULL(p_data);
    return p_data->h_action_name;
}

number_t modifier_action::get_actual_duration() const {
    REQUIRES_NOT_NULL(_p_parent_keyframe);
    return _p_parent_keyframe->get_actual_duration();
}

number_t modifier_action::get_preferred_duration() const {
    REQUIRES_NOT_NULL(_p_parent_keyframe);
    return _p_parent_keyframe->get_preferred_duration();
}

hash_t modifier_action::get_attribute_name_hash() const {
    auto p_data = get_data();
    REQUIRES_NOT_NULL(p_data);
    return p_data->h_attribute_name;
}

variant::types modifier_action::get_value_type() const {
    auto p_data = get_data();
    REQUIRES_NOT_NULL(p_data);
    return p_data->value_type;
}

const std::map<text_t, variant> &modifier_action::get_default_params() const {
    auto p_data = get_data();
    REQUIRES_NOT_NULL(p_data);
    return p_data->default_params;
}

void modifier_action::init(const std::shared_ptr<action_data> &data, action_timeline_keyframe *p_parent) {
    const auto mad = std::dynamic_pointer_cast<modifier_action_data>(data);
    REQUIRES_NOT_NULL_MSG(mad, std::format("Failed to cast action data ({}) to modifier action data.", data->h_action_name));
    REQUIRES_VALID(*mad);

    continuous_action::init(data, p_parent);

    _p_parent_keyframe = p_parent;
    _p_timeline = &_p_parent_keyframe->get_timeline();

    _p_script = new scripting_helper::engine();

    std::set<text_t> seen;
    for (const auto &p : *p_parent->get_override_params()) {
        if (seen.contains(p.first)) {
            continue;
        }
        _p_script->set_property(p.first, p.second);
        seen.insert(p.first);
    }

    for (const auto &p : get_default_params()) {
        if (seen.contains(p.first)) {
            continue;
        }
        _p_script->set_property(p.first, p.second);
        seen.insert(p.first);
    }

    const auto *code = p_parent->get_timeline().get_stage().get_script_code(mad->h_script_name);
    THROW_IF(code == nullptr, std::format("Failed to find script ({}) for modifier action ({}).\n"
                                          "{}",
                                          mad->h_script_name, mad->h_action_name, get_locator()));

    try {
        _p_script->guarded_evaluate(*code, variant::VOID);
    } catch (const scripting_helper::engine::scripting_engine_error &err) {
        THROW(std::format("Error while evaluating script ({}) for modifier action ({}):\n"
                          "{}",
                          mad->h_script_name, mad->h_action_name, err.what()));
    }

    continuous_action::init(data, p_parent);
}

void modifier_action::fina() {
    _p_timeline = nullptr;
    final_value = variant();

    if (_p_script != nullptr) {
        delete _p_script;
        _p_script = nullptr;
    }

    continuous_action::fina();
}

variant modifier_action::apply_modifier(const number_t action_time, const hash_t h_attribute_name, const variant &val) const {
    if (get_attribute_name_hash() != h_attribute_name) {
        return val;
    }
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
    REQUIRES_NOT_NULL(_p_script);

    try {
        // set built-in constants
        _p_script->set_property(TIME_NAME, action_time);
        _p_script->set_property(DURATION_NAME, std::min(get_actual_duration(), get_preferred_duration()));
        _p_script->set_property(PREV_NAME, _p_timeline->get_prev_value(*this));
        _p_script->set_property(ORIG_NAME, base_value);

        return _p_script->guarded_invoke(RUN_NAME, 0, nullptr, get_value_type());

    } catch (scripting_helper::engine::scripting_engine_error &err) {
        const auto data = get_data();
        THROW(std::format("Error while invoking function 'run()' in script ({}) for modifier action ({}):\n"
                          "{}",
                          data->h_script_name, data->h_action_name, err.what()));
    }
}

void composite_action::init(const std::shared_ptr<action_data> &data, action_timeline_keyframe *p_parent) {
    const auto cad = std::dynamic_pointer_cast<composite_action_data>(data);
    REQUIRES_NOT_NULL_MSG(cad, std::format("Failed to cast action data ({}) to composite action data.", data->h_action_name));
    REQUIRES_VALID(*cad);

    action::init(data, p_parent);

    _timeline.init({cad->timeline}, p_parent->get_timeline().get_stage(), this);
}

void composite_action::fina() {
    _timeline.fina();
    action::fina();
}

action_timeline &composite_action::get_timeline() { return _timeline; }

action_data::action_types composite_action::get_type() const { return action_data::action_types::ACTION_COMPOSITE; }

std::string action::get_locator() const noexcept {
    if (_p_base_data == nullptr) {
        return std::format(R"({} > Action(???))", _p_parent_keyframe != nullptr ? _p_parent_keyframe->get_locator() : "???");
    }
    return std::format("{} > Action({})", _p_parent_keyframe != nullptr ? _p_parent_keyframe->get_locator() : "???", _p_base_data->h_action_name);
}

std::string continuous_action::get_locator() const noexcept {
    if (_p_base_data == nullptr) {
        return std::format(R"({} > ContinuousAction(???))", _p_parent_keyframe != nullptr ? _p_parent_keyframe->get_locator() : "???");
    }
    return std::format("{} > ContinuousAction({})", _p_parent_keyframe != nullptr ? _p_parent_keyframe->get_locator() : "???", _p_base_data->h_action_name);
}

std::string instant_action::get_locator() const noexcept {
    if (_p_base_data == nullptr) {
        return std::format(R"({} > InstantAction(???))", _p_parent_keyframe != nullptr ? _p_parent_keyframe->get_locator() : "???");
    }
    return std::format("{} > InstantAction({})", _p_parent_keyframe != nullptr ? _p_parent_keyframe->get_locator() : "???", _p_base_data->h_action_name);
}

std::string modifier_action::get_locator() const noexcept {
    if (_p_base_data == nullptr) {
        return std::format(R"({} > ModifierAction(???))", _p_parent_keyframe != nullptr ? _p_parent_keyframe->get_locator() : "???");
    }
    return std::format("{} > ModifierAction({})", _p_parent_keyframe != nullptr ? _p_parent_keyframe->get_locator() : "???", _p_base_data->h_action_name);
}

std::string composite_action::get_locator() const noexcept {
    if (_p_base_data == nullptr) {
        return std::format(R"({} > CompositeAction(???))", _p_parent_keyframe != nullptr ? _p_parent_keyframe->get_locator() : "???");
    }
    return std::format("{} > CompositeAction({})", _p_parent_keyframe != nullptr ? _p_parent_keyframe->get_locator() : "???", _p_base_data->h_action_name);
}

} // namespace camellia
