#include "action.h"
#include "attribute_registry.h"
#include "camellia.h"
#include "camellia_macro.h"
#include "live/play/stage.h"
#include <format>
#include <memory>
#include <set>

namespace camellia {
action_timeline_keyframe &action::get_parent_keyframe() const {
    REQUIRES_NOT_NULL(_p_parent_keyframe);
    return *_p_parent_keyframe;
}

void action::init(const std::shared_ptr<action_data> &data, action_timeline_keyframe *parent) {
    REQUIRES_VALID(*data);

    _p_base_data = data;
    _p_parent_keyframe = parent;

    _is_initialized = true;
    if (_after_init_cb != nullptr) {
        _after_init_cb(this);
    }
}

void action::fina() {
    if (_before_fina_cb != nullptr) {
        _before_fina_cb(this);
    }

    _is_initialized = false;
    _p_base_data = nullptr;
    _p_parent_keyframe = nullptr;
}

std::shared_ptr<modifier_action_data> modifier_action::get_data() const {
    REQUIRES_NOT_NULL(_p_base_data);
    return std::static_pointer_cast<modifier_action_data>(_p_base_data);
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

    _p_parent_keyframe = p_parent;
    _p_timeline = &_p_parent_keyframe->get_timeline();

    _p_script = new scripting_helper::scripting_engine();

    std::set<text_t> seen;
    auto process_params = [&](const std::map<text_t, variant> &params) {
        for (const auto &p : params) {
            if (seen.contains(p.first)) {
                continue;
            }

            if (p.second.get_value_type() == variant::types::ATTRIBUTE) {
                _ref_params[p.first] = static_cast<hash_t>(p.second);
            } else {
                _p_script->set_property(p.first, p.second);
            }

            seen.insert(p.first);
        }
    };

    process_params(*p_parent->get_override_params());
    process_params(mad->default_params);

    const auto *code = p_parent->get_timeline().get_stage().get_script_code(mad->h_script_name);
    THROW_IF(code == nullptr, std::format("Failed to find script ({}) for modifier action ({}).\n"
                                          "{}",
                                          mad->h_script_name, mad->h_action_name, get_locator()));

    try {
        _p_script->guarded_evaluate(*code, variant::VOID);
    } catch (const scripting_helper::scripting_engine::scripting_engine_error &err) {
        THROW(std::format("Error while evaluating script ({}) for modifier action ({}):\n"
                          "{}",
                          mad->h_script_name, mad->h_action_name, err.what()));
    }

    action::init(data, p_parent);
}

void modifier_action::fina() {
    action::fina();

    _p_timeline = nullptr;
    final_value = variant();

    if (_p_script != nullptr) {
        delete _p_script;
        _p_script = nullptr;
    }
}

variant modifier_action::apply_modifier(const number_t action_time, const hash_t h_attribute_name, const variant &val,
                                        std::vector<std::map<hash_t, variant>> &ref_attributes) const {
    if (get_attribute_name_hash() != h_attribute_name) {
        return val;
    }
    return modify(action_time, val, ref_attributes);
}

void modifier_action::apply_modifier(const number_t action_time, std::map<hash_t, variant> &attributes,
                                     std::vector<std::map<hash_t, variant>> &ref_attributes) const {
    const auto it = attributes.find(get_attribute_name_hash());
    if (it == attributes.end()) {
        // TODO: Report warning
        return;
    }

    attributes[get_attribute_name_hash()] = modify(action_time, it->second, ref_attributes);
}

const char *modifier_action::TIME_NAME = "time";
const char *modifier_action::DURATION_NAME = "duration";
const char *modifier_action::ORIG_NAME = "orig";
const char *modifier_action::RUN_NAME = "run";

variant modifier_action::modify(const number_t action_time, const variant &base_value, std::vector<std::map<hash_t, variant>> &ref_attributes) const {
    REQUIRES_INITIALIZED(*this);

    try {
        // set built-in constants
        _p_script->set_property(TIME_NAME, action_time);
        _p_script->set_property(DURATION_NAME, get_actual_duration());
        _p_script->set_property(ORIG_NAME, base_value);

        for (const auto &p : _ref_params) {
            int i = 0;
            for (; i < ref_attributes.size(); i++) {
                const auto it = ref_attributes[i].find(p.second);
                if (it != ref_attributes[i].end()) {
                    _p_script->set_property(p.first, it->second);
                    break;
                }
            }
            if (i >= ref_attributes.size()) {
                THROW(std::format("Failed to find referenced attribute ({}) for modifier action.", p.second));
            }
        }

        return _p_script->guarded_invoke(RUN_NAME, 0, nullptr, get_value_type());

    } catch (scripting_helper::scripting_engine::scripting_engine_error &err) {
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

    _p_timeline->init({cad->timeline}, p_parent->get_timeline().get_stage(), this);

    action::init(data, p_parent);
}

void composite_action::fina() {
    action::fina();
    _p_timeline->fina();
}

action_timeline &composite_action::get_timeline() { return *_p_timeline; }

std::shared_ptr<composite_action_data> composite_action::get_data() const {
    REQUIRES_NOT_NULL(_p_base_data);
    return std::static_pointer_cast<composite_action_data>(_p_base_data);
}

std::string action::get_locator() const noexcept {
    if (_p_base_data == nullptr) {
        return std::format(R"({} > Action(???))", _p_parent_keyframe != nullptr ? _p_parent_keyframe->get_locator() : "???");
    }
    return std::format("{} > Action({})", _p_parent_keyframe != nullptr ? _p_parent_keyframe->get_locator() : "???", _p_base_data->h_action_name);
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
