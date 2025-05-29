//
// Created by LENOVO on 2025/4/4.
//

#include <memory>

#include "camellia_macro.h"
#include "live/play/stage.h"
#include "manager.h"

namespace camellia {

const hash_t stage::H_ROOT_ACTOR_ID = 2007ULL; // ROOT

void stage::set_beat(const std::shared_ptr<beat_data> &beat) {
    REQUIRES_NOT_NULL(beat);
    REQUIRES_VALID(*beat);

    auto *main_dialog = get_main_dialog();
    REQUIRES_NOT_NULL(main_dialog);

    _current_beat = beat;
    _beat_begin_time = _stage_time;

    _root_actor_data->children = beat->activities;
    auto *actor = get_actor(0);

    actor->fina(true);
    actor->init(_root_actor_data, *this, _root_activity);

    _root_activity.fina(true);
    _root_activity.init(_root_activity_data, 0, true, *this, nullptr);

    main_dialog->advance(beat->dialog);
}

void stage::advance() {
    REQUIRES_NOT_NULL(_p_scenario);

    if (_time_to_end > 0.0F) {
        _beat_begin_time -= _time_to_end;
        _time_to_end = 0.0F;
    } else {
        if (_next_beat_index >= _p_scenario->beats.size()) {
            return;
        }
        set_beat(_p_scenario->beats[_next_beat_index]);
        _next_beat_index++;
    }
}

std::shared_ptr<actor_data> stage::get_actor_data(const hash_t h_id) const {
    REQUIRES_NOT_NULL(_p_scenario);

    if (h_id == H_ROOT_ACTOR_ID) {
        return _root_actor_data;
    }
    const auto it = _p_scenario->actors.find(h_id);
    return it == _p_scenario->actors.end() ? nullptr : it->second;
}

std::shared_ptr<action_data> stage::get_action_data(const hash_t h_id) const {
    REQUIRES_NOT_NULL(_p_scenario);
    const auto it = _p_scenario->actions.find(h_id);
    return it == _p_scenario->actions.end() ? nullptr : it->second;
}

void stage::init(const std::shared_ptr<stage_data> &data, manager &parent) {
    REQUIRES_VALID(*data);

    _p_scenario = data;
    _p_parent_backend = &parent;

    _root_activity.init(_root_activity_data, 0, false, *this, nullptr);

    get_main_dialog()->init(*this);
    _is_initialized = true;
}

void stage::fina() {
    _is_initialized = false;
    get_main_dialog()->fina();

    _root_activity.fina(false);

    _p_parent_backend = nullptr;
    _p_scenario = nullptr;
}

number_t stage::update(const number_t stage_time) {
    auto *main_dialog = get_main_dialog();
    REQUIRES_NOT_NULL(main_dialog);

    _stage_time = stage_time;
    const auto beat_time = stage_time - _beat_begin_time;

    _time_to_end = std::max(main_dialog->update(beat_time), _root_activity.update(beat_time));
    return _time_to_end;
}

const std::string *stage::get_script_code(const hash_t h_script_name) const {
    REQUIRES_NOT_NULL(_p_scenario);
    const auto it = _p_scenario->scripts.find(h_script_name);
    return it == _p_scenario->scripts.end() ? nullptr : &it->second;
}

manager &stage::get_parent_manager() {
    REQUIRES_NOT_NULL(_p_parent_backend);
    return *_p_parent_backend;
}

stage::stage(const stage & /*other*/) { THROW_NO_LOC("Copying not allowed"); }

stage &stage::operator=(const stage &other) {
    if (this == &other) {
        return *this;
    }

    THROW_NO_LOC("Copying not allowed");
}

std::string stage::get_locator() const noexcept {
    if (_p_scenario == nullptr) {
        return std::format(R"({} > Stage(???))", _p_parent_backend != nullptr ? _p_parent_backend->get_locator() : "???");
    }
    return std::format("{} > Stage({})", _p_parent_backend != nullptr ? _p_parent_backend->get_locator() : "???", _p_scenario->h_stage_name);
}

} // namespace camellia